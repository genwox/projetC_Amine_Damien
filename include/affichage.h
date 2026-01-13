#ifndef AFFICHAGE_H
#define AFFICHAGE_H
#include <ncurses.h>
#include "plan.h"
#include "liste_car.h"

#define TAILLE_PLAN MAX_HAUTEUR
#define TERMINAL_MIN_LIGNES 32
#define TERMINAL_MIN_COLONNES 92
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
#define KEY_QUIT 'q'
#define KEY_PAUSE 'p'
#define KEY_RESET_GAME 'r'
#define KEY_SPEED_UP '+'
#define KEY_SPEED_DOWN '-'

typedef struct {
    int derniere_mise_a_jour[TAILLE_PLAN][TAILLE_PLAN];
    int frame_courante, affichage_optimise;
} GestionAffichage;

typedef struct { int offset_x, offset_y, largeur, hauteur; } Viewport;
typedef struct { int actifs, gares, sortis, score, places_libres, places_totales; } StatsJeu;

void initialiser_affichage();
void terminer_affichage();
int verifier_taille_terminal();
void effacer_ecran();
void rafraichir_ecran();
int afficher_plan(PlanParking *plan, Viewport *viewport);
#define afficher_plan_complet(plan) afficher_plan(plan, NULL)
#define afficher_plan_avec_viewport(plan, vp) afficher_plan(plan, vp)
void afficher_plan_optimise(PlanParking *plan, l_car *vehicules, GestionAffichage *gestion);
void afficher_caractere_colore(char c, int x, int y);
void calculer_viewport(PlanParking *plan, l_car *vehicules, Viewport *viewport);
void centrer_viewport_sur_zone(int centre_x, int centre_y, int plan_largeur, int plan_hauteur, Viewport *viewport);
void afficher_menu_principal();
void afficher_menu_modes();
int afficher_menu_difficulte();
void afficher_hud_jeu(PlanParking *plan, int temps_ecoule);
void afficher_hud_parking(PlanParking *plan, l_car *vehicules);
void afficher_titre_jeu();
PlanParking *afficher_ecran_demarrage();
void afficher_file_attente(FileAttenteEntree *file, int spawn_cd, PlanParking *plan);
void calculer_stats_jeu(l_car *liste, PlanParking *plan, StatsJeu *stats);
void afficher_vehicule(VEHICULE *vehicule, int offset_x, int offset_y);
int lire_touche_non_bloquant();
GestionAffichage *creer_gestion_affichage();
void detruire_gestion_affichage(GestionAffichage **gestion);
void nettoyer_fin_ligne(char *ligne);
#endif
