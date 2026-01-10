#include "jeu.h"
#include "affichage.h"
#include "plan.h"
#include "mouvement.h"
#include "liste_car.h"
#include <ncurses.h>
#include <stdlib.h>

// Déclaration de est_cellule_roulable (définie dans mouvement.c)
extern int est_cellule_roulable_externe(PlanParking *plan, int x, int y);

// Compteur global de frames pour le suivi du temps de stationnement
unsigned long int global_frame_counter = 0;

// Constantes de timeout pour la file d'attente
#define TIMEOUT_ATTENTE 300     // 30 secondes (300 frames à 100ms)
#define PENALITE_TIMEOUT 200    // Pénalité de score

// Constantes de spawn adaptatif - MODE NORMAL
#define SPAWN_RAPIDE_NORMAL 30         // 3 secondes (file peu remplie)
#define SPAWN_MOYEN_NORMAL 50          // 5 secondes (file moyennement remplie)
#define SPAWN_LENT_NORMAL 80           // 8 secondes (file presque pleine)

// Constantes de spawn adaptatif - MODE HARD
#define SPAWN_RAPIDE_HARD 20           // 2 secondes (plus rapide)
#define SPAWN_MOYEN_HARD 35            // 3.5 secondes
#define SPAWN_LENT_HARD 55             // 5.5 secondes (moins de ralentissement)

// Timeout et pénalités - MODE NORMAL
#define TIMEOUT_NORMAL 300             // 30 secondes
#define PENALITE_NORMAL 200            // -200 points

// Timeout et pénalités - MODE HARD
#define TIMEOUT_HARD 200               // 20 secondes (plus court)
#define PENALITE_HARD 300              // -300 points (plus sévère)

/*
 * Calcule l'intervalle de spawn adaptatif selon le remplissage de la file et la difficulté.
 * Plus la file est pleine, plus le spawn est ralenti pour éviter la congestion.
 */
static int calculer_spawn_interval(FileAttenteEntree *file, int difficulte)
{
    if (!file)
        return difficulte ? SPAWN_RAPIDE_HARD : SPAWN_RAPIDE_NORMAL;

    int longueur = file->longueur_attente;

    if (difficulte) // Mode HARD
    {
        if (longueur <= 3)
            return SPAWN_RAPIDE_HARD;
        else if (longueur <= 6)
            return SPAWN_MOYEN_HARD;
        else
            return SPAWN_LENT_HARD;
    }
    else // Mode NORMAL
    {
        if (longueur <= 3)
            return SPAWN_RAPIDE_NORMAL;
        else if (longueur <= 6)
            return SPAWN_MOYEN_NORMAL;
        else
            return SPAWN_LENT_NORMAL;
    }
}

