/**
 * ============================================================================
 * PLAN.C - Chargement et gestion du plan de parking
 * ============================================================================
 *
 * RESPONSABILITÉS:
 *   - Chargement du fichier plan.txt avec support UTF-8
 *   - Détection automatique des places de parking (pattern |_|)
 *   - Détection des entrées/sorties (ENTREE, Sortie)
 *   - Détection des barrières ([T] entrée, [B] sortie)
 *   - Détection et indexation des flèches de circulation
 *   - Gestion de l'occupation des places
 *   - Calcul du score et de l'argent total
 *
 * FORMAT DU FICHIER PLAN.TXT:
 *   - Caractères UTF-8 (box-drawing: ═ ║ ╔ ╗ ╚ ╝ ╦ ╩)
 *   - Flèches de circulation: ← → ↑ ↓
 *   - Flèches de virage: ⮠ ⮡ ⮢ ⮣ ⮤ ⮥ ⮦ ⮧
 *   - Places de parking: ╦ (haut) ║ (côtés) ╩ (bas)
 *   - Marqueurs: E/e (entrée), S/s (sortie)
 *
 * DÉTECTION DES PLACES:
 *   - Pattern recherché: |_| ou ╦...╩
 *   - Une place = 3 caractères de large minimum
 *   - Stockées avec leurs coordonnées (ligne, colonne)
 *
 * ============================================================================
 */

#include "plan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// FONCTIONS UTILITAIRES PRIVÉES
// ============================================================================

// Initialise tous les champs d'un PlanParking
static void initialiser_plan_parking(PlanParking *plan)
{
    plan->places_libres = 0;
    plan->places_totales = 0;
    plan->nb_fleches = 0;
    plan->difficulte = 0;  // Mode NORMAL par défaut
    plan->barriere_entree_ouverte = 0;
    plan->barriere_sortie_ouverte = 0;
    plan->entree_x = 0;
    plan->entree_y = 0;
    plan->sortie_x = 0;
    plan->sortie_y = 0;
    plan->borne_entree_x = 0;
    plan->borne_entree_y = 0;
    plan->borne_sortie_x = 0;
    plan->borne_sortie_y = 0;

    // Initialiser toute la grille avec des espaces (wchar_t)
    for (int i = 0; i < MAX_HAUTEUR; i++)
    {
        for (int j = 0; j < MAX_LARGEUR; j++)
        {
            plan->plan_statique[i][j] = L' ';
        }
    }
}

// Détecte et enregistre une place de parking via le caractère ╦ (wchar_t)
static void detecter_place_wchar(wchar_t c, PlanParking *plan, int ligne, int colonne)
{
    // Caractère ╦ (U+2566)
    if (c == L'╦')
    {
        if (plan->places_totales < 50)
        {
            plan->places[plan->places_totales].ligne = ligne;
            plan->places[plan->places_totales].colonne = colonne;
            plan->places[plan->places_totales].occupee = 0;
        }
        plan->places_libres++;
        plan->places_totales++;
    }
}

// Détecte les flèches (wchar_t) et les stocke pour le déplacement
static void detecter_fleche_wchar(wchar_t c, PlanParking *plan, int ligne, int colonne)
{
    char entree = '\0';
    char sortie = '\0';

    // Flèches simples (droites) - acceptent toute direction d'entrée
    if (c == L'←')
        sortie = 'O';       // Ouest
    else if (c == L'→')
        sortie = 'E';       // Est
    else if (c == L'↑')
        sortie = 'N';       // Nord
    else if (c == L'↓')
        sortie = 'S';       // Sud
    // Flèches de virage - direction d'entrée obligatoire
    else if (c == L'⮣')     // Nord → Est
    {
        entree = 'N';
        sortie = 'E';
    }
    else if (c == L'⮧')     // Est → Sud
    {
        entree = 'E';
        sortie = 'S';
    }
    else if (c == L'⮦')     // Ouest → Sud
    {
        entree = 'O';
        sortie = 'S';
    }
    else if (c == L'⮢')     // Nord → Ouest
    {
        entree = 'N';
        sortie = 'O';
    }
    else if (c == L'⮡')     // Sud → Est
    {
        entree = 'S';
        sortie = 'E';
    }
    else if (c == L'⮥')     // Est → Nord
    {
        entree = 'E';
        sortie = 'N';
    }
    else if (c == L'⮤')     // Ouest → Nord
    {
        entree = 'O';
        sortie = 'N';
    }
    else if (c == L'⮠')     // Sud → Ouest
    {
        entree = 'S';
        sortie = 'O';
    }

    // Si une flèche a été détectée, la stocker
    if (sortie != '\0' && plan->nb_fleches < 100)
    {
        plan->fleches[plan->nb_fleches].ligne = ligne;
        plan->fleches[plan->nb_fleches].colonne = colonne;
        plan->fleches[plan->nb_fleches].direction_entree = entree;
        plan->fleches[plan->nb_fleches].direction_sortie = sortie;
        plan->nb_fleches++;
    }
}

