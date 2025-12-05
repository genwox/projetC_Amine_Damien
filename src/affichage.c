#include "affichage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void initialiser_affichage()
{
    // Initialiser ncurses
    initscr();             // Démarre ncurses
    cbreak();              // Désactive buffering ligne
    noecho();              // N'affiche pas les touches tapées
    nodelay(stdscr, TRUE); //  getch() non-bloquant (= key_pressed)
    keypad(stdscr, TRUE);  //  Active flèches (KEY_UP, KEY_DOWN, etc.)
    curs_set(0);           // Cache le curseur

    // Initialiser les couleurs
    if (has_colors())
    {
        start_color();
        init_pair(COLOR_PAIR_ROUGE, COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_PAIR_VERT, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_PAIR_JAUNE, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_PAIR_BLEU, COLOR_BLUE, COLOR_BLACK);
        init_pair(COLOR_PAIR_CYAN, COLOR_CYAN, COLOR_BLACK);
        init_pair(COLOR_PAIR_BLANC, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_PAIR_GRIS, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_PAIR_BG_VERT, COLOR_BLACK, COLOR_GREEN);
        init_pair(COLOR_PAIR_BG_ROUGE, COLOR_BLACK, COLOR_RED);
    }

    clear();
    refresh();
}

void terminer_affichage()
{
    curs_set(1); // Réaffiche le curseur
    endwin();    // Ferme ncurses
}

void effacer_ecran()
{
    clear();
}

void rafraichir_ecran()
{
    refresh();
}

int afficher_plan_complet(PlanParking *plan)
{
    if (!plan)
        return 4;

    // Lire et afficher plan.txt
    FILE *fichier = fopen("plan.txt", "r");
    if (!fichier)
    {
        attron(COLOR_PAIR(COLOR_PAIR_ROUGE));
        mvprintw(5, 2, "Erreur: impossible d'ouvrir plan.txt");
        attroff(COLOR_PAIR(COLOR_PAIR_ROUGE));
        return 6;
    }

    char ligne[MAX_LIGNE];
    int y = 4; // Commence en ligne 4

    while (fgets(ligne, MAX_LIGNE, fichier) && y < LINES - 12) // Laisser de la place pour les infos
    {
        // Retirer le \n
        int len = strlen(ligne);
        if (len > 0 && ligne[len - 1] == '\n')
        {
            ligne[len - 1] = '\0';
        }

        // Utiliser move() + addstr() au lieu de mvprintw() pour UTF-8
        move(y, 2);
        addstr(ligne);
        y++;
    }

    fclose(fichier);
    return y; // Retourne la dernière ligne utilisée
}

void afficher_plan_optimise(PlanParking *plan, l_car *vehicules, GestionAffichage *gestion)
{
    // Mettre à jour le plan avec les véhicules
    placer_vehicules_sur_plan(plan, vehicules);

    // Afficher seulement les cases modifiées
    for (int i = 0; i < plan->hauteur && i < TAILLE_PLAN; i++)
    {
        for (int j = 0; j < plan->largeur && j < TAILLE_PLAN; j++)
        {
            if (gestion->derniere_mise_a_jour[i][j] != gestion->frame_courante)
            {
                afficher_caractere_colore(plan->plan_dynamique[i][j], i, j);
                gestion->derniere_mise_a_jour[i][j] = gestion->frame_courante;
            }
        }
    }

    gestion->frame_courante++;
}

void afficher_caractere_colore(char c, int x, int y)
{
    // Choisir la couleur selon le caractère
    int color_pair = COLOR_PAIR_DEFAULT;

    if (c == '|' || c == '=' || c == '-' || c == '_')
    {
        color_pair = COLOR_PAIR_GRIS;
    }

    attron(COLOR_PAIR(color_pair));
    mvaddch(x + 4, y + 2, c); // +4 et +2 pour le décalage
    attroff(COLOR_PAIR(color_pair));
}

void afficher_menu_principal()
{
    clear();
    afficher_titre_jeu();

    attron(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
    mvprintw(15, 15, "=== SIMULATEUR DE PARKING ===");
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);

    mvprintw(17, 20, "1. Nouvelle partie");
    mvprintw(18, 20, "2. Charger partie");
    mvprintw(19, 20, "3. Options");
    mvprintw(20, 20, "4. Quitter");

    attron(COLOR_PAIR(COLOR_PAIR_JAUNE));
    mvprintw(22, 15, "Votre choix: ");
    attroff(COLOR_PAIR(COLOR_PAIR_JAUNE));

    refresh();
}