// DIAGNOSTIC: Fonction de vérification complète du plan
static void diagnostic_plan(PlanParking *plan)
{
    FILE *diag = fopen("diagnostic_plan.log", "w");
    if (!diag)
        return;

    fprintf(stderr, "[DEBUG] diagnostic_plan: debut\n");

    if (!plan)
    {
        fprintf(diag, "ERREUR: plan est NULL\n");
        fclose(diag);
        return;
    }

    fprintf(diag, "========================================\n");
    fprintf(diag, "DIAGNOSTIC COMPLET DU PLAN\n");
    fprintf(diag, "========================================\n\n");
    fprintf(diag, "Dimensions plan: largeur=%d hauteur=%d\n\n", plan->largeur, plan->hauteur);

    // A) Vérifier l'entrée
    fprintf(stderr, "[DEBUG] Section A: verification entree\n");
    fprintf(diag, "A) VERIFICATION ENTREE\n");
    fprintf(diag, "   entree_x = %d (colonne)\n", plan->entree_x);
    fprintf(diag, "   entree_y = %d (ligne)\n", plan->entree_y);

    if (plan->entree_x >= 0 && plan->entree_x < plan->largeur &&
        plan->entree_y >= 0 && plan->entree_y < plan->hauteur)
    {
        wchar_t c = plan->plan_statique[plan->entree_y][plan->entree_x];
        fprintf(diag, "   wchar @ [%d][%d] = U+%04X (%lc)\n",
                plan->entree_y, plan->entree_x, (int)c, c);

        // SAFE: Ne pas appeler est_cellule_roulable_externe pour éviter segfault
        // int roulable = est_cellule_roulable_externe(plan, plan->entree_x, plan->entree_y);
        // fprintf(diag, "   est_cellule_roulable(%d,%d) = %d\n",
        //         plan->entree_x, plan->entree_y, roulable);

        fprintf(diag, "   est_cellule_roulable: SKIP (debug)\n");
    }
    else
    {
        fprintf(diag, "   *** ERREUR: ENTREE HORS LIMITES ***\n");
    }
    fprintf(stderr, "[DEBUG] Section A: OK\n");

    // A2) Vérifier la sortie
    fprintf(stderr, "[DEBUG] Section A2: verification sortie\n");
    fprintf(diag, "\nA2) VERIFICATION SORTIE\n");
    fprintf(diag, "   sortie_x = %d (colonne)\n", plan->sortie_x);
    fprintf(diag, "   sortie_y = %d (ligne)\n", plan->sortie_y);

    if (plan->sortie_x >= 0 && plan->sortie_x < plan->largeur &&
        plan->sortie_y >= 0 && plan->sortie_y < plan->hauteur)
    {
        wchar_t c = plan->plan_statique[plan->sortie_y][plan->sortie_x];
        fprintf(diag, "   wchar @ [%d][%d] = U+%04X (%lc)\n",
                plan->sortie_y, plan->sortie_x, (int)c, c);

        if (c == L'S' || c == L's')
            fprintf(diag, "   *** SORTIE CORRECTEMENT DETECTEE ('S') ***\n");
        else
            fprintf(diag, "   ATTENTION: caractere a la sortie n'est pas 'S'\n");
    }
    else
    {
        fprintf(diag, "   *** ERREUR: SORTIE HORS LIMITES OU NON DETECTEE ***\n");
        fprintf(diag, "   CONSEIL: Ajoutez un 'S' majuscule isole dans votre plan.txt\n");
    }
    fprintf(stderr, "[DEBUG] Section A2: OK\n");

    // B) Vérifier les places
    fprintf(stderr, "[DEBUG] Section B: verification places\n");
    fprintf(diag, "\nB) VERIFICATION PLACES (total=%d)\n", plan->places_totales);
    fprintf(diag, "   Format: i | (colonne,ligne) | wchar plan[ligne][colonne]\n");
    fprintf(diag, "   ----------------------------------------------------------------\n");

    if (plan->matrice_occupation)
    {
        fprintf(diag, "   Matrice occupation: m=%d n=%d\n",
                plan->matrice_occupation->m, plan->matrice_occupation->n);
    }
    else
    {
        fprintf(diag, "   Matrice occupation: NULL\n");
    }

    for (int i = 0; i < plan->places_totales && i < 50; i++)
    {
        int px = plan->places[i].colonne;
        int py = plan->places[i].ligne;

        wchar_t c = L'?';

        if (px >= 0 && px < plan->largeur && py >= 0 && py < plan->hauteur)
        {
            c = plan->plan_statique[py][px];
        }

        fprintf(diag, "   %2d | (%3d,%3d) | U+%04X (%lc) | occupee=%d\n",
                i, px, py, (int)c, c, plan->places[i].occupee);

        // Vérifier que c'est bien un symbole de place
        if (c != L'╦' && c != L'P' && c != L' ')
        {
            fprintf(diag, "      *** WARNING: caractere inattendu pour une place ***\n");
        }
    }
    fprintf(stderr, "[DEBUG] Section B: OK\n");

    // C) SKIP pour éviter segfault
    fprintf(stderr, "[DEBUG] Section C: SKIP (debug)\n");
    fprintf(diag, "\nC) VERIFICATION MATRICE vs PLAN: SKIP (debug)\n");

    fprintf(diag, "\n========================================\n");
    fprintf(diag, "FIN DIAGNOSTIC\n");
    fprintf(diag, "========================================\n");

    fprintf(stderr, "[DEBUG] diagnostic_plan: fin\n");
    fclose(diag);
}

