#ifndef AFFICHAGE_H
#define AFFICHAGE_H

#include <ncurses.h>
#include "plan.h"
#include "liste_car.h"

#define TAILLE_PLAN MAX_HAUTEUR
#define TERMINAL_MIN_LIGNES 32
#define TERMINAL_MIN_COLONNES 92

/* Les couleurs qu'on utilise avec ncurses */
#define COLOR_PAIR_DEFAULT 0
#define COLOR_PAIR_ROUGE 1
#define COLOR_PAIR_VERT 2
#define COLOR_PAIR_JAUNE 3
#define COLOR_PAIR_BLEU 4
#define COLOR_PAIR_CYAN 5
#define COLOR_PAIR_BLANC 6
#define COLOR_PAIR_GRIS 7
#define COLOR_PAIR_BG_VERT 8
#define COLOR_PAIR_BG_ROUGE 9

/* Touches du clavier */
#define KEY_QUIT 'q'
#define KEY_PAUSE 'p'
#define KEY_RESET_GAME 'r'
#define KEY_SPEED_UP '+'
#define KEY_SPEED_DOWN '-'

/* Pour optimiser l'affichage (éviter de tout redessiner) */
typedef struct {
    int derniere_mise_a_jour[TAILLE_PLAN][TAILLE_PLAN];
    int frame_courante;
    int affichage_optimise;
} GestionAffichage;

/* Pour afficher qu'une partie du plan */
typedef struct {
    int offset_x;
    int offset_y;
    int largeur;
    int hauteur;
} Viewport;

/* Stats pour le HUD */
typedef struct {
    int actifs;
    int gares;
    int sortis;
    int score;
    int places_libres;
    int places_totales;
} StatsJeu;

/* Démarre ncurses */
void initialiser_affichage();

/* Ferme ncurses proprement */
void terminer_affichage();

/* Vérifie si le terminal est assez grand */
int verifier_taille_terminal();

/* Affiche le plan */
int afficher_plan(PlanParking *plan, Viewport *viewport);

/* Raccourcis pratiques */
#define afficher_plan_complet(plan) afficher_plan(plan, NULL)
#define afficher_plan_avec_viewport(plan, vp) afficher_plan(plan, vp)

/* Affiche un caractère avec sa couleur */
void afficher_caractere_colore(char c, int x, int y);

/* Calcule le viewport pour centrer sur le bas du plan */
void calculer_viewport(PlanParking *plan, l_car *vehicules, Viewport *viewport);

void afficher_menu_principal();
void afficher_menu_modes();
int afficher_menu_difficulte();

/* Affiche les infos en haut de l'écran */
void afficher_hud_jeu(PlanParking *plan, int temps_ecoule);
void afficher_hud_parking(PlanParking *plan, l_car *vehicules);

void afficher_titre_jeu();
PlanParking *afficher_ecran_demarrage();

/* Affiche les voitures qui attendent à l'entrée */
void afficher_file_attente(FileAttenteEntree *file, int spawn_cd, PlanParking *plan);

/* Compte les voitures actives, garées, etc. */
void calculer_stats_jeu(l_car *liste, PlanParking *plan, StatsJeu *stats);

/* Dessine une voiture à l'écran */
void afficher_vehicule(VEHICULE *vehicule, int offset_x, int offset_y);

/* Lit une touche sans bloquer */
int lire_touche_non_bloquant();

GestionAffichage *creer_gestion_affichage();
void detruire_gestion_affichage(GestionAffichage **gestion);

/* Enlève les \n et \r à la fin */
void nettoyer_fin_ligne(char *ligne);

#endif
