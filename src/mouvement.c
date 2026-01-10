/**
 * ============================================================================
 * MOUVEMENT.C - Système de navigation et déplacement des véhicules
 * ============================================================================
 *
 * RESPONSABILITÉS:
 *   - Navigation intelligente basée sur les flèches du plan
 *   - Système de changement de direction avec anticipation
 *   - Parking automatique sur détection de places libres
 *   - Gestion des cibles de stationnement
 *   - Système anti-blocage avec lane-lock
 *
 * MODULES EXTERNES UTILISÉS:
 *   - mouvement/sprites.c    : Gestion des carrosseries et orientations
 *   - mouvement/collision.c  : Détection de collision AABB et vérifications
 *
 * ALGORITHMES PRINCIPAUX:
 *   1. Suivi de flèches avec anticipation (4 cellules devant)
 *   2. Recherche de la flèche la plus proche (distance Manhattan)
 *   3. Parking automatique sur détection flèche ↑/↓
 *   4. Système de scores pour choisir la meilleure direction
 *
 * ============================================================================
 */

#include "mouvement.h"
#include "mouvement/sprites.h"
#include "mouvement/collision.h"
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdio.h>

// Référence au compteur global de frames (défini dans jeu.c)
extern unsigned long int global_frame_counter;

// ============================================================================
// PHASE B: SYSTÈME DE CIBLES STABLES (SANS MODIFIER VEHICULE)
// ============================================================================

#define MAX_VOITURES 20

// ============================================================================
// ETAPE 1: ORIENTATION STABLE (PASSES + FLECHES ZONE, SANS MANHATTAN)
// ============================================================================

#define MAX_SCAN 10    // Nombre max de cellules à scanner pour les passes
#define RAYON_SCAN 8   // Rayon de scan pour détecter les flèches

// ============================================================================
// ETAPE 2: CORRECTION DU CHEMIN (STICKY + ALLÉES STRICTES)
// ============================================================================

// Déclarations forward
static void corriger_alignement_fleche(VEHICULE *vehicule, PlanParking *plan);

typedef struct
{
    VEHICULE *vehicule; // Pointeur vers la voiture (clé)
    int target_place;   // Index de la place cible (-1 = pas de cible, -2 = sortie)
} VoitureTarget;

static VoitureTarget targets[MAX_VOITURES];
static int targets_initialized = 0;

#define TARGET_SORTIE -2  // Valeur spéciale pour indiquer que la voiture doit aller vers la sortie

/* LANE-KEEPING: Verrou de maintien de voie après refus de parking */
typedef struct
{
    VEHICULE *vehicule;
    int turn_lock_counter; // Compteur de pas avant autorisation de tourner
} VoitureLaneLock;

static VoitureLaneLock lane_locks[MAX_VOITURES];
static int lane_locks_initialized = 0;

#define TURN_LOCK_DURATION 5 /* Nombre de pas après PARK_REFUSE avant de pouvoir tourner */

// Initialiser le tableau de lane locks
static void init_lane_locks()
{
    for (int i = 0; i < MAX_VOITURES; i++)
    {
        lane_locks[i].vehicule = NULL;
        lane_locks[i].turn_lock_counter = 0;
    }
    lane_locks_initialized = 1;
}

// Activer le verrou de voie pour une voiture
static void activer_lane_lock(VEHICULE *v)
{
    if (!lane_locks_initialized)
        init_lane_locks();

    // Chercher slot existant ou libre
    for (int i = 0; i < MAX_VOITURES; i++)
    {
        if (lane_locks[i].vehicule == v || lane_locks[i].vehicule == NULL)
        {
            lane_locks[i].vehicule = v;
            lane_locks[i].turn_lock_counter = TURN_LOCK_DURATION;
            return;
        }
    }
}

// Vérifier si une voiture a un verrou de voie actif
static int a_lane_lock_actif(VEHICULE *v)
{
    if (!lane_locks_initialized)
        return 0;

    for (int i = 0; i < MAX_VOITURES; i++)
    {
        if (lane_locks[i].vehicule == v)
            return lane_locks[i].turn_lock_counter > 0;
    }
    return 0;
}

// Décrémenter les compteurs de lane lock (appelé à chaque pas)
static void decrementer_lane_locks()
{
    if (!lane_locks_initialized)
        return;

    for (int i = 0; i < MAX_VOITURES; i++)
    {
        if (lane_locks[i].vehicule != NULL && lane_locks[i].turn_lock_counter > 0)
        {
            lane_locks[i].turn_lock_counter--;
        }
    }
}

// Initialiser le tableau de cibles
static void init_targets()
{
    for (int i = 0; i < MAX_VOITURES; i++)
    {
        targets[i].vehicule = NULL;
        targets[i].target_place = -1;
    }
    targets_initialized = 1;
}

// Obtenir la cible d'une voiture (-1 si pas de cible)
static int obtenir_target(VEHICULE *v)
{
    if (!targets_initialized)
        init_targets();

    for (int i = 0; i < MAX_VOITURES; i++)
    {
        if (targets[i].vehicule == v)
            return targets[i].target_place;
    }
    return -1;
}

// Définir la cible d'une voiture
static void set_target(VEHICULE *v, int place_index)
{
    if (!targets_initialized)
        init_targets();

    // Chercher si la voiture a déjà une entrée
    for (int i = 0; i < MAX_VOITURES; i++)
    {
        if (targets[i].vehicule == v)
        {
            targets[i].target_place = place_index;
            return;
        }
    }

    // Sinon, trouver un slot libre
    for (int i = 0; i < MAX_VOITURES; i++)
    {
        if (targets[i].vehicule == NULL)
        {
            targets[i].vehicule = v;
            targets[i].target_place = place_index;
            return;
        }
    }
}

// Effacer la cible d'une voiture
static void clear_target(VEHICULE *v)
{
    if (!targets_initialized)
        init_targets();

    for (int i = 0; i < MAX_VOITURES; i++)
    {
        if (targets[i].vehicule == v)
        {
            targets[i].vehicule = NULL;
            targets[i].target_place = -1;
            return;
        }
    }
}

// Marque un véhicule pour qu'il aille vers la sortie (fonction publique)
void marquer_vehicule_en_sortie(VEHICULE *vehicule)
{
    set_target(vehicule, TARGET_SORTIE);
}

// ============================================================================
// FONCTIONS HELPERS POUR ORIENTATION STABLE
// ============================================================================

/*
 * Vérifie si un caractère est une flèche simple (←→↑↓)
 */
static int est_fleche_simple(wchar_t c)
{
    return (c == L'←' || c == L'→' || c == L'↑' || c == L'↓');
}

/*
 * LANE-KEEPING: Distingue flèches de circulation (←→) des flèches de parking (↑↓)
 * Retourne 1 si c'est une flèche de CIRCULATION (horizontale), 0 sinon
 */
static int est_fleche_circulation(wchar_t c)
{
    return (c == L'←' || c == L'→');
}

/*
 * Retourne le sens d'une flèche simple : 'N', 'S', 'E', 'O', ou '\0' si pas une flèche
 */
static char sens_fleche(wchar_t c)
{
    if (c == L'←')
        return 'O';
    if (c == L'→')
        return 'E';
    if (c == L'↑')
        return 'N';
    if (c == L'↓')
        return 'S';
    return '\0';
}

/*
 * Compte le nombre de cellules roulables consécutives dans une direction,
 * à partir de la cellule SUIVANTE (pas la position actuelle).
 * Retourne le nombre de "passes" disponibles (max MAX_SCAN).
 */
static int compter_passes(PlanParking *plan, int cx, int cy, char dir)
{
    if (!plan)
        return 0;

    int dx = 0, dy = 0;
    switch (dir)
    {
    case 'N':
        dy = -1;
        break;
    case 'S':
        dy = 1;
        break;
    case 'E':
        dx = 1;
        break;
    case 'O':
        dx = -1;
        break;
    default:
        return 0;
    }

    int passes = 0;
    int x = cx + dx;
    int y = cy + dy;

    for (int i = 0; i < MAX_SCAN; i++)
    {
        if (x < 0 || y < 0 || x >= plan->largeur || y >= plan->hauteur)
            break;

        if (!est_cellule_roulable(plan, x, y))
            break;

        passes++;
        x += dx;
        y += dy;
    }

    return passes;
}

