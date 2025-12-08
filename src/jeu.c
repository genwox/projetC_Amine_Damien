#include "jeu.h"
#include "affichage.h"
#include "plan.h"
#include <ncurses.h>

void executer_boucle_jeu(PlanParking *plan, l_car *vehicules)
{
    if (!plan || !vehicules)
        return;

    // Affichage initial
    clear();
    afficher_titre_jeu();
    afficher_plan_complet(plan);

    int running = 1;
    nodelay(stdscr, TRUE);

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

        // 2. Affichage
        clear();
        afficher_titre_jeu();
        afficher_plan_complet(plan);

        // Afficher tous les véhicules
        VEHICULE *current = vehicules->premier;
        while (current != NULL)
        {
            afficher_vehicule(current);
            current = current->NXT;
        }

        // Afficher le HUD (infos + légende + contrôles)
        afficher_hud_parking(plan, vehicules);

        // 3. Rafraîchir et pause
        refresh();
        napms(200); // 200ms de pause
    }
}