void afficher_menu_modes()
{
    clear();

    attron(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
    mvprintw(10, 15, "=== SELECTION DU MODE ===");
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);

    attron(COLOR_PAIR(COLOR_PAIR_VERT));
    mvprintw(12, 18, "1. Mode FLUIDE");
    attroff(COLOR_PAIR(COLOR_PAIR_VERT));
    mvprintw(13, 20, "   - Moins de vehicules");
    mvprintw(14, 20, "   - Circulation normale");

    attron(COLOR_PAIR(COLOR_PAIR_JAUNE));
    mvprintw(16, 18, "2. Mode CHARGE");
    attroff(COLOR_PAIR(COLOR_PAIR_JAUNE));
    mvprintw(17, 20, "   - Plus de vehicules");
    mvprintw(18, 20, "   - Risque de bouchons");

    attron(COLOR_PAIR(COLOR_PAIR_CYAN));
    mvprintw(20, 18, "3. Mode CONDUITE");
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN));
    mvprintw(21, 20, "   - Vous conduisez!");

    attron(COLOR_PAIR(COLOR_PAIR_JAUNE));
    mvprintw(23, 15, "Votre choix: ");
    attroff(COLOR_PAIR(COLOR_PAIR_JAUNE));

    refresh();
}

void afficher_hud_jeu(PlanParking *plan, int temps_ecoule)
{
    if (!plan)
        return;

    int hud_y = LINES - 4;
    int hud_x = 2;

    attron(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
    mvprintw(hud_y, hud_x, "=== PARKING SIMULATOR ===");
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);

    attron(COLOR_PAIR(COLOR_PAIR_VERT));
    mvprintw(hud_y + 1, hud_x, "Places: %d/%d", plan->places_libres, plan->places_totales);
    attroff(COLOR_PAIR(COLOR_PAIR_VERT));

    mvprintw(hud_y + 2, hud_x, "Temps: %02d:%02d", temps_ecoule / 60, temps_ecoule % 60);

    // État des barrières
    if (plan->barriere_entree_ouverte)
    {
        attron(COLOR_PAIR(COLOR_PAIR_VERT));
        mvprintw(hud_y + 1, hud_x + 30, "Entree: OUVERTE");
        attroff(COLOR_PAIR(COLOR_PAIR_VERT));
    }
    else
    {
        attron(COLOR_PAIR(COLOR_PAIR_ROUGE));
        mvprintw(hud_y + 1, hud_x + 30, "Entree: FERMEE ");
        attroff(COLOR_PAIR(COLOR_PAIR_ROUGE));
    }

    if (plan->barriere_sortie_ouverte)
    {
        attron(COLOR_PAIR(COLOR_PAIR_VERT));
        mvprintw(hud_y + 2, hud_x + 30, "Sortie: OUVERTE");
        attroff(COLOR_PAIR(COLOR_PAIR_VERT));
    }
    else
    {
        attron(COLOR_PAIR(COLOR_PAIR_ROUGE));
        mvprintw(hud_y + 2, hud_x + 30, "Sortie: FERMEE ");
        attroff(COLOR_PAIR(COLOR_PAIR_ROUGE));
    }

    // Commandes
    attron(COLOR_PAIR(COLOR_PAIR_JAUNE));
    mvprintw(hud_y + 3, hud_x, "Commandes: [Q]uitter [P]ause [R]eset  [Fleches]=Deplacement");
    attroff(COLOR_PAIR(COLOR_PAIR_JAUNE));
}

void afficher_vehicule(VEHICULE *vehicule)
{
    if (!vehicule || vehicule->etat != '1')
        return;

    // Afficher la carrosserie du véhicule
    attron(COLOR_PAIR(COLOR_PAIR_CYAN));
    for (int i = 0; i < 4; i++)
    {
        mvprintw(vehicule->posx + i + 4, vehicule->posy + 2, "%s", vehicule->Carrosserie[i]);
    }
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN));
}

void afficher_titre_jeu()
{
    attron(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
    mvprintw(0, (COLS - 50) / 2, "╔════════════════════════════════════════════════╗");
    mvprintw(1, (COLS - 50) / 2, "║     SIMULATEUR DE PARKING - VUE AERIENNE      ║");
    mvprintw(2, (COLS - 50) / 2, "╚════════════════════════════════════════════════╝");
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
}

int lire_touche_non_bloquant()
{
    //  MODE NON-BLOQUANT grâce à nodelay(stdscr, TRUE)
    int ch = getch();

    if (ch == ERR)
    {
        return 0; // Aucune touche pressée
    }

    return ch; // Retourne le code (KEY_UP, KEY_DOWN, 'q', etc.)
}

GestionAffichage *creer_gestion_affichage()
{
    GestionAffichage *gestion = malloc(sizeof(GestionAffichage));
    if (!gestion)
        return NULL;

    gestion->frame_courante = 1;
    gestion->affichage_optimise = 1;

    // Initialiser toutes les cases à 0
    for (int i = 0; i < TAILLE_PLAN; i++)
    {
        for (int j = 0; j < TAILLE_PLAN; j++)
        {
            gestion->derniere_mise_a_jour[i][j] = 0;
        }
    }

    return gestion;
}

void detruire_gestion_affichage(GestionAffichage **gestion)
{
    if (*gestion)
    {
        free(*gestion);
        *gestion = NULL;
    }
}

// obtenir_couleur_caractere est définie dans plan.c