/*
 * Compte les flèches simples dans une zone carrée de rayon R autour de (cx, cy).
 * Remplit counts[4] avec le nombre de flèches pour chaque direction :
 * counts[0] = Nord, counts[1] = Sud, counts[2] = Est, counts[3] = Ouest
 */
static void compter_fleches_zone(PlanParking *plan, int cx, int cy, int R, int counts[4])
{
    counts[0] = counts[1] = counts[2] = counts[3] = 0;

    if (!plan)
        return;

    for (int y = cy - R; y <= cy + R; y++)
    {
        for (int x = cx - R; x <= cx + R; x++)
        {
            if (x < 0 || y < 0 || x >= plan->largeur || y >= plan->hauteur)
                continue;

            wchar_t c = plan->plan_statique[y][x];
            /* RÈGLE C: Ne compter QUE les flèches de circulation (←→), ignorer parking (↑↓) */
            if (!est_fleche_circulation(c))
                continue;

            char sens = sens_fleche(c);
            switch (sens)
            {
            case 'N':
                counts[0]++;
                break;
            case 'S':
                counts[1]++;
                break;
            case 'E':
                counts[2]++;
                break;
            case 'O':
                counts[3]++;
                break;
            }
        }
    }
}

/*
 * ETAPE 2: Compte le nombre de voisins roulables (N, S, E, O).
 * Utilisé pour détecter les intersections (>= 3 voisins).
 */
static int compter_voisins_roulables(PlanParking *plan, int cx, int cy)
{
    int count = 0;

    if (est_cellule_roulable(plan, cx, cy - 1))
        count++; // Nord
    if (est_cellule_roulable(plan, cx, cy + 1))
        count++; // Sud
    if (est_cellule_roulable(plan, cx + 1, cy))
        count++; // Est
    if (est_cellule_roulable(plan, cx - 1, cy))
        count++; // Ouest

    return count;
}

/*
 * ETAPE 2: Détermine si on doit recalculer la direction (logique STICKY).
 * On recalcule SEULEMENT si :
 * 1) Flèche simple sur la cellule actuelle
 * 2) Flèche détectée dans la zone
 * 3) Cellule devant non roulable (blocage)
 * 4) Intersection (>= 3 voisins roulables)
 * 5) S'éloigne de la cible (direction opposée à la cible et distance > 5)
 */
static int doit_recalculer_direction(VEHICULE *vehicule, PlanParking *plan, int fleches_zone[4], int target_x, int target_y)
{
    if (!vehicule || !plan)
        return 1;

    /* IMPORTANT: Utiliser le CENTRE du véhicule pour les tests, pas le coin */
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);

    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;
    char dir = vehicule->direction;

    // Vérifier limites du centre
    if (centre_x < 0 || centre_y < 0 || centre_x >= plan->largeur || centre_y >= plan->hauteur)
        return 1;

    wchar_t c_actuel = plan->plan_statique[centre_y][centre_x];

    /* LANE-KEEPING RÈGLE A: Si lane_lock actif ET allée continue devant → interdire recalcul */
    if (a_lane_lock_actif(vehicule))
    {
        /* FIX RÉGRESSION: Vérifier cellule devant en utilisant le CENTRE (pas le coin) */
        int dx = 0, dy = 0;
        switch (dir)
        {
        case 'N': dy = -1; break;
        case 'S': dy = 1; break;
        case 'E': dx = 1; break;
        case 'O': dx = -1; break;
        }

        /* Calculer position du centre APRÈS déplacement */
        int next_centre_x = centre_x + dx;
        int next_centre_y = centre_y + dy;

        /* Vérifier si le centre suivant est dans une allée roulable */
        if (next_centre_x >= 0 && next_centre_y >= 0 &&
            next_centre_x < plan->largeur && next_centre_y < plan->hauteur)
        {
            wchar_t c_next = plan->plan_statique[next_centre_y][next_centre_x];
            int allee_devant = (c_next == L' ' || c_next == L'←' || c_next == L'→' ||
                               c_next == L'↑' || c_next == L'↓' || c_next == L'.');


            if (allee_devant)
                return 0; /* INTERDIRE changement de direction */
        }
    }

    // Condition 1a: Flèche de CIRCULATION sur la cellule du centre
    /* RÈGLE C: Flèches de circulation (←→) déclenchent TOUJOURS recalcul */
    if (est_fleche_circulation(c_actuel))
        return 1;

    // Condition 1b: Flèche de PARKING (↑↓) PROCHE du centre (±3) ET place libre disponible
    /* PATCH RÉGRESSION: Permettre recalcul si flèche parking + place libre sur rangée */
    /* Utiliser MÊME logique que tenter_parking_automatique(): zone ±3 pour compenser décalage sprite */
    wchar_t fleche_proche = 0;
    int fleche_proche_y = -1;

    for (int dy = -3; dy <= 3; dy++)
    {
        for (int dx = -3; dx <= 3; dx++)
        {
            int check_x = centre_x + dx;
            int check_y = centre_y + dy;

            if (check_x >= 0 && check_x < plan->largeur &&
                check_y >= 0 && check_y < plan->hauteur)
            {
                wchar_t c = plan->plan_statique[check_y][check_x];
                if (c == L'↑' || c == L'↓')
                {
                    fleche_proche = c;
                    fleche_proche_y = check_y;
                    break;
                }
            }
        }
        if (fleche_proche)
            break;
    }

    if (fleche_proche)
    {
        /* Vérifier s'il existe une place libre sur cette rangée */
        int ligne_rangee = fleche_proche_y - 1;

        for (int i = 0; i < plan->places_totales; i++)
        {
            if (plan->places[i].occupee == 0 && plan->places[i].ligne == ligne_rangee)
            {
                /* Place libre trouvée sur la rangée -> autoriser recalcul direction */
                return 1; /* AUTORISER changement vers direction parking */
            }
        }
        /* Pas de place libre -> ne pas recalculer, continuer tout droit */
        return 0;
    }

    // Condition 2: Flèches de circulation perpendiculaires détectées
    /* RÈGLE C: Ne compter QUE les flèches de circulation (←→), PAS les flèches de parking (↑↓) */
    /* Cette condition sera mise à jour dans compter_fleches_zone() - pour l'instant désactivée */
    int fleches_perp = 0;
    if (dir == 'N' || dir == 'S') fleches_perp = fleches_zone[2] + fleches_zone[3]; // E/O
    else fleches_perp = fleches_zone[0] + fleches_zone[1]; // N/S

    if (fleches_perp > 0)
        return 1;

    // Condition 3: Sprite entier ne peut pas avancer (blocage RÉEL, pas place occupée)
    int dx = 0, dy = 0;
    switch (dir)
    {
    case 'N':
        dy = -1;
        break;
    case 'S':
        dy = 1;
        break;
    case 'E':
        dx = 1;
        break;
    case 'O':
        dx = -1;
        break;
    }

    int nx = vehicule->posx + dx;
    int ny = vehicule->posy + dy;

    /* NE PAS recalculer si bloqué uniquement par bordures de places (allée continue) */
    if (!peut_deplacer_sur_allee(vehicule, plan, nx, ny))
    {
        /* Vérifier si c'est un vrai blocage (hors limites / mur) ou juste place adjacente */
        int est_hors_limites = (nx < 0 || ny < 0 ||
                                nx + largeur > plan->largeur ||
                                ny + hauteur > plan->hauteur);

        if (est_hors_limites)
            return 1; /* Vrai blocage */

        /* Sinon, vérifier si c'est juste les allées/places qui continuent */
        /* Si le CENTRE de la nouvelle position est sur espace/flèche, continuer */
        int new_centre_x = nx + largeur / 2;
        int new_centre_y = ny + hauteur / 2;

        if (new_centre_x >= 0 && new_centre_y >= 0 &&
            new_centre_x < plan->largeur && new_centre_y < plan->hauteur)
        {
            wchar_t c_new = plan->plan_statique[new_centre_y][new_centre_x];
            /* Si centre sur allée/flèche, l'allée continue - NE PAS recalculer */
            if (c_new == L' ' || c_new == L'←' || c_new == L'→' ||
                c_new == L'↑' || c_new == L'↓' || c_new == L'.')
                return 0; /* Allée continue, pas de recalcul */
        }

        /* Sinon vrai blocage */
        return 1;
    }

    // Condition 4: Intersection (>= 3 voisins) - tester au CENTRE du véhicule
    int voisins = compter_voisins_roulables(plan, centre_x, centre_y);
    if (voisins >= 3)
        return 1;

    // Condition 5: S'éloigne de la cible (si cible définie)
    if (target_x >= 0 && target_y >= 0)
    {
        int dx_cible = target_x - centre_x;
        int dy_cible = target_y - centre_y;

        /* Recalculer si on va dans le sens opposé à la cible et qu'on est assez loin */
        if (abs(dy_cible) > 5 || abs(dx_cible) > 5)
        {
            // Se dirige vers le Nord mais cible au Sud
            if (dir == 'N' && dy_cible > 5)
                return 1;
            // Se dirige vers le Sud mais cible au Nord
            if (dir == 'S' && dy_cible < -5)
                return 1;
            // Se dirige vers l'Est mais cible à l'Ouest
            if (dir == 'E' && dx_cible < -5)
                return 1;
            // Se dirige vers l'Ouest mais cible à l'Est
            if (dir == 'O' && dx_cible > 5)
                return 1;
        }
    }

    // Aucune condition remplie : ne pas recalculer (STICKY)
    return 0;
}

