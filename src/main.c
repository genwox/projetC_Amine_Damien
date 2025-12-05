#include "liste_car.h"
#include "matrice.h"
#include "plan.h"
#include "affichage.h"
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <unistd.h>

int main()
{
    setlocale(LC_ALL, "");

    // Initialiser ncurses
    initialiser_affichage();

    clear();

    // Titre
    afficher_titre_jeu();

    int y = 5;  // Position verticale pour les messages

    // Test liste chaînée
    attron(COLOR_PAIR(COLOR_PAIR_VERT));
    mvprintw(y++, 2, "[OK] Test liste chainee... ");
    attroff(COLOR_PAIR(COLOR_PAIR_VERT));

    l_car *l = nv_liste_car();
    if (l)
    {
        attron(COLOR_PAIR(COLOR_PAIR_VERT));
        printw("OK");
        attroff(COLOR_PAIR(COLOR_PAIR_VERT));
        detruire_liste_car(&l);
    }
    else
    {
        attron(COLOR_PAIR(COLOR_PAIR_ROUGE));
        printw("ERREUR");
        attroff(COLOR_PAIR(COLOR_PAIR_ROUGE));
        refresh();
        napms(2000);
        terminer_affichage();
        return 1;
    }

    y++;

    // Chargement du plan
    attron(COLOR_PAIR(COLOR_PAIR_VERT));
    mvprintw(y++, 2, "[OK] Chargement du plan... ");
    attroff(COLOR_PAIR(COLOR_PAIR_VERT));

    PlanParking *plan = charger_plan("plan.txt");
    if (plan)
    {
        attron(COLOR_PAIR(COLOR_PAIR_VERT));
        printw("OK");
        attroff(COLOR_PAIR(COLOR_PAIR_VERT));

        attron(COLOR_PAIR(COLOR_PAIR_CYAN));
        mvprintw(y++, 2, "[i] Dimensions: %dx%d", plan->hauteur, plan->largeur);
        mvprintw(y++, 2, "[i] Places detectees: ");
        attroff(COLOR_PAIR(COLOR_PAIR_CYAN));
        attron(COLOR_PAIR(COLOR_PAIR_VERT));
        printw("%d", plan->places_totales);
        attroff(COLOR_PAIR(COLOR_PAIR_VERT));

        y += 2;
        attron(COLOR_PAIR(COLOR_PAIR_JAUNE));
        mvprintw(y++, 2, "Appuyez sur [ESPACE] pour voir le parking...");
        attroff(COLOR_PAIR(COLOR_PAIR_JAUNE));

        refresh();

        // Attendre appui sur espace
        nodelay(stdscr, FALSE);  // Mode bloquant
        int ch;
        do {
            ch = getch();
        } while (ch != ' ' && ch != '\n' && ch != KEY_ENTER);
        nodelay(stdscr, TRUE);   // Retour en mode non-bloquant

        // Afficher le plan complet
        clear();
        afficher_titre_jeu();
        int info_y = afficher_plan_complet(plan);  // Récupère la dernière ligne du plan

        // Afficher les infos APRÈS le plan
        info_y += 1;  // Sauter une ligne
        attron(COLOR_PAIR(COLOR_PAIR_CYAN));
        mvprintw(info_y++, 2, "Places libres: %d/%d", plan->places_libres, plan->places_totales);
        mvprintw(info_y++, 2, "Barriere entree: %s", plan->barriere_entree_ouverte ? "OUVERTE" : "FERMEE");
        mvprintw(info_y++, 2, "Barriere sortie: %s", plan->barriere_sortie_ouverte ? "OUVERTE" : "FERMEE");
        attroff(COLOR_PAIR(COLOR_PAIR_CYAN));

        info_y++;

        // Légende
        attron(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
        mvprintw(info_y++, 2, "LEGENDE:");
        attroff(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);

        attron(COLOR_PAIR(COLOR_PAIR_VERT));
        mvprintw(info_y++, 4, "|_|  Place libre");
        attroff(COLOR_PAIR(COLOR_PAIR_VERT));

        attron(COLOR_PAIR(COLOR_PAIR_ROUGE));
        mvprintw(info_y++, 4, "|X|  Place occupee");
        attroff(COLOR_PAIR(COLOR_PAIR_ROUGE));

        attron(COLOR_PAIR(COLOR_PAIR_CYAN));
        mvprintw(info_y++, 4, "> < ^ v  Sens de circulation");
        attroff(COLOR_PAIR(COLOR_PAIR_CYAN));

        info_y++;
        attron(COLOR_PAIR(COLOR_PAIR_VERT) | A_BOLD);
        mvprintw(info_y++, 2, "[OK] Tests reussis !");
        attroff(COLOR_PAIR(COLOR_PAIR_VERT) | A_BOLD);

        attron(COLOR_PAIR(COLOR_PAIR_JAUNE));
        mvprintw(LINES - 1, 2, "Appuyez sur [Q] pour quitter...");
        attroff(COLOR_PAIR(COLOR_PAIR_JAUNE));

        refresh();

        // Attendre 'q' pour quitter
        nodelay(stdscr, FALSE);
        int quit_ch;
        do {
            quit_ch = getch();
        } while (quit_ch != 'q' && quit_ch != 'Q');

        detruire_plan(&plan);
    }
    else
    {
        attron(COLOR_PAIR(COLOR_PAIR_ROUGE));
        printw("ERREUR");
        attroff(COLOR_PAIR(COLOR_PAIR_ROUGE));
        refresh();
        napms(2000);
        terminer_affichage();
        return 1;
    }

    // Fermer ncurses
    terminer_affichage();

    printf("\nMerci d'avoir utilise le simulateur de parking !\n");

    return 0;
}