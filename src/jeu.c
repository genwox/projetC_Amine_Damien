#include "jeu.h"
#include "affichage.h"
#include "plan.h"
#include "mouvement.h"
#include "liste_car.h"
#include "utils.h"
#include <ncurses.h>
#include <stdlib.h>
extern int est_cellule_roulable_externe(PlanParking *plan, int x, int y);
unsigned long int global_frame_counter = 0;
#define TIMEOUT_ATTENTE 300
#define PENALITE_TIMEOUT 200
#define SPAWN_RAPIDE_NORMAL 30
#define SPAWN_MOYEN_NORMAL 50
#define SPAWN_LENT_NORMAL 80
#define SPAWN_RAPIDE_HARD 20
#define SPAWN_MOYEN_HARD 35
#define SPAWN_LENT_HARD 55
#define TIMEOUT_NORMAL 300
#define PENALITE_NORMAL 200
#define TIMEOUT_HARD 200
#define PENALITE_HARD 300
static int calculer_spawn_interval(FileAttenteEntree *file, int difficulte) {
    if (!file) return difficulte ? SPAWN_RAPIDE_HARD : SPAWN_RAPIDE_NORMAL;
    int l = file->longueur_attente;
    if (difficulte) return l <= 3 ? SPAWN_RAPIDE_HARD : l <= 6 ? SPAWN_MOYEN_HARD : SPAWN_LENT_HARD;
    return l <= 3 ? SPAWN_RAPIDE_NORMAL : l <= 6 ? SPAWN_MOYEN_NORMAL : SPAWN_LENT_NORMAL;
}
static int entree_libre(l_car *vehicules, int ex, int ey) {
    if (!vehicules || est_vide_liste_car(vehicules)) return 1;
    for (VEHICULE *v = vehicules->premier; v; v = v->NXT) {
        if (v->etat == '1') {
            int cx, cy; calculer_centre_vehicule(v, &cx, &cy);
            if (abs(cx - ex) < 10 && abs(cy - ey) < 6) return 0;
        }
    }
    return 1;
}
static void traiter_entree_vehicules(PlanParking *plan, FileAttenteEntree *file, l_car *vehicules, unsigned long frame) {
    if (!plan->barriere_entree_ouverte || file_attente_est_vide(file) || plan->places_libres <= 0) return;
    if (!entree_libre(vehicules, plan->entree_x, plan->entree_y)) return;
    VEHICULE *v = retirer_de_file_attente(file);
    if (!v) return;
    v->posx = plan->entree_x;
    v->posy = plan->entree_y;
    v->direction = 'O';
    v->etat = '1';
    v->tps = frame;
    orienter_carrosserie(v);
    ajouter_queue_liste_car(v, vehicules);
    corriger_alignement_vehicule(v, plan);
    const unsigned long PRIX_ENTREE = 500;
    plan->argent_total += PRIX_ENTREE;
    plan->score += 100;
    plan->vehicules_servis++;
}
static void traiter_timeout_attente(PlanParking *plan, FileAttenteEntree *file_attente,
                                     unsigned long frame, int *notification_timeout)
{
    if (file_attente_est_vide(file_attente))
        return;
    *notification_timeout = 0;
    VEHICULE *v = file_attente->premier_attente;
    VEHICULE *prev = NULL;
    unsigned long timeout = plan->difficulte ? TIMEOUT_HARD : TIMEOUT_NORMAL;
    unsigned long penalite = plan->difficulte ? PENALITE_HARD : PENALITE_NORMAL;
    while (v != NULL)
    {
        unsigned long temps_ecoule = frame - v->temps_attente;
        if (temps_ecoule >= timeout)
        {
            VEHICULE *next = v->NXT;
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
static int position_valide_pour_vehicule(PlanParking *plan, int coin_x, int coin_y, int largeur, int hauteur)
{
    const int MARGE_SECURITE = 3;
    if (coin_x < MARGE_SECURITE || coin_y < MARGE_SECURITE)
        return 0;
    if (coin_x + largeur >= plan->largeur - MARGE_SECURITE ||
        coin_y + hauteur >= plan->hauteur - MARGE_SECURITE)
        return 0;
    for (int dy = 0; dy < hauteur; dy++)
    {
        for (int dx = 0; dx < largeur; dx++)
        {
            int check_x = coin_x + dx;
            int check_y = coin_y + dy;
            if (!est_cellule_roulable_externe(plan, check_x, check_y))
                return 0;
        }
    }
    return 1;
}
static int trouver_position_route_proche(PlanParking *plan, int x_depart, int y_depart,
                                          int largeur, int hauteur,
                                          int *coin_x_sortie, int *coin_y_sortie, char *direction_sortie)
{
    for (int rayon = 1; rayon <= 15; rayon++)
    {
        for (int dx = -rayon; dx <= rayon; dx++)
        {
            for (int dy = -rayon; dy <= rayon; dy++)
            {
                if (abs(dx) + abs(dy) > rayon)
                    continue;
                int centre_x = x_depart + dx;
                int centre_y = y_depart + dy;
                if (centre_x < 0 || centre_y < 0 || centre_x >= plan->largeur || centre_y >= plan->hauteur)
                    continue;
                wchar_t c = plan->plan_statique[centre_y][centre_x];
                if (c == L'←' || c == L'→' || c == L'↑' || c == L'↓')
                {
                    int coin_x = centre_x - largeur / 2;
                    int coin_y = centre_y - hauteur / 2;
                    if (position_valide_pour_vehicule(plan, coin_x, coin_y, largeur, hauteur))
                    {
                        *coin_x_sortie = coin_x;
                        *coin_y_sortie = coin_y;
                        if (c == L'←')
                            *direction_sortie = 'O';
                        else if (c == L'→')
                            *direction_sortie = 'E';
                        else if (c == L'↑')
                            *direction_sortie = 'N';
                        else if (c == L'↓')
                            *direction_sortie = 'S';
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}
static void verifier_et_reactiver_vehicules_gares(l_car *vehicules, PlanParking *plan, unsigned long int current_frame)
{
    if (!vehicules || !plan || est_vide_liste_car(vehicules))
        return;
    const unsigned long int DUREE_MIN = 300;
    const unsigned long int DUREE_MAX = 600;
    VEHICULE *v = vehicules->premier;
    while (v != NULL)
    {
        if (v->etat == '0')
        {
            unsigned long int duree_parking = current_frame - v->tps;
            unsigned long int duree_cible = DUREE_MIN +
                                            ((unsigned long int)v % (DUREE_MAX - DUREE_MIN + 1));
            if (duree_parking >= duree_cible)
            {
                int centre_x, centre_y;
                calculer_centre_vehicule(v, &centre_x, &centre_y);
                int largeur, hauteur;
                obtenir_dimensions_vehicule(v, &largeur, &hauteur);
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
                int coin_x, coin_y;
                char nouvelle_direction;
                if (trouver_position_route_proche(plan, centre_x, centre_y, largeur, hauteur,
                                                   &coin_x, &coin_y, &nouvelle_direction))
                {
                    v->posx = coin_x;
                    v->posy = coin_y;
                    v->direction = nouvelle_direction;
                    orienter_carrosserie(v);
                }
                v->etat = '1';
                marquer_vehicule_en_sortie(v);
            }
        }
        v = v->NXT;
    }
}
static void gerer_spawn_vehicules(JeuState *etat, PlanParking *plan, l_car *tous_vehicules,
                                   VEHICULE **entree_queue)
{
    (void)entree_queue;
    traiter_entree_vehicules(plan, etat->file_attente, tous_vehicules, global_frame_counter);
    if (etat->spawn_cd > 0)
    {
        etat->spawn_cd--;
    }
    else
    {
        if (!file_attente_est_pleine(etat->file_attente))
        {
            VEHICULE *nouvelle = creer_voiture_aleatoire(plan);
            if (nouvelle)
            {
                if (ajouter_a_file_attente(etat->file_attente, nouvelle, global_frame_counter))
                {
                    etat->spawn_cd = calculer_spawn_interval(etat->file_attente, plan->difficulte);
                }
                else
                {
                    detruire_vehicule(&nouvelle);
                }
            }
        }
        else
        {
            etat->spawn_cd = calculer_spawn_interval(etat->file_attente, plan->difficulte);
        }
    }
}
static int gerer_collisions_et_game_over(JeuState *etat, l_car *tous_vehicules, PlanParking *plan)
{
    global_frame_counter++;
    etat->frame_counter++;
    if (etat->frame_counter >= 5)
    {
        verifier_et_reactiver_vehicules_gares(tous_vehicules, plan, global_frame_counter);
        int collision = deplacer_tous_vehicules(tous_vehicules, plan);
        if (collision)
        {
            clear();
            if (plan->score > plan->high_score)
            {
                plan->high_score = plan->score;
            }
            attron(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);
            mvprintw(LINES / 2 - 6, (COLS - 30) / 2, "*** COLLISION DETECTEE ***");
            attroff(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);
            attron(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
            mvprintw(LINES / 2 - 4, (COLS - 20) / 2, "GAME OVER");
            attroff(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
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
            return 1;
        }
        etat->frame_counter = 0;
    }
    return 0;
}
void executer_boucle_jeu(PlanParking *plan, l_car *vehicules, FileAttenteEntree *file_attente)
{
    if (!plan || !vehicules || !file_attente)
        return;
    JeuState etat = {
        .file_attente = file_attente,
        .spawn_cd = 0,
        .frame_counter = 0,
        .notification_timeout = 0
    };
    Viewport viewport;
    calculer_viewport(plan, vehicules, &viewport);
    int running = 1;
    nodelay(stdscr, TRUE);
    while (running)
    {
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
        gerer_spawn_vehicules(&etat, plan, vehicules, NULL);
        int should_end = gerer_collisions_et_game_over(&etat, vehicules, plan);
        if (should_end)
        {
            running = 0;
            break;
        }
        traiter_timeout_attente(plan, file_attente, global_frame_counter, &etat.notification_timeout);
        clear();
        afficher_titre_jeu();
        afficher_plan_avec_viewport(plan, &viewport);
        VEHICULE *current = vehicules->premier;
        while (current != NULL)
        {
            afficher_vehicule(current, viewport.offset_x, viewport.offset_y);
            current = current->NXT;
        }
        afficher_hud_parking(plan, vehicules);
        afficher_file_attente(file_attente, etat.spawn_cd, plan);
        refresh();
        napms(57);
    }
}
