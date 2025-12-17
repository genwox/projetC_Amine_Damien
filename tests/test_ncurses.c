#include "affichage.h"
#include "plan.h"
#include <locale.h>
#include <unistd.h>

int main() {
    setlocale(LC_ALL, "");

    // Initialiser ncurses
    initialiser_affichage();

    // Charger le plan
    PlanParking* plan = charger_plan("plan.txt");
    if (!plan) {
        terminer_affichage();
        printf("Erreur chargement plan\n");
        return 1;
    }

    int running = 1;
    int vitesse = 5;
    int pause = 0;
    int temps = 0;

    // Message de bienvenue
    clear();
    afficher_titre_jeu();
    attron(COLOR_PAIR(COLOR_PAIR_JAUNE) | A_BOLD);
    mvprintw(10, 20, "Bienvenue dans le simulateur de parking !");
    mvprintw(12, 20, "Utilisez les fleches pour vous deplacer");
    mvprintw(13, 20, "Appuyez sur [ESPACE] pour continuer...");
    attroff(COLOR_PAIR(COLOR_PAIR_JAUNE) | A_BOLD);
    refresh();

    // Attendre une touche
    nodelay(stdscr, FALSE);  // Mode bloquant temporairement
    getch();
    nodelay(stdscr, TRUE);   // Retour en mode non-bloquant

    // Boucle principale
    while (running) {
        // 1. Lire input (NON-BLOQUANT)
        int ch = lire_touche_non_bloquant();

        if (ch != 0) {
            switch(ch) {
                case KEY_UP:
                    vitesse++;
                    if (vitesse > 10) vitesse = 10;
                    break;

                case KEY_DOWN:
                    vitesse--;
                    if (vitesse < 1) vitesse = 1;
                    break;

                case KEY_LEFT:
                    // Fermer barrière entrée
                    plan->barriere_entree_ouverte = 0;
                    break;

                case KEY_RIGHT:
                    // Ouvrir barrière entrée
                    plan->barriere_entree_ouverte = 1;
                    break;

                case KEY_QUIT:  // 'q'
                    running = 0;
                    break;

                case KEY_PAUSE:  // 'p'
                case ' ':
                    pause = !pause;
                    break;

                case KEY_RESET_GAME:  // 'r'
                    temps = 0;
                    vitesse = 5;
                    break;
            }
        }

        // 2. Mettre à jour simulation (si pas en pause)
        if (!pause) {
            temps++;
        }

        // 3. Afficher
        clear();

        afficher_titre_jeu();
        afficher_plan_complet(plan);
        afficher_hud_jeu(plan, temps / 20);  // Divisé par 20 pour avoir des secondes

        // Message vitesse
        attron(COLOR_PAIR(COLOR_PAIR_CYAN));
        mvprintw(LINES - 4, COLS - 25, "Vitesse: %d/10", vitesse);
        attroff(COLOR_PAIR(COLOR_PAIR_CYAN));

        if (pause) {
            attron(COLOR_PAIR(COLOR_PAIR_JAUNE) | A_BOLD);
            mvprintw(LINES / 2, (COLS - 10) / 2, "  [PAUSE]  ");
            attroff(COLOR_PAIR(COLOR_PAIR_JAUNE) | A_BOLD);
        }

        refresh();

        // 4. Délai (dépend de la vitesse)
        usleep(100000 / vitesse);  // Plus vitesse est haute, moins on attend
    }

    // Fermer ncurses
    terminer_affichage();

    // Message de fin
    printf("\nMerci d'avoir testé ncurses !\n");
    printf("Touches détectées: %d\n", temps);

    detruire_plan(&plan);
    return 0;
}
