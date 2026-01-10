#include "jeu.h"
#include "affichage.h"
#include "plan.h"
#include "mouvement.h"
#include "liste_car.h"
#include <ncurses.h>
#include <stdlib.h>

// Déclaration de est_cellule_roulable (définie dans mouvement.c)
extern int est_cellule_roulable_externe(PlanParking *plan, int x, int y);

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

void executer_boucle_jeu(PlanParking *plan, l_car *vehicules)
{
    if (!plan || !vehicules)
        return;

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

    // PHASE A: Variables pour spawn cadencé
    static int spawn_cd = 0;
    const int SPAWN_INTERVAL = 30; // ~6s avec napms(200) - intervalle augmenté

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

        // 1.5. PHASE A + E: Spawn cadencé de voitures avec logs détaillés
        if (spawn_cd > 0)
        {
            spawn_cd--;
        }
        else
        {
            // E) Log tentative de spawn
            if (spawn_log)
            {
                fprintf(spawn_log, "Frame %d: Tentative spawn\n", frame_counter);
                fprintf(spawn_log, "   entree = (%d,%d)\n", plan->entree_x, plan->entree_y);
                fprintf(spawn_log, "   places_libres = %d\n", plan->places_libres);
                fflush(spawn_log);
            }

            // Vérifier si entrée libre
            int libre = entree_libre(vehicules, plan->entree_x, plan->entree_y, spawn_log);

            if (spawn_log)
            {
                if (!libre)
                {
                    fprintf(spawn_log, "   -> REFUSE: entree occupee\n\n");
                    fflush(spawn_log);
                }
                else if (plan->places_libres <= 0)
                {
                    fprintf(spawn_log, "   -> REFUSE: plus de places libres\n\n");
                    fflush(spawn_log);
                }
            }

            // Spawner une nouvelle voiture si conditions OK
            if (libre && plan->places_libres > 0)
            {
                VEHICULE *nouvelle = creer_voiture_aleatoire(plan);
                if (nouvelle)
                {
                    // E) Log spawn réussi
                    if (spawn_log)
                    {
                        wchar_t c_spawn = L'?';
                        if (nouvelle->posx >= 0 && nouvelle->posx < plan->largeur &&
                            nouvelle->posy >= 0 && nouvelle->posy < plan->hauteur)
                        {
                            c_spawn = plan->plan_statique[nouvelle->posy][nouvelle->posx];
                        }

                        fprintf(spawn_log, "   -> SPAWN REUSSI\n");
                        fprintf(spawn_log, "      pos = (%d,%d)\n", nouvelle->posx, nouvelle->posy);
                        fprintf(spawn_log, "      char plan[%d][%d] = U+%04X (%lc)\n",
                                nouvelle->posy, nouvelle->posx, (int)c_spawn, c_spawn);
                        fprintf(spawn_log, "      places_libres apres = %d\n\n", plan->places_libres);
                        fflush(spawn_log);
                    }

                    ajouter_queue_liste_car(nouvelle, vehicules);
                    spawn_cd = SPAWN_INTERVAL;
                }
                else
                {
                    if (spawn_log)
                    {
                        fprintf(spawn_log, "   -> ECHEC: creer_voiture_aleatoire a echoue\n\n");
                    }
                }
            }
        }

        // 2. Déplacer les vehicules et détecter collisions
        frame_counter++;
        if (frame_counter >= 5)
        {
            int collision = deplacer_tous_vehicules(vehicules, plan);
            if (collision)
            {
                // GAME OVER - COLLISION !
                clear();
                attron(COLOR_PAIR(1)); // Rouge
                mvprintw(LINES / 2, (COLS - 30) / 2, "*** COLLISION DETECTEE ***");
                attroff(COLOR_PAIR(1));
                mvprintw(LINES / 2 + 2, (COLS - 20) / 2, "GAME OVER");
                mvprintw(LINES / 2 + 4, (COLS - 30) / 2, "Appuyez sur une touche...");
                refresh();
                nodelay(stdscr, FALSE);
                getch();
                running = 0;
            }
            frame_counter = 0;
        }

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
        napms(100); // 100ms de pause (vitesse 2x)
    }

    // Fermer le log spawn
    if (spawn_log)
    {
        fclose(spawn_log);
    }
}
