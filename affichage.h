#ifndef AFFICHAGE_H
#define AFFICHAGE_H

#include "plan.h"
#include "liste_car.h"

// Codes couleurs ANSI
#define RESET_COLOR     "\x1b[0m"
#define ROUGE           "\x1b[31m"
#define VERT            "\x1b[32m"
#define JAUNE           "\x1b[33m"
#define BLEU            "\x1b[34m"
#define MAGENTA         "\x1b[35m"
#define CYAN            "\x1b[36m"
#define BLANC           "\x1b[37m"
#define GRIS            "\x1b[90m"

// Couleurs de fond
#define BG_ROUGE        "\x1b[41m"
#define BG_VERT         "\x1b[42m"
#define BG_JAUNE        "\x1b[43m"
#define BG_BLEU         "\x1b[44m"

// Contrôle du curseur
#define CLEAR_SCREEN    "\x1b[2J"
#define CURSOR_HOME     "\x1b[H"
#define HIDE_CURSOR     "\x1b[?25l"
#define SHOW_CURSOR     "\x1b[?25h"

// Macros pour positionner le curseur
#define GOTO_XY(x,y)    printf("\x1b[%d;%dH", (x)+1, (y)+1)

// Structure pour gérer l'affichage
typedef struct {
    int derniere_mise_a_jour[TAILLE_PLAN][TAILLE_PLAN];
    int frame_courante;
    int affichage_optimise;
} GestionAffichage;

// Fonctions d'affichage
void initialiser_affichage();
void terminer_affichage();
void effacer_ecran();
void afficher_plan_complet(PlanParking* plan);
void afficher_plan_optimise(PlanParking* plan, l_car* vehicules, GestionAffichage* gestion);
void afficher_caractere_colore(char c, int x, int y);
void afficher_menu_principal();
void afficher_menu_modes();
void afficher_hud_jeu(PlanParking* plan, int temps_ecoule);
void afficher_vehicule(VEHICULE* vehicule);
void afficher_titre_jeu();
char* obtenir_couleur_caractere(char c);
GestionAffichage* creer_gestion_affichage();
void detruire_gestion_affichage(GestionAffichage** gestion);

#endif