// Détecte les entrées et sorties
// Détecte les entrées et sorties (wchar_t)
static void detecter_entree_sortie_wchar(const wchar_t *ligne, size_t pos, size_t len,
                                          PlanParking *plan, int ligne_courante)
{
    // PRIORITE 1: Détecter un 'E' SEUL (marqueur d'entrée posé par l'utilisateur)
    if (ligne[pos] == L'E')
    {
        // Vérifier que ce n'est PAS le début du mot "ENTREE"
        int est_entree = (pos + 6 <= len && wcsncmp(&ligne[pos], L"ENTREE", 6) == 0);

        if (!est_entree)
        {
            // C'est un 'E' seul → position d'entrée !
            // Ne garder que le PREMIER 'E' trouvé
            if (plan->entree_x == 0)
            {
                plan->entree_x = pos;
                plan->entree_y = ligne_courante;
            }
            return; // Priorité absolue au 'E' seul
        }
    }

    // PRIORITE 3: Détection du mot "ENTREE" (fallback)
    // Seulement si aucune entrée n'a été trouvée
    if (plan->entree_x == 0 && pos + 6 <= len && wcsncmp(&ligne[pos], L"ENTREE", 6) == 0)
    {
        // Chercher une cellule roulable après "ENTREE"
        int offset_x = pos + 7; // Après "ENTREE "

        while (offset_x < (int)len)
        {
            wchar_t c = ligne[offset_x];
            if (c == L' ' || c == L'←' || c == L'→' || c == L'↑' || c == L'↓')
            {
                plan->entree_x = offset_x;
                plan->entree_y = ligne_courante;
                break;
            }
            offset_x++;
            if (offset_x > (int)pos + 17)
                break;
        }

        if (plan->entree_x == 0)
        {
            plan->entree_x = pos + 7;
            plan->entree_y = ligne_courante;
        }
    }
    // Détection de la sortie
    else if (pos + 6 <= len && wcsncmp(&ligne[pos], L"Sortie", 6) == 0)
    {
        // Même correction pour la sortie
        int offset_x = pos + 7;
        while (offset_x < (int)len)
        {
            wchar_t c = ligne[offset_x];
            if (c == L' ' || c == L'←' || c == L'→' || c == L'↑' || c == L'↓')
            {
                plan->sortie_x = offset_x;
                plan->sortie_y = ligne_courante;
                break;
            }
            offset_x++;
            if (offset_x > (int)pos + 17)
                break;
        }

        if (plan->sortie_x == 0)
        {
            plan->sortie_x = pos + 7;
            plan->sortie_y = ligne_courante;
        }
    }
    // Détection d'un 'S' majuscule isolé comme marqueur de sortie
    else if (ligne[pos] == L'S')
    {
        // Vérifier que c'est bien un 'S' isolé (entouré d'espaces ou flèches)
        int est_isole = 1;
        if (pos > 0)
        {
            wchar_t avant = ligne[pos - 1];
            if (!(avant == L' ' || avant == L'←' || avant == L'→' || avant == L'↑' || avant == L'↓'))
                est_isole = 0;
        }
        if (pos + 1 < len && est_isole)
        {
            wchar_t apres = ligne[pos + 1];
            if (!(apres == L' ' || apres == L'←' || apres == L'→' || apres == L'↑' || apres == L'↓' || apres == L'\0'))
                est_isole = 0;
        }

        // Si c'est un 'S' isolé, le marquer comme sortie
        if (est_isole)
        {
            plan->sortie_x = pos;
            plan->sortie_y = ligne_courante;
        }
    }
    // Détection borne d'entrée [T]
    else if (ligne[pos] == L'[' && pos + 1 < len && ligne[pos + 1] == L'T')
    {
        plan->borne_entree_x = pos;
        plan->borne_entree_y = ligne_courante;
    }
    // Détection borne de sortie [P]
    else if (ligne[pos] == L'[' && pos + 1 < len && ligne[pos + 1] == L'P')
    {
        plan->borne_sortie_x = pos;
        plan->borne_sortie_y = ligne_courante;
    }
}