/*
 * ETAPE 2: Choisit la meilleure direction avec score amélioré (10*passes + 3*counts).
 * Priorité : Y (Nord/Sud) puis X (Est/Ouest).
 * Sticky amélioré : garde la direction actuelle si score >= meilleur-2.
 * Bonus cible : ajoute +20 au score de la direction qui rapproche de la cible.
 */
static char choisir_direction_stable(VEHICULE *vehicule, PlanParking *plan, int target_x, int target_y)
{
    if (!vehicule || !plan)
        return vehicule->direction;

    /* IMPORTANT: Utiliser le CENTRE du véhicule pour tous les calculs */
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);

    int cx = vehicule->posx + largeur / 2;
    int cy = vehicule->posy + hauteur / 2;

    // Vérifier si le centre est hors limites
    if (cx < 0 || cy < 0 || cx >= plan->largeur || cy >= plan->hauteur)
        return vehicule->direction;

    wchar_t c_actuel = plan->plan_statique[cy][cx];

    // REGLE A : Si cellule du centre est une flèche simple, suivre immédiatement
    if (est_fleche_simple(c_actuel))
    {
        char nouvelle_dir = sens_fleche(c_actuel);
        return nouvelle_dir;
    }

    // REGLE A bis : Si flèche de circulation (←→) dans zone ±2 autour du centre, suivre immédiatement
    // Cette règle compense le décalage du sprite (centre peut être décalé d'une ligne par rapport à la flèche)
    for (int dy = -2; dy <= 2; dy++)
    {
        for (int dx = -2; dx <= 2; dx++)
        {
            int check_y = cy + dy;
            int check_x = cx + dx;

            if (check_x >= 0 && check_x < plan->largeur && check_y >= 0 && check_y < plan->hauteur)
            {
                wchar_t c = plan->plan_statique[check_y][check_x];
                if (est_fleche_circulation(c))
                {
                    char dir_fleche = sens_fleche(c);

                    // Pour les flèches horizontales (←→), vérifier si on est sur la bonne ligne (±1)
                    // Pour les flèches verticales (↑↓), vérifier si on est sur la bonne colonne (±1)
                    int sur_ligne_horizontale = (dir_fleche == 'E' || dir_fleche == 'O') && (abs(dy) <= 1);
                    int sur_colonne_verticale = (dir_fleche == 'N' || dir_fleche == 'S') && (abs(dx) <= 1);

                    if (sur_ligne_horizontale || sur_colonne_verticale)
                    {
                        return dir_fleche;
                    }
                }
            }
        }
    }

    // PATCH RÉGRESSION: Si PROCHE flèche de PARKING (↑↓, zone ±3) ET place libre, forcer direction
    /* Utiliser MÊME logique que tenter_parking_automatique(): zone ±3 pour compenser décalage sprite */
    wchar_t fleche_parking_proche = 0;
    int fleche_parking_y = -1;

    for (int dy = -3; dy <= 3; dy++)
    {
        for (int dx = -3; dx <= 3; dx++)
        {
            int check_x = cx + dx;
            int check_y = cy + dy;

            if (check_x >= 0 && check_x < plan->largeur &&
                check_y >= 0 && check_y < plan->hauteur)
            {
                wchar_t c = plan->plan_statique[check_y][check_x];
                if (c == L'↑' || c == L'↓')
                {
                    fleche_parking_proche = c;
                    fleche_parking_y = check_y;
                    break;
                }
            }
        }
        if (fleche_parking_proche)
            break;
    }

    if (fleche_parking_proche)
    {
        int ligne_rangee = fleche_parking_y - 1;

        /* Vérifier s'il existe une place libre sur cette rangée */
        for (int i = 0; i < plan->places_totales; i++)
        {
            if (plan->places[i].occupee == 0 && plan->places[i].ligne == ligne_rangee)
            {
                /* Place libre trouvée -> forcer direction parking */
                char dir_parking = sens_fleche(fleche_parking_proche); // 'N' pour ↑, 'S' pour ↓
                /* PATCH CRITIQUE: Activer lane-lock pour protéger la direction parking des intersections */
                activer_lane_lock(vehicule);
                return dir_parking;
            }
        }
        /* Pas de place libre -> ne pas forcer, utiliser logique normale */
    }

    // REGLE B/C : Scanner la zone et calculer les scores (depuis le centre)
    int fleches_zone[4]; // [N, S, E, O]
    compter_fleches_zone(plan, cx, cy, RAYON_SCAN, fleches_zone);

    // Calculer les passes pour chaque direction (depuis le centre)
    int passes_N = compter_passes(plan, cx, cy, 'N');
    int passes_S = compter_passes(plan, cx, cy, 'S');
    int passes_E = compter_passes(plan, cx, cy, 'E');
    int passes_O = compter_passes(plan, cx, cy, 'O');

    // ETAPE 2: Calculer les scores avec poids: 10*passes + 3*counts
    int score_N = 10 * passes_N + 3 * fleches_zone[0];
    int score_S = 10 * passes_S + 3 * fleches_zone[1];
    int score_E = 10 * passes_E + 3 * fleches_zone[2];
    int score_O = 10 * passes_O + 3 * fleches_zone[3];

    /* Bonus cible : +20 pour la direction qui rapproche de la cible */
    if (target_x >= 0 && target_y >= 0)
    {
        int dx_cible = target_x - cx;
        int dy_cible = target_y - cy;

        if (dy_cible < -3 && passes_N > 0) score_N += 20; // Cible au-dessus
        if (dy_cible > 3 && passes_S > 0) score_S += 20;  // Cible en-dessous
        if (dx_cible > 3 && passes_E > 0) score_E += 20;  // Cible à droite
        if (dx_cible < -3 && passes_O > 0) score_O += 20; // Cible à gauche
    }

    /* Bonus navigation vers la ligne de parking */
    if (cy >= 20 && cy <= 26)
    {
        // Entre lignes 20-26 :
        // - Si colonne > 85 : favoriser Ouest d'abord (éviter blocage)
        // - Sinon : favoriser Nord pour atteindre la ligne de parking
        if (cx > 85)
        {
            if (passes_O > 0) score_O += 50;
        }
        else
        {
            if (passes_N > 0) score_N += 40;
        }
    }
    else if (cy >= 17 && cy <= 19)
    {
        // Sur ligne de parking (17-19) : favoriser Ouest pour chercher les flèches
        if (passes_O > 0) score_O += 60;
        if (passes_E > 0) score_E += 30;

    }

    char dir_actuelle = vehicule->direction;
    char nouvelle_dir = dir_actuelle;

    // Trouver le meilleur score parmi toutes les directions
    int meilleur = score_N;
    char dir_meilleur = 'N';

    if (score_S > meilleur)
    {
        meilleur = score_S;
        dir_meilleur = 'S';
    }
    if (score_E > meilleur)
    {
        meilleur = score_E;
        dir_meilleur = 'E';
    }
    if (score_O > meilleur)
    {
        meilleur = score_O;
        dir_meilleur = 'O';
    }

    // Si tous les scores sont à 0, garder direction actuelle
    if (meilleur == 0)
    {
        nouvelle_dir = dir_actuelle;
    }
    else
    {
        // Sinon, choisir la direction avec le meilleur score
        nouvelle_dir = dir_meilleur;
    }

    // Sticky final : si direction actuelle a des passes > 0 et que le score est égal au meilleur, garder
    int score_actuel = 0;
    if (dir_actuelle == 'N')
        score_actuel = score_N;
    else if (dir_actuelle == 'S')
        score_actuel = score_S;
    else if (dir_actuelle == 'E')
        score_actuel = score_E;
    else if (dir_actuelle == 'O')
        score_actuel = score_O;

    int meilleur_score = score_N;
    if (score_S > meilleur_score)
        meilleur_score = score_S;
    if (score_E > meilleur_score)
        meilleur_score = score_E;
    if (score_O > meilleur_score)
        meilleur_score = score_O;

    // ETAPE 2: Stabilité - garder direction actuelle si score >= meilleur-2
    if (score_actuel >= meilleur_score - 2 && score_actuel > 0)
    {
        nouvelle_dir = dir_actuelle;
    }

    /* PATCHES A+B+C+D: RÈGLES STRICTES ANTI-CHANGEMENT D'ALLÉE */
    int dx_new = 0, dy_new = 0;
    switch (nouvelle_dir)
    {
    case 'N': dy_new = -1; break;
    case 'S': dy_new = 1; break;
    case 'E': dx_new = 1; break;
    case 'O': dx_new = -1; break;
    }

    int dx_old = 0, dy_old = 0;
    switch (dir_actuelle)
    {
    case 'N': dy_old = -1; break;
    case 'S': dy_old = 1; break;
    case 'E': dx_old = 1; break;
    case 'O': dx_old = -1; break;
    }

    int next_cx_new = cx + dx_new;
    int next_cy_new = cy + dy_new;
    int next_cx_old = cx + dx_old;
    int next_cy_old = cy + dy_old;

    /* Vérifier si cellule devant dans chaque direction est roulable */
    int new_dir_safe = 0;
    int old_dir_safe = 0;
    wchar_t c_ahead_new = L'?';
    wchar_t c_ahead_old = L'?';

    if (next_cx_new >= 0 && next_cy_new >= 0 && next_cx_new < plan->largeur && next_cy_new < plan->hauteur)
    {
        c_ahead_new = plan->plan_statique[next_cy_new][next_cx_new];
        new_dir_safe = (c_ahead_new == L' ' || c_ahead_new == L'←' || c_ahead_new == L'→' ||
                       c_ahead_new == L'↑' || c_ahead_new == L'↓' || c_ahead_new == L'.');
    }

    if (next_cx_old >= 0 && next_cy_old >= 0 && next_cx_old < plan->largeur && next_cy_old < plan->hauteur)
    {
        c_ahead_old = plan->plan_statique[next_cy_old][next_cx_old];
        old_dir_safe = (c_ahead_old == L' ' || c_ahead_old == L'←' || c_ahead_old == L'→' ||
                       c_ahead_old == L'↑' || c_ahead_old == L'↓' || c_ahead_old == L'.');
    }

    char chosen_dir = nouvelle_dir;
    const char *block_reason = NULL;

    /* RÈGLE A: Si allée continue devant (old_dir_safe), INTERDIRE changement latéral */
    /* Changement latéral = passer de E/O à N/S ou inversement */
    int is_lateral_change = 0;
    if ((dir_actuelle == 'N' || dir_actuelle == 'S') && (nouvelle_dir == 'E' || nouvelle_dir == 'O'))
        is_lateral_change = 1;
    if ((dir_actuelle == 'E' || dir_actuelle == 'O') && (nouvelle_dir == 'N' || nouvelle_dir == 'S'))
        is_lateral_change = 1;

    /* Détecter intersection RÉELLE (>=3 voisins ALLÉE, pas bordures parking) */
    int nb_voisins_allee = 0;
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0) continue; // Pas le centre
            if (abs(dx) + abs(dy) != 1) continue; // Seulement N/S/E/O, pas diagonales

            int check_x = cx + dx;
            int check_y = cy + dy;
            if (check_x >= 0 && check_y >= 0 && check_x < plan->largeur && check_y < plan->hauteur)
            {
                wchar_t c = plan->plan_statique[check_y][check_x];
                if (c == L' ' || c == L'←' || c == L'→' || c == L'↑' || c == L'↓' || c == L'.')
                    nb_voisins_allee++;
            }
        }
    }
    int is_intersection = (nb_voisins_allee >= 3);

    /* Flèche de circulation SOUS le centre (pas juste dans la zone) */
    int fleche_circulation_sous_centre = est_fleche_circulation(c_actuel);

    /* PATCH A: Exception pour flèches de PARKING (zone ±3) - vérifier si nouvelle direction = direction parking */
    int exception_parking = 0;

    /* Chercher flèche de parking proche du centre (±3 pour compenser décalage sprite) */
    wchar_t fleche_exception = 0;
    int fleche_exception_y = -1;

    for (int dy = -3; dy <= 3; dy++)
    {
        for (int dx = -3; dx <= 3; dx++)
        {
            int check_x = cx + dx;
            int check_y = cy + dy;

            if (check_x >= 0 && check_x < plan->largeur &&
                check_y >= 0 && check_y < plan->hauteur)
            {
                wchar_t c = plan->plan_statique[check_y][check_x];
                if (c == L'↑' || c == L'↓')
                {
                    fleche_exception = c;
                    fleche_exception_y = check_y;
                    break;
                }
            }
        }
        if (fleche_exception)
            break;
    }

    if (fleche_exception)
    {
        char dir_parking_attendue = sens_fleche(fleche_exception); // 'N' pour ↑, 'S' pour ↓
        if (nouvelle_dir == dir_parking_attendue)
        {
            /* La nouvelle direction correspond à la flèche de parking */
            int ligne_rangee = fleche_exception_y - 1;

            /* Vérifier qu'il y a une place libre sur cette rangée */
            for (int i = 0; i < plan->places_totales; i++)
            {
                if (plan->places[i].occupee == 0 && plan->places[i].ligne == ligne_rangee)
                {
                    exception_parking = 1;
                    break;
                }
            }
        }
    }

    /* RÈGLE A: Interdire changement latéral si allée continue devant ET pas intersection/flèche SAUF exception parking */
    if (is_lateral_change && old_dir_safe && !is_intersection && !fleche_circulation_sous_centre && !exception_parking)
    {
        chosen_dir = dir_actuelle;
        block_reason = "LANE_KEEP";
    }

    /* RÈGLE B: Interdire demi-tour (U-turn) sauf blocage réel */
    int is_uturn = 0;
    if ((dir_actuelle == 'N' && nouvelle_dir == 'S') || (dir_actuelle == 'S' && nouvelle_dir == 'N'))
        is_uturn = 1;
    if ((dir_actuelle == 'E' && nouvelle_dir == 'O') || (dir_actuelle == 'O' && nouvelle_dir == 'E'))
        is_uturn = 1;

    if (is_uturn && old_dir_safe && !block_reason)
    {
        chosen_dir = dir_actuelle;
        block_reason = "UTURN_FORBIDDEN";
    }

    /* Si nouvelle direction pas safe mais ancienne l'est, garder ancienne */
    if (!new_dir_safe && old_dir_safe && !block_reason)
    {
        chosen_dir = dir_actuelle;
        block_reason = "NEW_DIR_UNSAFE";
    }
    /* Si ni nouvelle ni ancienne pas safe, garder ancienne (voiture bloquée) */
    else if (!new_dir_safe && !old_dir_safe && !block_reason)
    {
        chosen_dir = dir_actuelle;
        block_reason = "BOTH_UNSAFE";
    }

    /* PATCH D: Log [TURN_DECISION] ou [TURN_BLOCKED] */


    return chosen_dir;
}