// CORRECTION: Vérifier si l'entrée est libre (avec zone de sécurité)
static int entree_libre(l_car *vehicules, int entree_x, int entree_y, FILE *log)
{
    if (!vehicules || est_vide_liste_car(vehicules))
    {
        if (log)
            fprintf(log, "   entree_libre: liste vide -> OK\n");
        return 1;
    }

    VEHICULE *v = vehicules->premier;
    while (v != NULL)
    {
        if (v->etat == '1')
        {
            /* PATCH A: Utiliser le CENTRE du véhicule, pas le coin */
            int largeur, hauteur;
            obtenir_dimensions_vehicule(v, &largeur, &hauteur);
            int centre_x = v->posx + largeur / 2;
            int centre_y = v->posy + hauteur / 2;

            /* Zone de sécurité stricte : 10x6 cellules autour de l'entrée (GOULET) */
            int dist_x = abs(centre_x - entree_x);
            int dist_y = abs(centre_y - entree_y);

            if (dist_x < 10 && dist_y < 6)
            {
                if (log)
                {
                    fprintf(log, "   [SPAWN_SKIP] voiture dans zone goulet\n");
                    fprintf(log, "      pos=(%d,%d) centre=(%d,%d) dist=(%d,%d) -> REFUSE\n",
                            v->posx, v->posy, centre_x, centre_y, dist_x, dist_y);
                }
                return 0; // Entrée occupée
            }
        }
        v = v->NXT;
    }

    if (log)
        fprintf(log, "   entree_libre: aucune voiture proche de (%d,%d) -> OK\n", entree_x, entree_y);
    return 1; // Entrée libre
}

// Traitement d'entrée des véhicules depuis la file d'attente
static void traiter_entree_vehicules(PlanParking *plan, FileAttenteEntree *file_attente,
                                      l_car *vehicules, unsigned long frame)
{
    // Vérifier conditions d'entrée
    if (!plan->barriere_entree_ouverte)
        return;  // Barrière fermée

    if (file_attente_est_vide(file_attente))
        return;  // Pas de véhicules en attente

    if (plan->places_libres <= 0)
        return;  // Parking plein

    if (!entree_libre(vehicules, plan->entree_x, plan->entree_y, NULL))
        return;  // Entrée bloquée

    // Faire entrer le premier véhicule de la file
    VEHICULE *v = retirer_de_file_attente(file_attente);
    if (!v)
        return;

    // Positionner à l'entrée
    v->posx = plan->entree_x;
    v->posy = plan->entree_y;
    v->direction = 'O';  // Direction Ouest par défaut
    v->etat = '1';       // Actif
    v->tps = frame;

    // Orienter la carrosserie selon la direction
    orienter_carrosserie(v);

    // Ajouter à la liste active
    ajouter_queue_liste_car(v, vehicules);

    // Corriger l'alignement sur une flèche
    corriger_alignement_vehicule(v, plan);

    // PAIEMENT ET SCORE
    const unsigned long PRIX_ENTREE = 500;  // 5.00€ en centimes
    plan->argent_total += PRIX_ENTREE;
    plan->score += 100;
    plan->vehicules_servis++;
}