// Traite une ligne du fichier de plan
static void traiter_ligne_plan(const char *ligne, PlanParking *plan,
                                int ligne_courante, int *largeur_max)
{
    // Convertir la ligne UTF-8 en wchar_t
    wchar_t wligne[MAX_LARGEUR];
    size_t len = mbstowcs(wligne, ligne, MAX_LARGEUR);

    if (len == (size_t)-1)
    {
        // Erreur de conversion, ligne ignorée
        return;
    }

    // Retirer le \n si présent
    if (len > 0 && wligne[len-1] == L'\n')
    {
        wligne[len-1] = L'\0';
        len--;
    }

    if ((int)len > *largeur_max)
    {
        *largeur_max = (int)len;
    }

    // Copier dans le plan et détecter les éléments
    for (size_t i = 0; i < len && i < MAX_LARGEUR; i++)
    {
        wchar_t c = wligne[i];

        // Stocker le caractère
        plan->plan_statique[ligne_courante][i] = c;

        // Détecter les places de parking
        detecter_place_wchar(c, plan, ligne_courante, i);

        // Détecter les flèches directionnelles
        detecter_fleche_wchar(c, plan, ligne_courante, i);

        // Détecter entrées et sorties
        detecter_entree_sortie_wchar(wligne, i, len, plan, ligne_courante);
    }
}

// Nettoie le retour à la ligne
static void nettoyer_retour_ligne(char *ligne)
{
    int len = (int)strlen(ligne);
    if (len > 0 && ligne[len - 1] == '\n')
    {
        ligne[len - 1] = '\0';
    }
}

// ============================================================================
// CHARGEMENT ET DESTRUCTION DU PLAN
// ============================================================================

PlanParking *charger_plan(const char *fichier_plan)
{
    FILE *fichier = fopen(fichier_plan, "r");
    if (!fichier)
    {
        printf("Erreur : impossible d'ouvrir le fichier %s\n", fichier_plan);
        return NULL;
    }

    PlanParking *plan = malloc(sizeof(PlanParking));
    if (!plan)
    {
        fclose(fichier);
        return NULL;
    }

    initialiser_plan_parking(plan);

    // Lecture et traitement du fichier
    char ligne[MAX_LIGNE];
    int ligne_courante = 0;
    int largeur_max = 0;

    while (fgets(ligne, MAX_LIGNE, fichier) && ligne_courante < MAX_HAUTEUR)
    {
        nettoyer_retour_ligne(ligne);
        traiter_ligne_plan(ligne, plan, ligne_courante, &largeur_max);
        ligne_courante++;
    }

    fclose(fichier);

    plan->hauteur = ligne_courante;
    plan->largeur = largeur_max;

    // Créer matrice d'occupation
    plan->matrice_occupation = creer_matrice(plan->hauteur, plan->largeur);
    if (!plan->matrice_occupation)
    {
        free(plan);
        return NULL;
    }

    initialiser_matrice_depuis_plan(plan);

    // Initialiser l'état du jeu
    plan->argent_total = 0;
    plan->score = 0;
    plan->vehicules_servis = 0;
    plan->vehicules_perdus = 0;
    plan->high_score = 0;

    return plan;
}

void detruire_plan(PlanParking **plan)
{
    if (*plan)
    {
        if ((*plan)->matrice_occupation)
        {
            detruire_matrice(&((*plan)->matrice_occupation));
        }
        free(*plan);
        *plan = NULL;
    }
}

// ============================================================================
// GESTION DE LA MATRICE D'OCCUPATION
// ============================================================================

void initialiser_matrice_depuis_plan(PlanParking *plan)
{
    if (!plan || !plan->matrice_occupation)
        return;

    for (int i = 0; i < plan->hauteur; i++)
    {
        for (int j = 0; j < plan->largeur; j++)
        {
            char c = plan->plan_statique[i][j];
            if (c == '#' || c == '|' || c == '_' || c == PLACE_OCCUPEE)
            {
                remplir_case(plan->matrice_occupation, i, j);
            }
            else
            {
                liberer_case(plan->matrice_occupation, i, j);
            }
        }
    }
}

// ============================================================================
// GESTION DES VÉHICULES SUR LE PLAN
// ============================================================================

void placer_vehicules_sur_plan(PlanParking *plan, l_car *liste_vehicules)
{
    (void)plan;
    (void)liste_vehicules;
}

int est_position_libre(PlanParking *plan, int x, int y)
{
    if (!plan || x < 0 || x >= plan->hauteur || y < 0 || y >= plan->largeur)
    {
        return 0;
    }
    return (plan->matrice_occupation->tab[x][y].o == 0);
}

// ============================================================================
// GESTION DES PLACES DE PARKING
// ============================================================================

void occuper_place_parking(PlanParking *plan, int x, int y)
{
    if (!plan || x < 0 || x >= plan->hauteur || y < 0 || y >= plan->largeur)
    {
        return;
    }

    if (plan->plan_statique[x][y] == PLACE_LIBRE)
    {
        plan->plan_statique[x][y] = PLACE_OCCUPEE;
        remplir_case(plan->matrice_occupation, x, y);
        plan->places_libres--;
    }
}