/*
 * ============================================================================
 * SYSTEME DE COORDONNEES (UNE SEULE SOURCE DE VERITE)
 * ============================================================================
 *
 * Ce fichier utilise UNIQUEMENT des coordonnées cellules wchar:
 *
 * - vehicule->posx, vehicule->posy : indices dans plan_statique[y][x]
 *   (coin haut-gauche du sprite en cellules wchar, PAS des pixels visuels)
 *
 * - centre_x = posx + largeur/2, centre_y = posy + hauteur/2
 *   (centre du sprite pour la détection de flèches)
 *
 * - plan->plan_statique[y][x] : grille de wchar_t
 *   x = colonne (0 à plan->largeur-1)
 *   y = ligne (0 à plan->hauteur-1)
 *
 * - plan->fleches[i].colonne, plan->fleches[i].ligne : indices wchar
 *   (positions exactes des flèches dans plan_statique)
 *
 * IMPORTANT: Aucune conversion pixel↔cellule n'est nécessaire.
 * Tout le mouvement et la détection se fait en coordonnées cellules.
 * L'affichage se charge de convertir cellule→pixel si nécessaire.
 *
 * ============================================================================
 * LOGIQUE DE MOUVEMENT (SIMPLE ET ROBUSTE)
 * ============================================================================
 *
 * 1. FLECHES SIMPLES (← → ↑ ↓):
 *    - TOUJOURS imposent leur direction
 *    - dir_voiture = direction_sortie
 *
 * 2. VIRAGES (⮠ ⮡ ⮢ ⮣ ⮤ ⮥ ⮦ ⮧):
 *    - Chaque virage a (dir_entree -> dir_sortie)
 *    - SI dir_voiture == dir_entree:
 *        dir_voiture = dir_sortie  (on tourne)
 *    - SINON:
 *        dir_voiture inchangée     (on IGNORE le virage)
 *
 * 3. DEPLACEMENT:
 *    - Voiture avance TOUJOURS d'une cellule dans dir_voiture
 *    - Si hors limites ou obstacle: reste sur place
 *    - JAMAIS d'arrêt logique (etat reste à '1')
 *    - JAMAIS de disparition
 *
 * REGLE FONDAMENTALE: Les virages ne sont JAMAIS bloquants
 *
 * ============================================================================
 */