// Traitement du timeout des véhicules en attente
static void traiter_timeout_attente(PlanParking *plan, FileAttenteEntree *file_attente,
                                     unsigned long frame, int *notification_timeout)
{
    if (file_attente_est_vide(file_attente))
        return;

    *notification_timeout = 0;

    VEHICULE *v = file_attente->premier_attente;
    VEHICULE *prev = NULL;

    // Choisir timeout et pénalité selon difficulté
    unsigned long timeout = plan->difficulte ? TIMEOUT_HARD : TIMEOUT_NORMAL;
    unsigned long penalite = plan->difficulte ? PENALITE_HARD : PENALITE_NORMAL;

    while (v != NULL)
    {
        unsigned long temps_ecoule = frame - v->temps_attente;

        if (temps_ecoule >= timeout)
        {
            VEHICULE *next = v->NXT;

            // Retirer de la file
            if (prev == NULL)
            {
                file_attente->premier_attente = next;
            }
            else
            {
                prev->NXT = next;
            }

            if (v == file_attente->dernier_attente)
            {
                file_attente->dernier_attente = prev;
            }

            file_attente->longueur_attente--;

            // Statistiques et pénalité
            plan->vehicules_perdus++;
            plan->score = (plan->score > penalite) ?
                          (plan->score - penalite) : 0;
            *notification_timeout = 1;

            detruire_vehicule(&v);
            v = next;
        }
        else
        {
            prev = v;
            v = v->NXT;
        }
    }
}

// Fonction helper pour vérifier si tout le sprite d'une voiture peut tenir à une position
static int position_valide_pour_vehicule(PlanParking *plan, int coin_x, int coin_y, int largeur, int hauteur)
{
    /* MARGE DE SÉCURITÉ: 3 cellules de chaque côté pour éviter les collisions aux bords */
    const int MARGE_SECURITE = 3;

    // Vérifier les limites avec marge de sécurité
    if (coin_x < MARGE_SECURITE || coin_y < MARGE_SECURITE)
        return 0;
    if (coin_x + largeur >= plan->largeur - MARGE_SECURITE ||
        coin_y + hauteur >= plan->hauteur - MARGE_SECURITE)
        return 0;

    // Vérifier que toutes les cellules du sprite sont roulables
    for (int dy = 0; dy < hauteur; dy++)
    {
        for (int dx = 0; dx < largeur; dx++)
        {
            int check_x = coin_x + dx;
            int check_y = coin_y + dy;

            // Vérifier que c'est roulable
            if (!est_cellule_roulable_externe(plan, check_x, check_y))
                return 0;
        }
    }
    return 1; // Toutes les cellules sont OK
}

// Fonction helper pour trouver une position valide pour placer le véhicule sur la route
static int trouver_position_route_proche(PlanParking *plan, int x_depart, int y_depart,
                                          int largeur, int hauteur,
                                          int *coin_x_sortie, int *coin_y_sortie, char *direction_sortie)
{
    // Chercher dans un rayon croissant autour de la position de départ
    for (int rayon = 1; rayon <= 15; rayon++)
    {
        for (int dx = -rayon; dx <= rayon; dx++)
        {
            for (int dy = -rayon; dy <= rayon; dy++)
            {
                if (abs(dx) + abs(dy) > rayon)
                    continue; // Distance Manhattan

                int centre_x = x_depart + dx;
                int centre_y = y_depart + dy;

                // Vérifier que le centre est dans les limites
                if (centre_x < 0 || centre_y < 0 || centre_x >= plan->largeur || centre_y >= plan->hauteur)
                    continue;

                wchar_t c = plan->plan_statique[centre_y][centre_x];

                // Chercher une cellule avec une flèche de circulation
                if (c == L'←' || c == L'→' || c == L'↑' || c == L'↓')
                {
                    // Calculer le coin haut-gauche pour centrer le véhicule sur cette flèche
                    int coin_x = centre_x - largeur / 2;
                    int coin_y = centre_y - hauteur / 2;

                    // Vérifier que tout le sprite peut tenir
                    if (position_valide_pour_vehicule(plan, coin_x, coin_y, largeur, hauteur))
                    {
                        *coin_x_sortie = coin_x;
                        *coin_y_sortie = coin_y;

                        // Déterminer la direction selon la flèche
                        if (c == L'←')
                            *direction_sortie = 'O';
                        else if (c == L'→')
                            *direction_sortie = 'E';
                        else if (c == L'↑')
                            *direction_sortie = 'N';
                        else if (c == L'↓')
                            *direction_sortie = 'S';

                        return 1; // Position valide trouvée !
                    }
                }
            }
        }
    }
    return 0; // Aucune position valide trouvée
}