void liberer_place_parking(PlanParking *plan, int x, int y)
{
    if (!plan || x < 0 || x >= plan->hauteur || y < 0 || y >= plan->largeur)
    {
        return;
    }

    if (plan->plan_statique[x][y] == PLACE_OCCUPEE)
    {
        plan->plan_statique[x][y] = PLACE_LIBRE;
        liberer_case(plan->matrice_occupation, x, y);
        plan->places_libres++;
    }
}

int trouver_place_a_position(PlanParking *plan, int ligne, int colonne)
{
    if (!plan)
        return -1;

    for (int i = 0; i < plan->places_totales; i++)
    {
        // Vérifier si la position est dans la zone de la place (3 lignes de hauteur)
        if (plan->places[i].colonne == colonne &&
            ligne >= plan->places[i].ligne &&
            ligne <= plan->places[i].ligne + 2)
        {
            return i;
        }
    }

    return -1;
}

void marquer_place_occupee(PlanParking *plan, int index_place)
{
    if (!plan || index_place < 0 || index_place >= plan->places_totales)
        return;

    if (!plan->places[index_place].occupee)
    {
        plan->places[index_place].occupee = 1;
        plan->places_libres--;
    }
}

void marquer_place_libre(PlanParking *plan, int index_place)
{
    if (!plan || index_place < 0 || index_place >= plan->places_totales)
        return;

    if (plan->places[index_place].occupee)
    {
        plan->places[index_place].occupee = 0;
        plan->places_libres++;
    }
}

// ============================================================================
// GESTION DES BARRIÈRES
// ============================================================================

void basculer_barriere_entree(PlanParking *plan)
{
    if (!plan)
        return;
    plan->barriere_entree_ouverte = !plan->barriere_entree_ouverte;
}

void basculer_barriere_sortie(PlanParking *plan)
{
    if (!plan)
        return;
    plan->barriere_sortie_ouverte = !plan->barriere_sortie_ouverte;
}

// ============================================================================
// AFFICHAGE (MODE TERMINAL)
// ============================================================================

void afficher_infos_parking(PlanParking *plan)
{
    if (!plan)
        return;

    printf("\n");
    printf("%s===== PARKING INFO =====%s\n", CYAN, RESET_COLOR);
    printf("Places: %s%d%s/%d\n", VERT, plan->places_libres, RESET_COLOR, plan->places_totales);
    printf("Entree: %s%s%s\n", plan->barriere_entree_ouverte ? VERT : ROUGE,
           plan->barriere_entree_ouverte ? "OUVERTE" : "FERMEE", RESET_COLOR);
    printf("Sortie: %s%s%s\n", plan->barriere_sortie_ouverte ? VERT : ROUGE,
           plan->barriere_sortie_ouverte ? "OUVERTE" : "FERMEE", RESET_COLOR);
    printf("%s========================%s\n", CYAN, RESET_COLOR);
}

char *obtenir_couleur_caractere(char c, int est_place_libre)
{
    switch (c)
    {
    case '#':
    case '=':
        return GRIS;
    case 'P':
        return est_place_libre ? BG_VERT : BG_ROUGE;
    case '|':
    case '_':
        return est_place_libre ? VERT : ROUGE;
    case '>':
    case '<':
    case '^':
    case 'v':
        return CYAN;
    case 'E':
        return BG_JAUNE;
    case 'T':
    case '[':
    case ']':
        return JAUNE;
    case 'A':
    case 'L':
        return CYAN;
    default:
        return RESET_COLOR;
    }
}

void afficher_plan_couleur(PlanParking *plan)
{
    if (!plan)
        return;

    system("clear");

    printf("\n%s", BG_CYAN);
    printf("                    SIMULATEUR DE PARKING                    ");
    printf("%s\n\n", RESET_COLOR);

    for (int i = 0; i < plan->hauteur; i++)
    {
        for (int j = 0; j < plan->largeur; j++)
        {
            char c = plan->plan_statique[i][j];

            // Places |P| en vert
            int est_place_libre = (c == 'P' && j > 0 && plan->plan_statique[i][j - 1] == '|');

            printf("%s%c%s", obtenir_couleur_caractere(c, est_place_libre), c, RESET_COLOR);
        }
        printf("\n");
    }

    afficher_infos_parking(plan);
}

void afficher_plan_unicode(const char *fichier_plan)
{
    FILE *fichier = fopen(fichier_plan, "r");
    if (!fichier)
    {
        printf("Erreur : impossible d'ouvrir le fichier %s\n", fichier_plan);
        return;
    }

    system("clear");

    printf("\n%s", BG_CYAN);
    printf("                    SIMULATEUR DE PARKING                    ");
    printf("%s\n\n", RESET_COLOR);

    char ligne[MAX_LIGNE];
    while (fgets(ligne, MAX_LIGNE, fichier))
    {
        // Afficher la ligne complète (supporte UTF-8)
        printf("%s%s%s", GRIS, ligne, RESET_COLOR);
    }

    fclose(fichier);
}