void suivre_fleches(VEHICULE *vehicule, PlanParking *plan)
{
    if (!vehicule || !plan || vehicule->etat != '1')
        return;

    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);

    /* Vérifier uniquement le CENTRE du véhicule pour éviter les fausses détections */
    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;

    if (centre_x < 0 || centre_y < 0 || centre_x >= plan->largeur || centre_y >= plan->hauteur)
        return;

    /* ANTICIPATION MODÉRÉE: regarder quelques cellules devant */
    int anticipation = 4; // Réduit pour éviter détections multiples

    /* Trouver la flèche LA PLUS PROCHE (pas la première trouvée) */
    int meilleure_distance = 999;
    int meilleur_index = -1;

    for (int i = 0; i < plan->nb_fleches; i++)
    {
        int fx = plan->fleches[i].colonne;
        int fy = plan->fleches[i].ligne;

        /* Distance Manhattan au centre */
        int dist_centre = abs(fx - centre_x) + abs(fy - centre_y);

        /* Vérifier si flèche est DEVANT selon la direction */
        int est_devant = 0;
        switch (vehicule->direction)
        {
            case 'N': est_devant = (fy < centre_y && fy >= centre_y - anticipation); break;
            case 'S': est_devant = (fy > centre_y && fy <= centre_y + anticipation); break;
            case 'E': est_devant = (fx > centre_x && fx <= centre_x + anticipation); break;
            case 'O': est_devant = (fx < centre_x && fx >= centre_x - anticipation); break;
        }

        /* Si sur flèche ou flèche devant, garder la plus proche */
        if ((dist_centre <= 2) || est_devant)
        {
            if (dist_centre < meilleure_distance)
            {
                meilleure_distance = dist_centre;
                meilleur_index = i;
            }
        }
    }

    /* Appliquer la direction de la flèche la plus proche */
    if (meilleur_index >= 0)
    {
        if (plan->fleches[meilleur_index].direction_entree == '\0')
        {
            /* Flèche simple : TOUJOURS impose la direction */
            vehicule->direction = plan->fleches[meilleur_index].direction_sortie;
        }
        else
        {
            /* Virage : applique SI direction d'entrée correcte */
            if (vehicule->direction == plan->fleches[meilleur_index].direction_entree)
            {
                vehicule->direction = plan->fleches[meilleur_index].direction_sortie;
            }
        }
    }
}

// ============================================================================
// GESTION DES SPRITES PAR DIRECTION
// ============================================================================




void deplacer_vehicule(VEHICULE *vehicule, PlanParking *plan)
{
    if (!vehicule || !plan || vehicule->etat != '1')
        return;

    /* Vérifier si on doit changer de direction (flèche ou virage) */
    char ancienne_direction = vehicule->direction;
    suivre_fleches(vehicule, plan);

    /* Si la direction a changé, orienter la carrosserie */
    if (ancienne_direction != vehicule->direction)
    {
        orienter_carrosserie(vehicule);
    }

    /* Calculer la nouvelle position selon la direction actuelle */
    int nouveau_x = vehicule->posx;
    int nouveau_y = vehicule->posy;

    switch (vehicule->direction)
    {
    case 'N':
        nouveau_y -= vehicule->vitesse;
        break;
    case 'S':
        nouveau_y += vehicule->vitesse;
        break;
    case 'E':
        nouveau_x += vehicule->vitesse;
        break;
    case 'O':
        nouveau_x -= vehicule->vitesse;
        break;
    default:
        return;
    }

    /* Vérifier limites + obstacles */
    if (peut_deplacer(vehicule, plan, nouveau_x, nouveau_y))
    {
        vehicule->posx = nouveau_x;
        vehicule->posy = nouveau_y;
    }
    else
    {
        /* BLOQUÉ: essayer de tourner pour éviter de rester coincé */
        /* Essayer dans l'ordre: gauche, droite, demi-tour */
        char nouvelles_directions[] = {'N', 'S', 'E', 'O'};

        for (int i = 0; i < 4; i++)
        {
            if (nouvelles_directions[i] == vehicule->direction)
                continue; // Pas la direction actuelle

            int test_x = vehicule->posx;
            int test_y = vehicule->posy;

            switch (nouvelles_directions[i])
            {
                case 'N': test_y -= vehicule->vitesse; break;
                case 'S': test_y += vehicule->vitesse; break;
                case 'E': test_x += vehicule->vitesse; break;
                case 'O': test_x -= vehicule->vitesse; break;
            }

            if (peut_deplacer(vehicule, plan, test_x, test_y))
            {
                vehicule->direction = nouvelles_directions[i];
                orienter_carrosserie(vehicule);
                vehicule->posx = test_x;
                vehicule->posy = test_y;
                return;
            }
        }
        /* Sinon reste sur place */
    }
}

