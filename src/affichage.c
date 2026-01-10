#include "affichage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// ============================================================================
// FONCTIONS UTILITAIRES PRIVÉES
// ============================================================================

// Affiche un texte avec une couleur et des attributs
static void afficher_texte_colore(int y, int x, const char *texte, int color_pair, int attrs)
{
    attron(COLOR_PAIR(color_pair) | attrs);
    mvprintw(y, x, "%s", texte);
    attroff(COLOR_PAIR(color_pair) | attrs);
}

// Nettoie les fins de ligne (\n, \r)
static void nettoyer_fin_ligne(char *ligne)
{
    int len = strlen(ligne);

    while (len > 0 && (ligne[len - 1] == '\n' || ligne[len - 1] == '\r'))
    {
        ligne[len - 1] = '\0';
        len--;
    }
}

// Affiche un indicateur de place de parking (■ vert ou rouge)
static void afficher_indicateur_place(int y, int x, int occupee)
{
    int couleur = occupee ? COLOR_PAIR_ROUGE : COLOR_PAIR_VERT;
    afficher_texte_colore(y, x, "■", couleur, A_BOLD);
}

// Affiche l'état d'une barrière (ouverte/fermée)
static void afficher_etat_barriere(int y, int x, const char *nom, int ouverte)
{
    char message[50];
    snprintf(message, sizeof(message), "%s: %s", nom, ouverte ? "OUVERTE" : "FERMEE ");

    int couleur = ouverte ? COLOR_PAIR_VERT : COLOR_PAIR_ROUGE;
    afficher_texte_colore(y, x, message, couleur, 0);
}

// Affiche tous les indicateurs de places pour une ligne donnée
static void afficher_indicateurs_ligne(PlanParking *plan, int ligne_courante, int y_ecran)
{
    for (int i = 0; i < plan->places_totales; i++)
    {
        // Si on est sur la ligne juste au-dessus d'une place
        if (ligne_courante == plan->places[i].ligne - 1)
        {
            int x_indicateur = 2 + plan->places[i].colonne;
            afficher_indicateur_place(y_ecran, x_indicateur, plan->places[i].occupee);
        }
    }
}

// ============================================================================
// FONCTIONS D'INITIALISATION
// ============================================================================