// Fonction pour vérifier et réactiver les véhicules garés depuis trop longtemps
static void verifier_et_reactiver_vehicules_gares(l_car *vehicules, PlanParking *plan, unsigned long int current_frame)
{
    if (!vehicules || !plan || est_vide_liste_car(vehicules))
        return;

    const unsigned long int DUREE_MIN = 300; // 5 secondes à 100ms/frame
    const unsigned long int DUREE_MAX = 600; // 10 secondes

    VEHICULE *v = vehicules->premier;
    while (v != NULL)
    {
        if (v->etat == '0') // Véhicule garé
        {
            unsigned long int duree_parking = current_frame - v->tps;

            // Durée cible aléatoire basée sur l'adresse du véhicule
            unsigned long int duree_cible = DUREE_MIN +
                                            ((unsigned long int)v % (DUREE_MAX - DUREE_MIN + 1));

            if (duree_parking >= duree_cible)
            {
                // Libérer la place de parking d'abord
                int largeur, hauteur;
                obtenir_dimensions_vehicule(v, &largeur, &hauteur);
                int centre_x = v->posx + largeur / 2;
                int centre_y = v->posy + hauteur / 2;

                for (int i = 0; i < plan->places_totales; i++)
                {
                    int dx = abs(plan->places[i].colonne - centre_x);
                    int dy = abs(plan->places[i].ligne - centre_y);

                    if (dx <= 2 && dy <= 2 && plan->places[i].occupee)
                    {
                        marquer_place_libre(plan, i);
                        break;
                    }
                }

                // Repositionner la voiture sur une route proche avec alignement parfait
                int coin_x, coin_y;
                char nouvelle_direction;
                if (trouver_position_route_proche(plan, centre_x, centre_y, largeur, hauteur,
                                                   &coin_x, &coin_y, &nouvelle_direction))
                {
                    // Placer la voiture parfaitement alignée sur la route
                    v->posx = coin_x;
                    v->posy = coin_y;
                    v->direction = nouvelle_direction;
                    orienter_carrosserie(v);
                }

                // Réactiver le véhicule
                v->etat = '1';

                // Marquer le véhicule pour qu'il aille vers la sortie
                marquer_vehicule_en_sortie(v);
            }
        }
        v = v->NXT;
    }
}