// ============================================================================
// SYSTÈME DE PARKING AUTOMATIQUE (SIMPLE, SANS ÉTAT)
// ============================================================================

/*
 * Trouve la place libre la plus proche d'un véhicule (distance Manhattan).
 * Retourne l'index de la place dans plan->places[], ou -1 si aucune place libre.
 */
int trouver_place_libre_proche(VEHICULE *vehicule, PlanParking *plan)
{
    if (!vehicule || !plan)
        return -1;

    int meilleur_index = -1;
    int meilleure_distance = 999999;

    /* Position du véhicule (centre approximatif) */
    int cx = vehicule->posx;
    int cy = vehicule->posy;

    /* Parcourir toutes les places */
    for (int i = 0; i < plan->places_totales; i++)
    {
        /* Vérifier si la place est libre */
        if (plan->places[i].occupee == 0)
        {
            /* Calculer distance Manhattan */
            int place_x = plan->places[i].colonne;
            int place_y = plan->places[i].ligne;
            int distance = abs(cx - place_x) + abs(cy - place_y);

            /* Garder la plus proche */
            if (distance < meilleure_distance)
            {
                meilleure_distance = distance;
                meilleur_index = i;
            }
        }
    }

    return meilleur_index;
}

/*
 * Vérifie si une cellule est "roulable" (pas un mur)
 */
/*
 * ETAPE 2: Définition STRICTE des allées roulables.
 * Une cellule est roulable UNIQUEMENT si elle appartient à une allée.
 * Les places de parking (╦, ╩, ║, ═, P) NE SONT PAS roulables.
 */

/*
 * Version exportée pour le diagnostic (même fonction)
 */

/*
 * ETAPE 2: Détecte si la voiture passe devant une place libre et la fait spawner dessus.
 * Retourne 1 si la voiture a été garée, 0 sinon.
 */
static int tenter_parking_automatique(VEHICULE *vehicule, PlanParking *plan)
{
    if (!vehicule || !plan)
        return 0;

    /* Calculer le centre du véhicule */
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;

    /* Vérifier limites */
    if (centre_x < 0 || centre_y < 0 || centre_x >= plan->largeur || centre_y >= plan->hauteur)
        return 0;

    /* Chercher une flèche de parking (↓ ou ↑) près du centre (tolérance ±3 pour compenser décalage sprite) */
    wchar_t fleche_trouvee = 0;
    int fleche_x = -1, fleche_y = -1;

    for (int dy = -3; dy <= 3; dy++)
    {
        for (int dx = -3; dx <= 3; dx++)
        {
            int check_x = centre_x + dx;
            int check_y = centre_y + dy;

            if (check_x >= 0 && check_x < plan->largeur &&
                check_y >= 0 && check_y < plan->hauteur)
            {
                wchar_t c = plan->plan_statique[check_y][check_x];
                if (c == L'↓' || c == L'↑')
                {
                    fleche_trouvee = c;
                    fleche_x = check_x;
                    fleche_y = check_y;
                    break;
                }
            }
        }
        if (fleche_trouvee)
            break;
    }

    if (!fleche_trouvee)
    {
        return 0; /* Aucune flèche de parking proche du centre */
    }

    /* Déterminer la direction de la flèche */
    char dir_parking = (fleche_trouvee == L'↓') ? 'S' : 'N';


    /* VERROU STRICT: Direction véhicule DOIT matcher direction parking */
    if (vehicule->direction != dir_parking)
    {
        /* LANE-KEEPING: Activer verrou de maintien de voie après refus */
        activer_lane_lock(vehicule);
        return 0; /* Direction incompatible - REFUSER parking */
    }

    /* ETAPE 3: Chercher une place libre sur la MÊME RANGÉE que la flèche
     * Les flèches sont ligne Y (milieu de rangée), les places (╦) sont ligne Y-1
     * Structure:
     *   Ligne Y-1: ╦ (haut de place)
     *   Ligne Y  : ║ + flèches ↓↑ (milieu, dans l'allée)
     *   Ligne Y+1: ╩ (bas de place)
     */
    int place_trouvee = -1;
    int dist_min = 99999;

    /* Calculer la ligne de la rangée : flèche ligne Y → places ligne Y-1 */
    int ligne_rangee = fleche_y - 1;

    for (int i = 0; i < plan->places_totales; i++)
    {
        if (plan->places[i].occupee != 0)
            continue; /* Place déjà occupée */

        /* ETAPE 3: La place doit être sur la MÊME RANGÉE (ligne Y-1) */
        if (plan->places[i].ligne != ligne_rangee)
            continue; /* Pas sur la bonne rangée */

        /* Distance horizontale (colonne) */
        int dx = abs(plan->places[i].colonne - fleche_x);

        /* Garder la place la plus proche horizontalement (±15 colonnes max) */
        if (dx < dist_min && dx <= 15)
        {
            dist_min = dx;
            place_trouvee = i;
        }
    }

    /* Si aucune place trouvée, ne rien faire */
    if (place_trouvee == -1)
    {
        return 0;
    }


    /* SPAWN : Téléporter la voiture sur la place */
    vehicule->posx = plan->places[place_trouvee].colonne - largeur / 2;
    vehicule->posy = plan->places[place_trouvee].ligne + 1; /* Juste en dessous du ╦ */
    vehicule->direction = dir_parking;
    orienter_carrosserie(vehicule);

    /* Marquer la place comme occupée */
    marquer_place_occupee(plan, place_trouvee);
    vehicule->etat = '0';              /* Désactiver la voiture (garée) */
    vehicule->tps = global_frame_counter;  /* Enregistrer le temps d'entrée */


    return 1; /* Parking réussi */
}

/*
 * ETAPE 2: Vérifie si TOUT le sprite du véhicule peut être placé sur des cellules roulables.
 * Retourne 1 si toutes les cellules occupées par le sprite sont roulables, 0 sinon.
 */

/*
 * PHASE C: Déplace un véhicule d'une cellule vers une place cible.
 * Tente X d'abord, sinon tente Y, sinon reste sur place.
 * Si le véhicule arrive sur la place, marque la place comme occupée.
 */
void deplacer_vers_place(VEHICULE *vehicule, PlanParking *plan, int index_place)
{
    if (!vehicule || !plan || vehicule->etat != '1')
        return;
    if (index_place < 0 || index_place >= plan->places_totales)
        return;

    int target_x = plan->places[index_place].colonne;
    int target_y = plan->places[index_place].ligne;

    /* Vérifier si arrivé */
    if (vehicule->posx == target_x && vehicule->posy == target_y)
    {
        /* Position == target : voiture arrivée, marquer la place occupée */
        marquer_place_occupee(plan, index_place);
        clear_target(vehicule); /* Libérer la cible */
        return;
    }

    /* PHASE C: Tenter déplacement sur X d'abord */
    int nouveau_x = vehicule->posx;
    int nouveau_y = vehicule->posy;

    if (vehicule->posx < target_x)
    {
        nouveau_x = vehicule->posx + 1;
        if (est_cellule_roulable(plan, nouveau_x, nouveau_y))
        {
            vehicule->posx = nouveau_x;
            return;
        }
    }
    else if (vehicule->posx > target_x)
    {
        nouveau_x = vehicule->posx - 1;
        if (est_cellule_roulable(plan, nouveau_x, nouveau_y))
        {
            vehicule->posx = nouveau_x;
            return;
        }
    }

    /* Si X bloqué ou aligné, tenter Y */
    nouveau_x = vehicule->posx; /* Reset X */
    nouveau_y = vehicule->posy;

    if (vehicule->posy < target_y)
    {
        nouveau_y = vehicule->posy + 1;
        if (est_cellule_roulable(plan, nouveau_x, nouveau_y))
        {
            vehicule->posy = nouveau_y;
            return;
        }
    }
    else if (vehicule->posy > target_y)
    {
        nouveau_y = vehicule->posy - 1;
        if (est_cellule_roulable(plan, nouveau_x, nouveau_y))
        {
            vehicule->posy = nouveau_y;
            return;
        }
    }

    /* Si X et Y bloqués, reste sur place */
}

