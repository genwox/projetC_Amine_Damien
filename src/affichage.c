/*
 * Affichage ncurses avec viewport et HUD
 */
#include "affichage.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static void afficher_texte_colore(int y, int x, const char *texte, int color_pair, int attrs) {
    attron(COLOR_PAIR(color_pair) | attrs);
    mvprintw(y, x, "%s", texte);
    attroff(COLOR_PAIR(color_pair) | attrs);
}
void nettoyer_fin_ligne(char *ligne) {
    int len = strlen(ligne);
    while (len > 0 && (ligne[len - 1] == '\n' || ligne[len - 1] == '\r')) ligne[--len] = '\0';
}
static void afficher_indicateur_place(int y, int x, int occupee) {
    int couleur = occupee ? COLOR_PAIR_ROUGE : COLOR_PAIR_VERT;
    afficher_texte_colore(y, x, "■", couleur, A_BOLD);
}
static void afficher_etat_barriere(int y, int x, const char *nom, int ouverte) {
    char message[50];
    snprintf(message, sizeof(message), "%s: %s", nom, ouverte ? "OUVERTE" : "FERMEE ");
    int couleur = ouverte ? COLOR_PAIR_VERT : COLOR_PAIR_ROUGE;
    afficher_texte_colore(y, x, message, couleur, 0);
}
static void afficher_indicateurs_ligne(PlanParking *plan, int ligne_courante, int y_ecran) {
    for (int i = 0; i < plan->places_totales; i++) {
        if (ligne_courante == plan->places[i].ligne - 1) {
            int x_indicateur = 2 + plan->places[i].colonne;
            afficher_indicateur_place(y_ecran, x_indicateur, plan->places[i].occupee);
        }
    }
}
static void afficher_barriere_avec_statut(int y, int x, const char *nom, int ouverte) {
    mvprintw(y, x, "[%s: ", nom);
    if (ouverte) {
        attron(COLOR_PAIR(COLOR_PAIR_VERT) | A_BOLD);
        mvprintw(y, x + strlen(nom) + 3, "OUVERTE");
        attroff(COLOR_PAIR(COLOR_PAIR_VERT) | A_BOLD);
    } else {
        attron(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);
        mvprintw(y, x + strlen(nom) + 3, "FERMEE ");
        attroff(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);
    }
    mvprintw(y, x + strlen(nom) + 10, "]");
}
void initialiser_affichage() {
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
    if (has_colors()) {
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
void terminer_affichage() {
    curs_set(1);
    endwin();
}
int verifier_taille_terminal() {
    if (LINES < TERMINAL_MIN_LIGNES || COLS < TERMINAL_MIN_COLONNES) {
        clear();
        afficher_texte_colore(2, 2, "ERREUR: Terminal trop petit!", COLOR_PAIR_ROUGE, A_BOLD);
        mvprintw(4, 2, "Taille actuelle:  %d lignes x %d colonnes", LINES, COLS);
        mvprintw(5, 2, "Taille requise:   %d lignes x %d colonnes", TERMINAL_MIN_LIGNES, TERMINAL_MIN_COLONNES);
        afficher_texte_colore(7, 2, "Veuillez agrandir votre fenetre de terminal et relancer le programme.",
                             COLOR_PAIR_CYAN, 0);
        afficher_texte_colore(9, 2, "Appuyez sur une touche pour quitter...", COLOR_PAIR_BLANC, 0);
        refresh();
        nodelay(stdscr, FALSE);
        getch();
        return 0;
    }
    return 1;
}
void effacer_ecran() {
    clear();
}
void rafraichir_ecran() {
    refresh();
}
void centrer_viewport_sur_zone(int centre_x, int centre_y, int plan_largeur, int plan_hauteur, Viewport *viewport) {
    if (!viewport) return;
    viewport->largeur = COLS - 4;
    viewport->hauteur = LINES - 15;
    if (viewport->hauteur > plan_hauteur) viewport->hauteur = plan_hauteur;
    viewport->offset_x = centre_x - (viewport->largeur / 2);
    viewport->offset_y = centre_y - (viewport->hauteur / 2);
    if (viewport->offset_x < 0) viewport->offset_x = 0;
    if (viewport->offset_y < 0) viewport->offset_y = 0;
    if (viewport->offset_x + viewport->largeur > plan_largeur) viewport->offset_x = plan_largeur - viewport->largeur;
    if (viewport->offset_y + viewport->hauteur > plan_hauteur) viewport->offset_y = plan_hauteur - viewport->hauteur;
    if (viewport->offset_x < 0) viewport->offset_x = 0;
    if (viewport->offset_y < 0) viewport->offset_y = 0;
}
void calculer_viewport(PlanParking *plan, l_car *vehicules, Viewport *viewport) {
    if (!plan || !viewport) return;
    (void)vehicules;
    viewport->largeur = COLS - 4;
    viewport->hauteur = LINES - 7;
    if (viewport->hauteur > plan->hauteur) viewport->hauteur = plan->hauteur;
    viewport->offset_x = 0;
    if (plan->hauteur > viewport->hauteur) {
        viewport->offset_y = plan->hauteur - viewport->hauteur;
    } else {
        viewport->offset_y = 0;
    }
    if (viewport->offset_x + viewport->largeur > plan->largeur) viewport->offset_x = plan->largeur - viewport->largeur;
    if (viewport->offset_x < 0) viewport->offset_x = 0;
    if (viewport->offset_y < 0) viewport->offset_y = 0;
}
int afficher_plan(PlanParking *plan, Viewport *viewport) {
    if (!plan) return 4;
    FILE *fichier = fopen("plan.txt", "r");
    if (!fichier) {
        afficher_texte_colore(5, 2, "Erreur: impossible d'ouvrir plan.txt", COLOR_PAIR_ROUGE, 0);
        return 6;
    }
    char ligne[MAX_LIGNE], ligne_visible[MAX_LIGNE];
    int y_ecran = viewport ? 2 : 4;
    int ligne_courante = 0;
    int limite_y = LINES - 5;
    while (fgets(ligne, MAX_LIGNE, fichier)) {
        nettoyer_fin_ligne(ligne);
        // Si viewport NULL, afficher tout
        if (!viewport) {
            if (y_ecran >= limite_y) break;
            move(y_ecran, 2);
            addstr(ligne);
            afficher_indicateurs_ligne(plan, ligne_courante, y_ecran);
            y_ecran++;
            ligne_courante++;
            continue;
        }
        // Avec viewport: filtrer et découper
        if (ligne_courante >= viewport->offset_y && ligne_courante < viewport->offset_y + viewport->hauteur) {
            int len = strlen(ligne);
            int colonne_actuelle = 0, idx = 0, idx_debut = 0, idx_fin = len;
            // Trouver idx_debut
            while (idx < len && colonne_actuelle < viewport->offset_x) {
                unsigned char c = ligne[idx];
                int bytes = largeur_affichage_utf8(c);
                if (bytes > 1 || (c & 0x80) == 0) colonne_actuelle++;
                idx += (bytes > 0) ? bytes : 1;
            }
            idx_debut = idx;
            // Trouver idx_fin
            int colonnes_a_afficher = viewport->largeur, colonnes_affichees = 0;
            while (idx < len && colonnes_affichees < colonnes_a_afficher) {
                unsigned char c = ligne[idx];
                int bytes = largeur_affichage_utf8(c);
                if (bytes > 1 || (c & 0x80) == 0) colonnes_affichees++;
                idx += (bytes > 0) ? bytes : 1;
            }
            idx_fin = idx;
            // Copier substring
            int taille_copie = idx_fin - idx_debut;
            if (taille_copie > 0 && taille_copie < MAX_LIGNE - 1) {
                strncpy(ligne_visible, ligne + idx_debut, taille_copie);
                ligne_visible[taille_copie] = '\0';
            } else {
                ligne_visible[0] = '\0';
            }
            move(y_ecran, 2);
            addstr(ligne_visible);
            // Indicateurs avec viewport
            for (int i = 0; i < plan->places_totales; i++) {
                if (ligne_courante == plan->places[i].ligne - 1) {
                    int colonne_place = plan->places[i].colonne;
                    if (colonne_place >= viewport->offset_x && colonne_place < viewport->offset_x + viewport->largeur) {
                        int x_indicateur = 2 + (colonne_place - viewport->offset_x);
                        afficher_indicateur_place(y_ecran, x_indicateur, plan->places[i].occupee);
                    }
                }
            }
            y_ecran++;
            if (y_ecran >= limite_y) break;
        }
        ligne_courante++;
        if (viewport && ligne_courante >= viewport->offset_y + viewport->hauteur) break;
    }
    fclose(fichier);
    return y_ecran;
}
void afficher_plan_optimise(PlanParking *plan, l_car *vehicules, GestionAffichage *gestion) {
    for (int i = 0; i < plan->hauteur && i < TAILLE_PLAN; i++) {
        for (int j = 0; j < plan->largeur && j < TAILLE_PLAN; j++) {
            if (gestion->derniere_mise_a_jour[i][j] != gestion->frame_courante) {
                char caractere_a_afficher = plan->plan_statique[i][j];
                if (vehicules && !est_vide_liste_car(vehicules)) {
                    VEHICULE *v = vehicules->premier;
                    while (v) {
                        if (v->etat == '1' && v->posy == i && v->posx == j) {
                            caractere_a_afficher = v->type;
                            break;
                        }
                        v = v->NXT;
                    }
                }
                afficher_caractere_colore(caractere_a_afficher, i, j);
                gestion->derniere_mise_a_jour[i][j] = gestion->frame_courante;
            }
        }
    }
    gestion->frame_courante++;
}
void afficher_caractere_colore(char c, int x, int y) {
    int color_pair = COLOR_PAIR_DEFAULT;
    if (c == '|' || c == '=' || c == '-' || c == '_') color_pair = COLOR_PAIR_GRIS;
    attron(COLOR_PAIR(color_pair));
    mvaddch(x + 4, y + 2, c);
    attroff(COLOR_PAIR(color_pair));
}
void afficher_menu_principal() {
    clear();
    afficher_titre_jeu();
    afficher_texte_colore(15, 15, "=== SIMULATEUR DE PARKING ===", COLOR_PAIR_CYAN, A_BOLD);
    mvprintw(17, 20, "1. Nouvelle partie");
    mvprintw(18, 20, "2. Charger partie");
    mvprintw(19, 20, "3. Options");
    mvprintw(20, 20, "4. Quitter");
    afficher_texte_colore(22, 15, "Votre choix: ", COLOR_PAIR_JAUNE, 0);
    refresh();
}
void afficher_menu_modes() {
    clear();
    afficher_texte_colore(10, 15, "=== SELECTION DU MODE ===", COLOR_PAIR_CYAN, A_BOLD);
    afficher_texte_colore(12, 18, "1. Mode FLUIDE", COLOR_PAIR_VERT, 0);
    mvprintw(13, 20, "   - Moins de vehicules");
    mvprintw(14, 20, "   - Circulation normale");
    afficher_texte_colore(16, 18, "2. Mode CHARGE", COLOR_PAIR_JAUNE, 0);
    mvprintw(17, 20, "   - Plus de vehicules");
    mvprintw(18, 20, "   - Risque de bouchons");
    afficher_texte_colore(20, 18, "3. Mode CONDUITE", COLOR_PAIR_CYAN, 0);
    mvprintw(21, 20, "   - Vous conduisez!");
    afficher_texte_colore(23, 15, "Votre choix: ", COLOR_PAIR_JAUNE, 0);
    refresh();
}
void afficher_hud_jeu(PlanParking *plan, int temps_ecoule) {
    if (!plan) return;
    int hud_y = LINES - 4, hud_x = 2;
    afficher_texte_colore(hud_y, hud_x, "=== PARKING SIMULATOR ===", COLOR_PAIR_CYAN, A_BOLD);
    attron(COLOR_PAIR(COLOR_PAIR_VERT));
    mvprintw(hud_y + 1, hud_x, "Places: %d/%d", plan->places_libres, plan->places_totales);
    attroff(COLOR_PAIR(COLOR_PAIR_VERT));
    mvprintw(hud_y + 2, hud_x, "Temps: %02d:%02d", temps_ecoule / 60, temps_ecoule % 60);
    afficher_etat_barriere(hud_y + 1, hud_x + 30, "Entree", plan->barriere_entree_ouverte);
    afficher_etat_barriere(hud_y + 2, hud_x + 30, "Sortie", plan->barriere_sortie_ouverte);
    afficher_texte_colore(hud_y + 3, hud_x, "Commandes: [Q]uitter [P]ause [R]eset  [Fleches]=Deplacement", COLOR_PAIR_JAUNE, 0);
}
void calculer_stats_jeu(l_car *liste, PlanParking *plan, StatsJeu *stats) {
    if (!stats) return;
    stats->actifs = stats->gares = stats->sortis = stats->score = 0;
    stats->places_libres = plan ? plan->places_libres : 0;
    stats->places_totales = plan ? plan->places_totales : 0;
    if (!liste) return;
    VEHICULE *v = liste->premier;
    while (v != NULL) {
        if (v->etat == '1') stats->actifs++;
        else if (v->etat == '0') stats->gares++;
        v = v->NXT;
    }
}
void afficher_hud_parking(PlanParking *plan, l_car *vehicules) {
    if (!plan) return;
    StatsJeu stats;
    calculer_stats_jeu(vehicules, plan, &stats);
    int info_y = LINES - 4;
    attron(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
    mvprintw(info_y, 2, "Mode: ");
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
    if (plan->difficulte) {
        attron(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);
        mvprintw(info_y, 8, "HARD");
        attroff(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);
    } else {
        attron(COLOR_PAIR(COLOR_PAIR_VERT) | A_BOLD);
        mvprintw(info_y, 8, "NORMAL");
        attroff(COLOR_PAIR(COLOR_PAIR_VERT) | A_BOLD);
    }
    attron(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
    mvprintw(info_y, 16, " | Places: %d/%d | Voitures: %d actifs, %d gares",
             stats.places_libres, stats.places_totales, stats.actifs, stats.gares);
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
    info_y++;
    mvprintw(info_y, 2, "Barrieres - ");
    afficher_barriere_avec_statut(info_y, 14, "Entree", plan->barriere_entree_ouverte);
    mvprintw(info_y, 32, " ");
    afficher_barriere_avec_statut(info_y, 33, "Sortie", plan->barriere_sortie_ouverte);
    info_y++;
    mvprintw(info_y, 2, "LEGENDE: [");
    attron(COLOR_PAIR(COLOR_PAIR_VERT));
    mvprintw(info_y, 13, "V");
    attroff(COLOR_PAIR(COLOR_PAIR_VERT));
    mvprintw(info_y, 14, "] Libre  [");
    attron(COLOR_PAIR(COLOR_PAIR_ROUGE));
    mvprintw(info_y, 24, "X");
    attroff(COLOR_PAIR(COLOR_PAIR_ROUGE));
    mvprintw(info_y, 25, "] Occupee");
    info_y++;
    attron(COLOR_PAIR(COLOR_PAIR_JAUNE));
    mvprintw(LINES - 1, 2, "[Q]uitter [E]ntree [S]ortie");
    attroff(COLOR_PAIR(COLOR_PAIR_JAUNE));
}
void afficher_vehicule(VEHICULE *vehicule, int offset_x, int offset_y) {
    if (!vehicule) return;
    int x_base = vehicule->posx - offset_x;
    int y_base = vehicule->posy - offset_y;
    int y_ecran = y_base + 2, x_ecran = x_base + 2;
    if (x_ecran < 0 || y_ecran < 2 || y_ecran >= LINES - 5) return;
    attron(COLOR_PAIR(vehicule->code_couleur));
    for (int i = 0; i < 4; i++) {
        int y_ligne = y_ecran + i;
        if (y_ligne >= 2 && y_ligne < LINES - 5) {
            mvprintw(y_ligne, x_ecran, "%s", vehicule->Carrosserie[i]);
        }
    }
    attroff(COLOR_PAIR(vehicule->code_couleur));
}
void afficher_titre_jeu() {
    afficher_texte_colore(0, 2, "=== SIMULATEUR DE PARKING ===", COLOR_PAIR_CYAN, A_BOLD);
}
int afficher_menu_difficulte() {
    clear();
    afficher_titre_jeu();
    int y = 8;
    afficher_texte_colore(y++, 15, "=== CHOIX DE LA DIFFICULTE ===", COLOR_PAIR_CYAN, A_BOLD);
    y++;
    afficher_texte_colore(y++, 18, "1. Mode NORMAL", COLOR_PAIR_VERT, A_BOLD);
    mvprintw(y++, 20, "   - Spawn: Standard (3-8 secondes)");
    mvprintw(y++, 20, "   - Timeout: 30 secondes");
    mvprintw(y++, 20, "   - Penalite: -200 points");
    y++;
    afficher_texte_colore(y++, 18, "2. Mode HARD", COLOR_PAIR_ROUGE, A_BOLD);
    mvprintw(y++, 20, "   - Spawn: Rapide (2-5.5 secondes)");
    mvprintw(y++, 20, "   - Timeout: 20 secondes");
    mvprintw(y++, 20, "   - Penalite: -300 points");
    y++;
    afficher_texte_colore(y, 15, "Votre choix (1 ou 2): ", COLOR_PAIR_JAUNE, 0);
    refresh();
    nodelay(stdscr, FALSE);
    int ch;
    do {
        ch = getch();
    } while (ch != '1' && ch != '2');
    nodelay(stdscr, TRUE);
    return (ch == '2') ? 1 : 0;
}
PlanParking *afficher_ecran_demarrage() {
    clear();
    afficher_titre_jeu();
    int y = 5;
    afficher_texte_colore(y++, 2, "[OK] Test liste chainee... ", COLOR_PAIR_VERT, 0);
    l_car *l = nv_liste_car();
    if (l) {
        afficher_texte_colore(y - 1, 32, "OK", COLOR_PAIR_VERT, 0);
        detruire_liste_car(&l);
    } else {
        afficher_texte_colore(y - 1, 32, "ERREUR", COLOR_PAIR_ROUGE, 0);
        refresh();
        napms(2000);
        return NULL;
    }
    y++;
    afficher_texte_colore(y++, 2, "[OK] Chargement du plan... ", COLOR_PAIR_VERT, 0);
    PlanParking *plan = charger_plan("plan.txt");
    if (!plan) {
        afficher_texte_colore(y - 1, 32, "ERREUR", COLOR_PAIR_ROUGE, 0);
        refresh();
        napms(2000);
        return NULL;
    }
    afficher_texte_colore(y - 1, 32, "OK", COLOR_PAIR_VERT, 0);
    attron(COLOR_PAIR(COLOR_PAIR_CYAN));
    mvprintw(y++, 2, "[i] Dimensions: %dx%d", plan->hauteur, plan->largeur);
    mvprintw(y++, 2, "[i] Places detectees: ");
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN));
    attron(COLOR_PAIR(COLOR_PAIR_VERT));
    printw("%d", plan->places_totales);
    attroff(COLOR_PAIR(COLOR_PAIR_VERT));
    y += 2;
    afficher_texte_colore(y++, 2, "Appuyez sur [ESPACE] pour voir le parking...", COLOR_PAIR_JAUNE, 0);
    refresh();
    nodelay(stdscr, FALSE);
    int ch;
    do {
        ch = getch();
    } while (ch != ' ' && ch != '\n' && ch != KEY_ENTER);
    nodelay(stdscr, TRUE);
    return plan;
}
int lire_touche_non_bloquant() {
    int ch = getch();
    if (ch == ERR) return 0;
    return ch;
}
GestionAffichage *creer_gestion_affichage() {
    GestionAffichage *gestion = malloc(sizeof(GestionAffichage));
    if (!gestion) return NULL;
    gestion->frame_courante = 1;
    gestion->affichage_optimise = 1;
    for (int i = 0; i < TAILLE_PLAN; i++) {
        for (int j = 0; j < TAILLE_PLAN; j++) {
            gestion->derniere_mise_a_jour[i][j] = 0;
        }
    }
    return gestion;
}
void detruire_gestion_affichage(GestionAffichage **gestion) {
    if (*gestion) {
        free(*gestion);
        *gestion = NULL;
    }
}
void afficher_file_attente(FileAttenteEntree *file, int spawn_cd, PlanParking *plan) {
    if (!file || !plan) return;
    int info_y = LINES - 1;
    attron(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
    mvprintw(info_y, 2, "File: ");
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
    int couleur = COLOR_PAIR_VERT;
    if (file->longueur_attente >= 8) couleur = COLOR_PAIR_ROUGE;
    else if (file->longueur_attente >= 5) couleur = COLOR_PAIR_JAUNE;
    attron(COLOR_PAIR(couleur) | A_BOLD);
    mvprintw(info_y, 8, "%d/%d", file->longueur_attente, file->longueur_max);
    attroff(COLOR_PAIR(couleur) | A_BOLD);
    attron(COLOR_PAIR(COLOR_PAIR_CYAN));
    mvprintw(info_y, 15, "| Spawn: %ds | Score: %ld pts | Argent: %.2f EUR",
             spawn_cd / 10, plan->score, plan->argent_total / 100.0);
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN));
}