void executer_boucle_jeu(PlanParking *plan, l_car *vehicules, FileAttenteEntree *file_attente)
{
    if (!plan || !vehicules || !file_attente)
        return;

    // Flag pour affichage warning timeout
    int notification_timeout = 0;

    // DIAGNOSTIC: Exécuter une seule fois au lancement
    static int diagnostic_done = 0;
    if (!diagnostic_done)
    {
        diagnostic_plan(plan);
        diagnostic_done = 1;
    }

    // Initialiser le viewport
    Viewport viewport;
    calculer_viewport(plan, vehicules, &viewport);

    int running = 1;
    int frame_counter = 0;
    int log_frame_0_done = 0;
    nodelay(stdscr, TRUE);

    // PHASE A: Variables pour spawn cadencé avec adaptation dynamique
    static int spawn_cd = 0;

    // E) Ouvrir fichier de log spawn
    FILE *spawn_log = fopen("spawn.log", "w");
    if (spawn_log)
    {
        fprintf(spawn_log, "=== LOG SPAWN ===\n");
        fprintf(spawn_log, "Convention: x=colonne, y=ligne\n\n");
    }

    while (running)
    {
        // 1. Lire input
        int ch = getch();
        if (ch == 'q' || ch == 'Q')
        {
            running = 0;
        }
        else if (ch == 'e' || ch == 'E')
        {
            basculer_barriere_entree(plan);
        }
        else if (ch == 's' || ch == 'S')
        {
            basculer_barriere_sortie(plan);
        }

        // 1.6. Traiter l'entrée des véhicules depuis la file d'attente
        traiter_entree_vehicules(plan, file_attente, vehicules, global_frame_counter);

        // 1.5. PHASE A + E: Spawn cadencé de voitures avec logs détaillés
        // Nouvelle logique: spawn dans la file d'attente
        if (spawn_cd > 0)
        {
            spawn_cd--;
        }
        else
        {
            // E) Log tentative de spawn
            if (spawn_log)
            {
                fprintf(spawn_log, "Frame %d: Tentative spawn dans file d'attente\n", frame_counter);
                fprintf(spawn_log, "   file_attente = %d/%d\n",
                        file_attente->longueur_attente, file_attente->longueur_max);
                fflush(spawn_log);
            }

            // Spawner dans la file si pas pleine
            if (!file_attente_est_pleine(file_attente))
            {
                VEHICULE *nouvelle = creer_voiture_aleatoire(plan);
                if (nouvelle)
                {
                    if (ajouter_a_file_attente(file_attente, nouvelle, global_frame_counter))
                    {
                        if (spawn_log)
                        {
                            fprintf(spawn_log, "   -> AJOUT FILE REUSSI\n");
                            fprintf(spawn_log, "      file_attente maintenant = %d/%d\n\n",
                                    file_attente->longueur_attente, file_attente->longueur_max);
                            fflush(spawn_log);
                        }
                        // Spawn adaptatif : ajuster l'intervalle selon le remplissage de la file et la difficulté
                        spawn_cd = calculer_spawn_interval(file_attente, plan->difficulte);
                    }
                    else
                    {
                        if (spawn_log)
                        {
                            fprintf(spawn_log, "   -> ECHEC AJOUT FILE\n\n");
                            fflush(spawn_log);
                        }
                        detruire_vehicule(&nouvelle);
                    }
                }
                else
                {
                    if (spawn_log)
                    {
                        fprintf(spawn_log, "   -> ECHEC: creer_voiture_aleatoire a echoue\n\n");
                        fflush(spawn_log);
                    }
                }
            }
            else
            {
                if (spawn_log)
                {
                    fprintf(spawn_log, "   -> REFUSE: file d'attente pleine\n\n");
                    fflush(spawn_log);
                }
                // File pleine : ralentir fortement le spawn
                spawn_cd = calculer_spawn_interval(file_attente, plan->difficulte);
            }
        }

        // 2. Déplacer les vehicules et détecter collisions
        global_frame_counter++; // Incrémenter le compteur global de temps
        frame_counter++;
        if (frame_counter >= 5)
        {
            // Vérifier et réactiver les véhicules garés depuis trop longtemps
            verifier_et_reactiver_vehicules_gares(vehicules, plan, global_frame_counter);

            int collision = deplacer_tous_vehicules(vehicules, plan);
            if (collision)
            {
                // GAME OVER - COLLISION !
                clear();

                // Mettre à jour high score
                if (plan->score > plan->high_score)
                {
                    plan->high_score = plan->score;
                }

                // Titre
                attron(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);
                mvprintw(LINES / 2 - 6, (COLS - 30) / 2, "*** COLLISION DETECTEE ***");
                attroff(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);

                attron(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
                mvprintw(LINES / 2 - 4, (COLS - 20) / 2, "GAME OVER");
                attroff(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);

                // Statistiques
                attron(COLOR_PAIR(COLOR_PAIR_BLANC));
                mvprintw(LINES / 2 - 2, (COLS - 50) / 2, "========== STATISTIQUES ==========");
                mvprintw(LINES / 2 - 1, (COLS - 50) / 2, "Score final:        %ld points", plan->score);
                mvprintw(LINES / 2, (COLS - 50) / 2, "Meilleur score:     %ld points", plan->high_score);
                mvprintw(LINES / 2 + 1, (COLS - 50) / 2, "Argent gagne:       %ld.%02ld EUR",
                         plan->argent_total / 100, plan->argent_total % 100);
                mvprintw(LINES / 2 + 2, (COLS - 50) / 2, "Vehicules servis:   %d", plan->vehicules_servis);
                mvprintw(LINES / 2 + 3, (COLS - 50) / 2, "Vehicules perdus:   %d", plan->vehicules_perdus);

                int total = plan->vehicules_servis + plan->vehicules_perdus;
                int efficacite = (total > 0) ? (plan->vehicules_servis * 100 / total) : 0;
                mvprintw(LINES / 2 + 4, (COLS - 50) / 2, "Efficacite:         %d%%", efficacite);
                attroff(COLOR_PAIR(COLOR_PAIR_BLANC));

                attron(COLOR_PAIR(COLOR_PAIR_JAUNE));
                mvprintw(LINES / 2 + 6, (COLS - 40) / 2, "Appuyez sur une touche pour quitter...");
                attroff(COLOR_PAIR(COLOR_PAIR_JAUNE));

                refresh();
                nodelay(stdscr, FALSE);
                getch();
                running = 0;
            }
            frame_counter = 0;
        }

        // 2.5. Traiter les timeouts de la file d'attente
        traiter_timeout_attente(plan, file_attente, global_frame_counter, &notification_timeout);

        // 3. Affichage
        clear();
        afficher_titre_jeu();

        // Afficher le plan avec viewport (viewport fixe, pas besoin de recalculer)
        afficher_plan_avec_viewport(plan, &viewport);

        // Afficher tous les véhicules avec viewport
        VEHICULE *current = vehicules->premier;
        while (current != NULL)
        {
            afficher_vehicule_viewport(current, &viewport);
            current = current->NXT;
        }

        // Afficher le HUD (infos + légende + contrôles)
        afficher_hud_parking(plan, vehicules);

        // LOG FRAME 0 : positions et caractères sous chaque voiture
        if (!log_frame_0_done && frame_counter == 1)
        {
            FILE *logf = fopen("debug_frame0.log", "w");
            if (logf)
            {
                fprintf(logf, "=== LOG FRAME 0 ===\n");
                fprintf(logf, "Convention: x=colonne, y=ligne (en cellules)\n\n");

                VEHICULE *v = vehicules->premier;
                int v_num = 1;
                while (v != NULL)
                {
                    int x = v->posx;
                    int y = v->posy;
                    wchar_t c = L'?';

                    // Lire le caractère sous la voiture
                    if (x >= 0 && x < plan->largeur && y >= 0 && y < plan->hauteur)
                    {
                        c = plan->plan_statique[y][x];
                    }

                    fprintf(logf, "Voiture %d: x=%d y=%d etat='%c' dir='%c' char=U+%04X (%lc)\n",
                            v_num, x, y, v->etat, v->direction, (int)c, c);

                    v = v->NXT;
                    v_num++;
                }

                // PHASE D: Vérifier collision stricte au spawn
                fprintf(logf, "\n=== VERIFICATION COLLISION (STRICTE) ===\n");
                int collision_spawn = 0;
                VEHICULE *v1 = vehicules->premier;
                int n1 = 1;
                while (v1 != NULL && v1->etat == '1')
                {
                    VEHICULE *v2 = v1->NXT;
                    int n2 = n1 + 1;
                    while (v2 != NULL && v2->etat == '1')
                    {
                        // Collision stricte: même position exacte
                        if (v1->posx == v2->posx && v1->posy == v2->posy)
                        {
                            collision_spawn = 1;
                            fprintf(logf, "COLLISION STRICTE: v%d(%d,%d) <-> v%d(%d,%d)\n",
                                    n1, v1->posx, v1->posy, n2, v2->posx, v2->posy);
                        }
                        v2 = v2->NXT;
                        n2++;
                    }
                    v1 = v1->NXT;
                    n1++;
                }

                if (!collision_spawn)
                {
                    fprintf(logf, ">>> Spawn OK (pas de collision stricte) <<<\n");
                }

                fclose(logf);
            }
            log_frame_0_done = 1;
        }

        // 4. Rafraîchir et pause
        refresh();
        napms(57); // 57ms de pause (vitesse 3.5x - augmenté de 1,75x)
    }

    // Fermer le log spawn
    if (spawn_log)
    {
        fclose(spawn_log);
    }
}