/*
 * PHASE D: Détecte si deux véhicules se chevauchent (collision par AABB).
 * Retourne 1 si collision (chevauchement de rectangles), 0 sinon.
 */

/*
 * Vérifie si la voie est libre dans une direction donnée (système de cédez-le-passage)
 * Retourne 1 si aucun véhicule détecté, 0 sinon
 */

/*
 * Détecte si le véhicule approche d'une intersection.
 * Une intersection = cellule avec au moins 3 directions de flèches possibles dans un rayon donné.
 * Rayon augmenté à 5 pour anticiper et ralentir plus tôt.
 */
static int detecter_intersection(VEHICULE *vehicule, PlanParking *plan)
{
    if (!vehicule || !plan)
        return 0;

    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;

    // Scanner dans un rayon de 5 cellules autour du véhicule pour anticiper
    int rayon = 5;
    int directions_trouvees = 0;
    int a_nord = 0, a_sud = 0, a_est = 0, a_ouest = 0;

    for (int dy = -rayon; dy <= rayon; dy++)
    {
        for (int dx = -rayon; dx <= rayon; dx++)
        {
            int check_x = centre_x + dx;
            int check_y = centre_y + dy;

            if (check_x < 0 || check_y < 0 || check_x >= plan->largeur || check_y >= plan->hauteur)
                continue;

            wchar_t c = plan->plan_statique[check_y][check_x];

            // Compter les flèches uniques
            if (c == L'↑' && !a_nord) { a_nord = 1; directions_trouvees++; }
            if (c == L'↓' && !a_sud) { a_sud = 1; directions_trouvees++; }
            if (c == L'→' && !a_est) { a_est = 1; directions_trouvees++; }
            if (c == L'←' && !a_ouest) { a_ouest = 1; directions_trouvees++; }
        }
    }

    // Intersection si au moins 3 directions différentes
    return (directions_trouvees >= 3);
}

/*
 * Calcule la vitesse adaptative selon le contexte (intersection, sortie, etc.)
 */
static int calculer_vitesse_adaptative(VEHICULE *vehicule, PlanParking *plan)
{
    if (!vehicule || !plan)
        return 1;

    int vitesse_base = vehicule->vitesse;

    // Vérifier si le véhicule est en mode sortie (target = sortie)
    int target = obtenir_target(vehicule);
    int mode_sortie = (target == TARGET_SORTIE);

    // PRIORITE 1 : Accélérer en mode sortie pour évacuer rapidement
    if (mode_sortie)
        return vitesse_base;  // Vitesse normale pour sortir (pas d'accélération excessive)

    // PRIORITE 2 : Ralentir FORTEMENT aux intersections pour éviter les collisions
    if (detecter_intersection(vehicule, plan))
        return 1;  // Vitesse réduite de 50% aux carrefours (base=2, intersection=1)

    // PRIORITE 3 : Vitesse normale ailleurs
    return vitesse_base;
}

/*
 * CORRECTION FINALE: Déplace un véhicule avec parking automatique.
 * Déplacement simple et robuste vers la place cible.
 */
void deplacer_vehicule_parking_auto(VEHICULE *vehicule, PlanParking *plan, l_car *tous_vehicules)
{
    if (!vehicule || !plan || vehicule->etat != '1')
        return;

    /* Obtenir la cible actuelle de cette voiture */
    int target = obtenir_target(vehicule);

    /* Gérer le cas spécial : cible = sortie */
    int mode_sortie = (target == TARGET_SORTIE);

    /* Vérifier si la cible est toujours valide */
    if (!mode_sortie && target >= 0 && target < plan->places_totales)
    {
        /* Si la place est devenue occupée, invalider la cible */
        if (plan->places[target].occupee != 0)
        {
            clear_target(vehicule);
            target = -1;
        }
    }
    else if (!mode_sortie)
    {
        target = -1;
    }

    /* Si pas de cible valide ET pas en mode sortie, en trouver une nouvelle */
    if (target == -1 && !mode_sortie)
    {
        int nouvelle_cible = trouver_place_libre_proche(vehicule, plan);
        if (nouvelle_cible >= 0)
        {
            set_target(vehicule, nouvelle_cible);
            target = nouvelle_cible;

            //         vehicule->posx, vehicule->posy, target,
            //         plan->places[target].colonne, plan->places[target].ligne);
        }
        else
        {
            //         vehicule->posx, vehicule->posy);
        }
    }

    /* FIX RÉGRESSION: Recalculer direction AVANT tenter parking (pour permettre changement vers N/S) */
    /* Si mode sortie, utiliser coordonnées de la sortie. Sinon, si cible valide, utiliser place. Sinon, (-1,-1) */
    int target_x, target_y;
    if (mode_sortie)
    {
        target_x = plan->sortie_x;
        target_y = plan->sortie_y;
    }
    else if (target >= 0)
    {
        target_x = plan->places[target].colonne;
        target_y = plan->places[target].ligne;
    }
    else
    {
        target_x = -1;
        target_y = -1;
    }

    /* ETAPE 2: Logique STICKY - ne recalculer la direction que si nécessaire */
    char ancienne_direction = vehicule->direction;
    char nouvelle_direction = ancienne_direction;

    /* Scanner la zone pour détecter les flèches (depuis le centre du véhicule) */
    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;

    int fleches_zone[4]; // [N, S, E, O]
    compter_fleches_zone(plan, centre_x, centre_y, RAYON_SCAN, fleches_zone);

    /* Ne recalculer la direction que si conditions STICKY remplies */
    if (doit_recalculer_direction(vehicule, plan, fleches_zone, target_x, target_y))
    {
        /* Passer la cible pour que la voiture se dirige vers elle */
        nouvelle_direction = choisir_direction_stable(vehicule, plan, target_x, target_y);
    }

    /* Appliquer la nouvelle direction SEULEMENT si changement ET voie libre */
    if (ancienne_direction != nouvelle_direction)
    {
        // NOUVEAU: Vérifier si la voie est libre dans la nouvelle direction (cédez-le-passage)
        if (voie_libre_direction(vehicule, tous_vehicules, nouvelle_direction, plan))
        {
            // Voie libre, on peut changer de direction
            vehicule->direction = nouvelle_direction;
            orienter_carrosserie(vehicule);

        }
        else
        {
            // Voie occupée, garder l'ancienne direction (attendre)
            nouvelle_direction = ancienne_direction; // Rester dans la direction actuelle

        }
    }

    /* ETAPE 2: Détection automatique du parking (APRÈS recalcul direction) */
    /* Ne pas tenter de se garer si le véhicule est en mode sortie */
    if (!mode_sortie && tenter_parking_automatique(vehicule, plan))
    {
        clear_target(vehicule);
        return; /* Voiture garée automatiquement */
    }

    /* Se déplacer vers la cible OU continuer dans l'allée si pas de cible */
    if (target >= 0 || 1) /* TOUJOURS bouger */
    {
        /* NOUVEAU: Calculer la vitesse adaptative (ralentir aux intersections, accélérer en sortie) */
        int vitesse_effective = calculer_vitesse_adaptative(vehicule, plan);

        /* Calculer la nouvelle position selon la direction choisie */
        int nouveau_x = vehicule->posx;
        int nouveau_y = vehicule->posy;

        switch (nouvelle_direction)
        {
        case 'N':
            nouveau_y -= vitesse_effective;
            break;
        case 'S':
            nouveau_y += vitesse_effective;
            break;
        case 'E':
            nouveau_x += vitesse_effective;
            break;
        case 'O':
            nouveau_x -= vitesse_effective;
            break;
        }

        /* ETAPE 2: Appliquer le déplacement seulement si TOUT le sprite est sur des allées */
        int peut_bouger = peut_deplacer_sur_allee(vehicule, plan, nouveau_x, nouveau_y);

        if (peut_bouger)
        {
            vehicule->posx = nouveau_x;
            vehicule->posy = nouveau_y;

            // NOTE: Correction d'alignement désactivée pendant le mouvement normal
            // La correction ne s'applique qu'au spawn et à la sortie du parking
            // Le système de navigation existant gère bien le mouvement
        }
        else
        {
        }
    }
    else
    {
        //         vehicule->posx, vehicule->posy);
    }
}