void initialiser_affichage()
{
    // Initialiser ncurses
    initscr();             // Démarre ncurses
    cbreak();              // Désactive buffering ligne
    noecho();              // N'affiche pas les touches tapées
    nodelay(stdscr, TRUE); // getch() non-bloquant
    keypad(stdscr, TRUE);  // Active flèches (KEY_UP, KEY_DOWN, etc.)
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

// ============================================================================
// VÉRIFICATION ET GESTION DE BASE
// ============================================================================

int verifier_taille_terminal()
{
    if (LINES < TERMINAL_MIN_LIGNES || COLS < TERMINAL_MIN_COLONNES)
    {
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

void effacer_ecran()
{
    clear();
}

void rafraichir_ecran()
{
    refresh();
}

// ============================================================================
// GESTION DU VIEWPORT
// ============================================================================

// Centre le viewport sur une zone donnée
void centrer_viewport_sur_zone(int centre_x, int centre_y, int plan_largeur, int plan_hauteur, Viewport *viewport)
{
    if (!viewport)
        return;

    // Calculer les dimensions disponibles (en tenant compte des marges)
    viewport->largeur = COLS - 4;  // -4 pour les marges gauche/droite
    viewport->hauteur = LINES - 15; // -15 pour titre + HUD

    // Limiter la hauteur pour ne pas dépasser le plan
    if (viewport->hauteur > plan_hauteur)
        viewport->hauteur = plan_hauteur;

    // Centrer sur la position donnée
    viewport->offset_x = centre_x - (viewport->largeur / 2);
    viewport->offset_y = centre_y - (viewport->hauteur / 2);

    // S'assurer que le viewport ne sort pas du plan
    if (viewport->offset_x < 0)
        viewport->offset_x = 0;
    if (viewport->offset_y < 0)
        viewport->offset_y = 0;

    if (viewport->offset_x + viewport->largeur > plan_largeur)
        viewport->offset_x = plan_largeur - viewport->largeur;
    if (viewport->offset_y + viewport->hauteur > plan_hauteur)
        viewport->offset_y = plan_hauteur - viewport->hauteur;

    // Dernière vérification pour les plans très larges
    if (viewport->offset_x < 0)
        viewport->offset_x = 0;
    if (viewport->offset_y < 0)
        viewport->offset_y = 0;
}

// Calcule la position du viewport en fonction des véhicules actifs
void calculer_viewport(PlanParking *plan, l_car *vehicules, Viewport *viewport)
{
    if (!plan || !viewport)
        return;

    // Paramètre vehicules non utilisé dans cette version (viewport fixe)
    (void)vehicules;

    // Calculer les dimensions disponibles
    viewport->largeur = COLS - 4;  // -4 pour les marges
    viewport->hauteur = LINES - 7;  // -7 pour titre (1) + espace (1) + HUD (5)

    // Limiter la hauteur pour ne pas dépasser le plan
    if (viewport->hauteur > plan->hauteur)
        viewport->hauteur = plan->hauteur;

    viewport->offset_x = 0;

    // Si le plan est plus grand que le viewport, afficher depuis LE BAS
    // Cela garantit que les places de parking et l'entrée (en bas) sont visibles
    if (plan->hauteur > viewport->hauteur)
    {
        // Calculer l'offset pour montrer le bas du plan
        viewport->offset_y = plan->hauteur - viewport->hauteur;
    }
    else
    {
        // Le plan tient entièrement dans le viewport
        viewport->offset_y = 0;
    }

    // S'assurer que le viewport ne dépasse pas les limites du plan
    if (viewport->offset_x + viewport->largeur > plan->largeur)
        viewport->offset_x = plan->largeur - viewport->largeur;

    // Dernière vérification pour éviter des valeurs négatives
    if (viewport->offset_x < 0)
        viewport->offset_x = 0;
    if (viewport->offset_y < 0)
        viewport->offset_y = 0;
}

// ============================================================================
// AFFICHAGE DU PLAN
// ============================================================================

int afficher_plan_complet(PlanParking *plan)
{
    if (!plan)
        return 4;

    FILE *fichier = fopen("plan.txt", "r");
    if (!fichier)
    {
        afficher_texte_colore(5, 2, "Erreur: impossible d'ouvrir plan.txt", COLOR_PAIR_ROUGE, 0);
        return 6;
    }

    char ligne[MAX_LIGNE];
    int y = 4;
    int ligne_courante = 0;

    while (fgets(ligne, MAX_LIGNE, fichier) && y < LINES - 5)
    {
        nettoyer_fin_ligne(ligne);

        // Afficher la ligne du plan
        move(y, 2);
        addstr(ligne);

        // Ajouter les indicateurs colorés au-dessus des places
        afficher_indicateurs_ligne(plan, ligne_courante, y);

        y++;
        ligne_courante++;
    }

    fclose(fichier);
    return y;
}

// Affiche le plan avec viewport (fenêtre glissante)
int afficher_plan_avec_viewport(PlanParking *plan, Viewport *viewport)
{
    if (!plan || !viewport)
        return 4;

    FILE *fichier = fopen("plan.txt", "r");
    if (!fichier)
    {
        afficher_texte_colore(5, 2, "Erreur: impossible d'ouvrir plan.txt", COLOR_PAIR_ROUGE, 0);
        return 6;
    }

    char ligne[MAX_LIGNE];
    char ligne_visible[MAX_LIGNE];
    int y_ecran = 2; // Commence à ligne 2 (après le titre compact)
    int ligne_courante = 0;

    // Lire le fichier ligne par ligne
    while (fgets(ligne, MAX_LIGNE, fichier))
    {
        nettoyer_fin_ligne(ligne);

        // Vérifier si cette ligne est dans le viewport
        if (ligne_courante >= viewport->offset_y &&
            ligne_courante < viewport->offset_y + viewport->hauteur)
        {
            // Extraire la portion visible de la ligne
            int len = strlen(ligne);

            // Calculer le début et la fin en tenant compte du viewport
            // On doit parcourir caractère par caractère en UTF-8
            int colonne_actuelle = 0;
            int idx = 0;
            int idx_debut = 0;
            int idx_fin = len;

            // Trouver l'index de début
            while (idx < len && colonne_actuelle < viewport->offset_x)
            {
                // Compter les colonnes visuelles (pas les octets)
                unsigned char c = ligne[idx];
                if ((c & 0x80) == 0)
                {
                    // ASCII (1 octet)
                    colonne_actuelle++;
                    idx++;
                }
                else if ((c & 0xE0) == 0xC0)
                {
                    // 2 octets UTF-8
                    colonne_actuelle++;
                    idx += 2;
                }
                else if ((c & 0xF0) == 0xE0)
                {
                    // 3 octets UTF-8
                    colonne_actuelle++;
                    idx += 3;
                }
                else if ((c & 0xF8) == 0xF0)
                {
                    // 4 octets UTF-8
                    colonne_actuelle++;
                    idx += 4;
                }
                else
                {
                    // Octet de continuation, skip
                    idx++;
                }
            }
            idx_debut = idx;

            // Trouver l'index de fin
            int colonnes_a_afficher = viewport->largeur;
            int colonnes_affichees = 0;
            while (idx < len && colonnes_affichees < colonnes_a_afficher)
            {
                unsigned char c = ligne[idx];
                if ((c & 0x80) == 0)
                {
                    colonnes_affichees++;
                    idx++;
                }
                else if ((c & 0xE0) == 0xC0)
                {
                    colonnes_affichees++;
                    idx += 2;
                }
                else if ((c & 0xF0) == 0xE0)
                {
                    colonnes_affichees++;
                    idx += 3;
                }
                else if ((c & 0xF8) == 0xF0)
                {
                    colonnes_affichees++;
                    idx += 4;
                }
                else
                {
                    idx++;
                }
            }
            idx_fin = idx;

            // Copier la portion visible
            int taille_copie = idx_fin - idx_debut;
            if (taille_copie > 0 && taille_copie < MAX_LIGNE - 1)
            {
                strncpy(ligne_visible, ligne + idx_debut, taille_copie);
                ligne_visible[taille_copie] = '\0';
            }
            else
            {
                ligne_visible[0] = '\0';
            }

            // Afficher la ligne visible
            move(y_ecran, 2);
            addstr(ligne_visible);

            // Ajouter les indicateurs colorés (ajustés pour le viewport)
            for (int i = 0; i < plan->places_totales; i++)
            {
                if (ligne_courante == plan->places[i].ligne - 1)
                {
                    int colonne_place = plan->places[i].colonne;
                    // Vérifier si la place est dans le viewport
                    if (colonne_place >= viewport->offset_x &&
                        colonne_place < viewport->offset_x + viewport->largeur)
                    {
                        int x_indicateur = 2 + (colonne_place - viewport->offset_x);
                        afficher_indicateur_place(y_ecran, x_indicateur, plan->places[i].occupee);
                    }
                }
            }

            y_ecran++;
            if (y_ecran >= LINES - 5)
                break;
        }

        ligne_courante++;
        if (ligne_courante >= viewport->offset_y + viewport->hauteur)
            break;
    }

    fclose(fichier);
    return y_ecran;
}

void afficher_plan_optimise(PlanParking *plan, l_car *vehicules, GestionAffichage *gestion)
{
    for (int i = 0; i < plan->hauteur && i < TAILLE_PLAN; i++)
    {
        for (int j = 0; j < plan->largeur && j < TAILLE_PLAN; j++)
        {
            if (gestion->derniere_mise_a_jour[i][j] != gestion->frame_courante)
            {
                // Chercher s'il y a un véhicule à cette position
                char caractere_a_afficher = plan->plan_statique[i][j];

                if (vehicules && !est_vide_liste_car(vehicules))
                {
                    VEHICULE *v = vehicules->premier;
                    while (v)
                    {
                        if (v->etat == '1' && v->posy == i && v->posx == j)
                        {
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

void afficher_caractere_colore(char c, int x, int y)
{
    int color_pair = COLOR_PAIR_DEFAULT;

    if (c == '|' || c == '=' || c == '-' || c == '_')
    {
        color_pair = COLOR_PAIR_GRIS;
    }

    attron(COLOR_PAIR(color_pair));
    mvaddch(x + 4, y + 2, c);
    attroff(COLOR_PAIR(color_pair));
}

// ============================================================================
// MENUS
// ============================================================================

void afficher_menu_principal()
{
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

void afficher_menu_modes()
{
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

// ============================================================================
// HUD (HEADS-UP DISPLAY)
// ============================================================================

void afficher_hud_jeu(PlanParking *plan, int temps_ecoule)
{
    if (!plan)
        return;

    int hud_y = LINES - 4;
    int hud_x = 2;

    afficher_texte_colore(hud_y, hud_x, "=== PARKING SIMULATOR ===", COLOR_PAIR_CYAN, A_BOLD);

    attron(COLOR_PAIR(COLOR_PAIR_VERT));
    mvprintw(hud_y + 1, hud_x, "Places: %d/%d", plan->places_libres, plan->places_totales);
    attroff(COLOR_PAIR(COLOR_PAIR_VERT));

    mvprintw(hud_y + 2, hud_x, "Temps: %02d:%02d", temps_ecoule / 60, temps_ecoule % 60);

    // État des barrières
    afficher_etat_barriere(hud_y + 1, hud_x + 30, "Entree", plan->barriere_entree_ouverte);
    afficher_etat_barriere(hud_y + 2, hud_x + 30, "Sortie", plan->barriere_sortie_ouverte);

    // Commandes
    afficher_texte_colore(hud_y + 3, hud_x,
                         "Commandes: [Q]uitter [P]ause [R]eset  [Fleches]=Deplacement",
                         COLOR_PAIR_JAUNE, 0);
}

void afficher_hud_parking(PlanParking *plan, l_car *vehicules,
                          FileAttenteEntree *file_attente, int notification_timeout)
{
    if (!plan)
        return;

    // Compter les véhicules actifs et garés
    int nb_actifs = 0;
    int nb_gares = 0;
    if (vehicules)
    {
        VEHICULE *v = vehicules->premier;
        while (v != NULL)
        {
            if (v->etat == '1')
                nb_actifs++;
            else if (v->etat == '0')
                nb_gares++;
            v = v->NXT;
        }
    }

    // Ligne -5: Argent, Score et Mode
    int info_y = LINES - 5;
    attron(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
    mvprintw(info_y, 2, "Argent: %ld.%02ld EUR | Score: %ld | Meilleur: %ld | Mode: ",
             plan->argent_total / 100, plan->argent_total % 100,
             plan->score, plan->high_score);
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);

    // Afficher le mode avec la couleur appropriée
    if (plan->difficulte)
    {
        attron(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);
        mvprintw(info_y, 67, "HARD");
        attroff(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);
    }
    else
    {
        attron(COLOR_PAIR(COLOR_PAIR_VERT) | A_BOLD);
        mvprintw(info_y, 67, "NORMAL");
        attroff(COLOR_PAIR(COLOR_PAIR_VERT) | A_BOLD);
    }
    info_y++;

    // Ligne -4: Places et Statistiques
    attron(COLOR_PAIR(COLOR_PAIR_CYAN));
    mvprintw(info_y++, 2, "Places: %d/%d | Voitures: %d actifs, %d gares | Servis: %d | Perdus: %d",
             plan->places_libres, plan->places_totales, nb_actifs, nb_gares,
             plan->vehicules_servis, plan->vehicules_perdus);
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN));

    // Ligne -3: File d'attente et Barrières
    mvprintw(info_y, 2, "File d'attente: ");

    // Couleur de la file selon le remplissage
    int longueur = file_attente ? file_attente->longueur_attente : 0;
    int couleur_file = COLOR_PAIR_VERT;
    if (longueur >= 8)
        couleur_file = COLOR_PAIR_ROUGE;
    else if (longueur >= 5)
        couleur_file = COLOR_PAIR_JAUNE;

    attron(COLOR_PAIR(couleur_file) | A_BOLD);
    mvprintw(info_y, 19, "%d/10", longueur);
    attroff(COLOR_PAIR(couleur_file) | A_BOLD);

    // Indicateur de niveau de trafic (spawn adaptatif)
    const char* niveau_trafic;
    int couleur_trafic;
    if (longueur <= 3) {
        niveau_trafic = "FLUIDE";
        couleur_trafic = COLOR_PAIR_VERT;
    } else if (longueur <= 6) {
        niveau_trafic = "RALENTI";
        couleur_trafic = COLOR_PAIR_JAUNE;
    } else {
        niveau_trafic = "SATURE";
        couleur_trafic = COLOR_PAIR_ROUGE;
    }

    mvprintw(info_y, 25, " [");
    attron(COLOR_PAIR(couleur_trafic) | A_BOLD);
    mvprintw(info_y, 27, "%s", niveau_trafic);
    attroff(COLOR_PAIR(couleur_trafic) | A_BOLD);
    mvprintw(info_y, 27 + strlen(niveau_trafic), "]");

    // Barrières avec couleurs
    mvprintw(info_y, 36, " [Entree: ");
    if (plan->barriere_entree_ouverte)
    {
        attron(COLOR_PAIR(COLOR_PAIR_VERT) | A_BOLD);
        mvprintw(info_y, 47, "OUVERTE");
        attroff(COLOR_PAIR(COLOR_PAIR_VERT) | A_BOLD);
    }
    else
    {
        attron(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);
        mvprintw(info_y, 47, "FERMEE ");
        attroff(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);
    }

    mvprintw(info_y, 54, "] [Sortie: ");
    if (plan->barriere_sortie_ouverte)
    {
        attron(COLOR_PAIR(COLOR_PAIR_VERT) | A_BOLD);
        mvprintw(info_y, 66, "OUVERTE");
        attroff(COLOR_PAIR(COLOR_PAIR_VERT) | A_BOLD);
    }
    else
    {
        attron(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);
        mvprintw(info_y, 66, "FERMEE ");
        attroff(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);
    }
    mvprintw(info_y, 73, "]");

    // Warning timeout
    if (notification_timeout)
    {
        attron(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);
        mvprintw(info_y, 76, "[TIMEOUT!]");
        attroff(COLOR_PAIR(COLOR_PAIR_ROUGE) | A_BOLD);
    }
    info_y++;

    // Ligne -2: Légende
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

    // Ligne -1: Contrôles
    attron(COLOR_PAIR(COLOR_PAIR_JAUNE));
    mvprintw(LINES - 1, 2, "[Q]uitter [E]ntree [S]ortie");
    attroff(COLOR_PAIR(COLOR_PAIR_JAUNE));
}

// ============================================================================
// AFFICHAGE DE VÉHICULES ET TITRE
// ============================================================================

void afficher_vehicule(VEHICULE *vehicule)
{
    if (!vehicule)
        return;

    // Vérifier les limites pour éviter d'afficher hors écran
    int y_ecran = vehicule->posy + 2; // +2 pour le titre compact
    int x_ecran = vehicule->posx + 2;

    // Ne pas afficher si hors limites
    if (x_ecran < 0 || y_ecran < 2 || y_ecran >= LINES - 5)
        return;

    // Afficher la carrosserie du véhicule (même si garée, état='0')
    attron(COLOR_PAIR(vehicule->code_couleur));
    for (int i = 0; i < 4; i++)
    {
        int y_ligne = y_ecran + i;
        // Vérifier que chaque ligne est dans les limites
        if (y_ligne >= 2 && y_ligne < LINES - 5)
        {
            mvprintw(y_ligne, x_ecran, "%s", vehicule->Carrosserie[i]);
        }
    }
    attroff(COLOR_PAIR(vehicule->code_couleur));
}

// Affiche un véhicule avec viewport (coordonnées relatives)
void afficher_vehicule_viewport(VEHICULE *vehicule, Viewport *viewport)
{
    if (!vehicule || !viewport)
        return;

    // Vérifier si le véhicule est dans le viewport
    if (vehicule->posx < viewport->offset_x ||
        vehicule->posx >= viewport->offset_x + viewport->largeur ||
        vehicule->posy < viewport->offset_y ||
        vehicule->posy >= viewport->offset_y + viewport->hauteur)
    {
        // Véhicule hors viewport, ne pas afficher
        return;
    }

    // Calculer les coordonnées relatives au viewport
    int x_relatif = vehicule->posx - viewport->offset_x;
    int y_relatif = vehicule->posy - viewport->offset_y;

    // Afficher la carrosserie du véhicule (même si garée, état='0')
    attron(COLOR_PAIR(vehicule->code_couleur));
    for (int i = 0; i < 4; i++)
    {
        int y_ecran = y_relatif + i + 2; // +2 pour le titre compact
        int x_ecran = x_relatif + 2;     // +2 pour la marge

        // Vérifier que la ligne est visible à l'écran
        if (y_ecran >= 2 && y_ecran < LINES - 5)
        {
            mvprintw(y_ecran, x_ecran, "%s", vehicule->Carrosserie[i]);
        }
    }
    attroff(COLOR_PAIR(vehicule->code_couleur));
}

void afficher_titre_jeu()
{
    // Titre compact sur une seule ligne pour gagner de la place
    afficher_texte_colore(0, 2, "=== SIMULATEUR DE PARKING ===", COLOR_PAIR_CYAN, A_BOLD);
}

// ============================================================================
// SÉLECTION DE DIFFICULTÉ
// ============================================================================

int afficher_menu_difficulte()
{
    clear();
    afficher_titre_jeu();

    int y = 8;

    afficher_texte_colore(y++, 15, "=== CHOIX DE LA DIFFICULTE ===", COLOR_PAIR_CYAN, A_BOLD);
    y++;

    // Mode NORMAL
    afficher_texte_colore(y++, 18, "1. Mode NORMAL", COLOR_PAIR_VERT, A_BOLD);
    mvprintw(y++, 20, "   - Spawn: Standard (3-8 secondes)");
    mvprintw(y++, 20, "   - Timeout: 30 secondes");
    mvprintw(y++, 20, "   - Penalite: -200 points");
    y++;

    // Mode HARD
    afficher_texte_colore(y++, 18, "2. Mode HARD", COLOR_PAIR_ROUGE, A_BOLD);
    mvprintw(y++, 20, "   - Spawn: Rapide (2-5.5 secondes)");
    mvprintw(y++, 20, "   - Timeout: 20 secondes");
    mvprintw(y++, 20, "   - Penalite: -300 points");
    y++;

    afficher_texte_colore(y, 15, "Votre choix (1 ou 2): ", COLOR_PAIR_JAUNE, 0);

    refresh();

    // Attendre le choix de l'utilisateur
    nodelay(stdscr, FALSE);
    int ch;
    do
    {
        ch = getch();
    } while (ch != '1' && ch != '2');
    nodelay(stdscr, TRUE);

    // Retourner 0 pour NORMAL, 1 pour HARD
    return (ch == '2') ? 1 : 0;
}

// ============================================================================
// ÉCRAN DE DÉMARRAGE
// ============================================================================

PlanParking *afficher_ecran_demarrage()
{
    clear();
    afficher_titre_jeu();

    int y = 5;

    // Test liste chaînée
    afficher_texte_colore(y++, 2, "[OK] Test liste chainee... ", COLOR_PAIR_VERT, 0);

    l_car *l = nv_liste_car();
    if (l)
    {
        afficher_texte_colore(y - 1, 32, "OK", COLOR_PAIR_VERT, 0);
        detruire_liste_car(&l);
    }
    else
    {
        afficher_texte_colore(y - 1, 32, "ERREUR", COLOR_PAIR_ROUGE, 0);
        refresh();
        napms(2000);
        return NULL;
    }

    y++;

    // Chargement du plan
    afficher_texte_colore(y++, 2, "[OK] Chargement du plan... ", COLOR_PAIR_VERT, 0);

    PlanParking *plan = charger_plan("plan.txt");
    if (!plan)
    {
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
    afficher_texte_colore(y++, 2, "Appuyez sur [ESPACE] pour voir le parking...",
                         COLOR_PAIR_JAUNE, 0);

    refresh();

    // Attendre appui sur espace
    nodelay(stdscr, FALSE);
    int ch;
    do
    {
        ch = getch();
    } while (ch != ' ' && ch != '\n' && ch != KEY_ENTER);
    nodelay(stdscr, TRUE);

    return plan;
}

// ============================================================================
// UTILITAIRES CLAVIER
// ============================================================================

int lire_touche_non_bloquant()
{
    int ch = getch();

    if (ch == ERR)
    {
        return 0; // Aucune touche pressée
    }

    return ch;
}

// ============================================================================
// GESTION AFFICHAGE OPTIMISÉ
// ============================================================================

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