/*
 * Corrige l'alignement de la voiture pour la centrer sur une flèche proche
 * VERSION DOUCE : ne corrige que les petits désalignements
 */
static void corriger_alignement_fleche(VEHICULE *vehicule, PlanParking *plan)
{
    if (!vehicule || !plan)
        return;

    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;

    // Vérifier si la cellule actuelle du centre est déjà alignée
    if (centre_x >= 0 && centre_y >= 0 && centre_x < plan->largeur && centre_y < plan->hauteur)
    {
        wchar_t c_centre = plan->plan_statique[centre_y][centre_x];

        // Si déjà sur une flèche ou un espace roulable, pas besoin de corriger
        if (c_centre == L'←' || c_centre == L'→' || c_centre == L'↑' || c_centre == L'↓' ||
            c_centre == L' ' || c_centre == L'.')
        {
            return; // Position acceptable
        }
    }

    // Chercher un meilleur alignement mais SEULEMENT à proximité immédiate (rayon 2)
    int search_radius = 2; // Rayon réduit pour éviter les corrections trop agressives
    int best_x = -1, best_y = -1;
    int best_dist = 999;

    for (int dy = -search_radius; dy <= search_radius; dy++)
    {
        for (int dx = -search_radius; dx <= search_radius; dx++)
        {
            // Ignorer la position actuelle
            if (dx == 0 && dy == 0)
                continue;

            int check_x = centre_x + dx;
            int check_y = centre_y + dy;

            // Vérifier les limites
            if (check_x < 0 || check_y < 0 || check_x >= plan->largeur || check_y >= plan->hauteur)
                continue;

            wchar_t c = plan->plan_statique[check_y][check_x];

            // Chercher une flèche compatible OU un espace roulable
            int compatible = 0;

            // Flèches dans la bonne direction (priorité haute)
            if ((vehicule->direction == 'O' && c == L'←') ||
                (vehicule->direction == 'E' && c == L'→') ||
                (vehicule->direction == 'N' && c == L'↑') ||
                (vehicule->direction == 'S' && c == L'↓'))
            {
                compatible = 2; // Priorité haute
            }
            // Espaces roulables (priorité basse)
            else if (c == L' ' || c == L'.')
            {
                compatible = 1; // Priorité basse
            }

            if (compatible > 0)
            {
                int dist = abs(dx) + abs(dy);

                // Bonus pour alignement perpendiculaire à la direction
                int priorite_bonus = 0;
                if (vehicule->direction == 'O' || vehicule->direction == 'E')
                {
                    if (dy == 0) // Même ligne
                        priorite_bonus = -5;
                }
                else // N ou S
                {
                    if (dx == 0) // Même colonne
                        priorite_bonus = -5;
                }

                // Bonus pour les flèches (compatible == 2)
                if (compatible == 2)
                    priorite_bonus -= 3;

                dist += priorite_bonus;

                if (dist < best_dist)
                {
                    best_dist = dist;
                    best_x = check_x;
                    best_y = check_y;
                }
            }
        }
    }

    // Ne corriger que si on a trouvé une position proche ET meilleure
    if (best_x >= 0 && best_y >= 0 && best_dist <= 2) // Distance max de 2 pour correction
    {
        int nouveau_coin_x = best_x - largeur / 2;
        int nouveau_coin_y = best_y - hauteur / 2;

        // Vérifier que toute la voiture peut tenir à cette position
        int position_ok = 1;
        for (int dy = 0; dy < hauteur; dy++)
        {
            for (int dx = 0; dx < largeur; dx++)
            {
                int test_x = nouveau_coin_x + dx;
                int test_y = nouveau_coin_y + dy;

                if (test_x < 0 || test_y < 0 || test_x >= plan->largeur || test_y >= plan->hauteur)
                {
                    position_ok = 0;
                    break;
                }

                if (!est_cellule_roulable(plan, test_x, test_y))
                {
                    position_ok = 0;
                    break;
                }
            }
            if (!position_ok)
                break;
        }

        // Appliquer SEULEMENT si la correction améliore vraiment la position
        if (position_ok)
        {
            vehicule->posx = nouveau_coin_x;
            vehicule->posy = nouveau_coin_y;
        }
    }
}

// Version publique de la fonction de correction d'alignement
void corriger_alignement_vehicule(VEHICULE *vehicule, PlanParking *plan)
{
    corriger_alignement_fleche(vehicule, plan);
}

/*
 * Vérifie si un véhicule a atteint la sortie
 */
static int vehicule_a_sortie(VEHICULE *vehicule, PlanParking *plan)
{
    if (!vehicule || !plan || vehicule->etat != '1')
        return 0;

    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);
    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;

    // METHODE 1 : Vérifier si une partie du véhicule touche le 'S'
    for (int dy = 0; dy < hauteur; dy++)
    {
        for (int dx = 0; dx < largeur; dx++)
        {
            int check_x = vehicule->posx + dx;
            int check_y = vehicule->posy + dy;

            if (check_x >= 0 && check_x < plan->largeur &&
                check_y >= 0 && check_y < plan->hauteur)
            {
                wchar_t c = plan->plan_statique[check_y][check_x];
                if (c == L'S' || c == L's')
                    return 1;  // Une partie du véhicule est sur la sortie !
            }
        }
    }

    // METHODE 2 : Vérifier la distance aux coordonnées de sortie (fallback élargi)
    int dx = abs(centre_x - plan->sortie_x);
    int dy = abs(centre_y - plan->sortie_y);

    return (dx <= 6 && dy <= 6);
}

/*
 * Déplace tous les véhicules avec parking automatique.
 * Retourne 0 si OK, 1 si collision détectée.
 */
int deplacer_tous_vehicules(l_car *vehicules, PlanParking *plan)
{
    if (!vehicules || est_vide_liste_car(vehicules))
        return 0;

    /* 0. Décrémenter les compteurs de lane lock */
    decrementer_lane_locks();

    /* 0.5 Collecter les véhicules à la sortie (seulement si barrière ouverte) */
    VEHICULE *v_check = vehicules->premier;
    VEHICULE *vehicules_a_supprimer[50];
    int nb_a_supprimer = 0;

    while (v_check != NULL)
    {
        if (vehicule_a_sortie(v_check, plan) && plan->barriere_sortie_ouverte)
        {
            vehicules_a_supprimer[nb_a_supprimer++] = v_check;
        }
        v_check = v_check->NXT;
    }

    /* 1. Déplacer tous les véhicules */
    VEHICULE *curr = vehicules->premier;
    while (curr != NULL)
    {
        if (curr->etat == '1')
        {
            deplacer_vehicule_parking_auto(curr, plan, vehicules);
        }
        curr = curr->NXT;
    }

    /* 1.5 Supprimer les véhicules qui ont atteint la sortie */
    for (int i = 0; i < nb_a_supprimer; i++)
    {
        detruire_vehicule_specifique(vehicules, vehicules_a_supprimer[i]);
    }

    /* 2. Détecter les collisions entre tous les véhicules */
    VEHICULE *v1 = vehicules->premier;
    while (v1 != NULL)
    {
        if (v1->etat == '1')
        {
            VEHICULE *v2 = v1->NXT;
            while (v2 != NULL)
            {
                if (v2->etat == '1' && vehicules_en_collision(v1, v2))
                {
                    /* COLLISION DÉTECTÉE ! */
                    return 1;
                }
                v2 = v2->NXT;
            }
        }
        v1 = v1->NXT;
    }

    return 0; /* Pas de collision */
}