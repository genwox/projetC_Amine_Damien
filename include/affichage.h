#ifndef AFFICHAGE_H
#define AFFICHAGE_H

#include <ncurses.h>
#include "plan.h"
#include "liste_car.h"

// Taille du plan pour l'affichage
#define TAILLE_PLAN MAX_HAUTEUR

// Taille minimale requise pour le terminal
#define TERMINAL_MIN_LIGNES 32   // Titre (3) + Plan (38) + Infos/Légende (10) + Marge (1)
#define TERMINAL_MIN_COLONNES 92 // Largeur du plan encadré

// Paires de couleurs ncurses (au lieu des codes ANSI)
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

// Touches de contrôle
#define KEY_QUIT 'q'
#define KEY_PAUSE 'p'
#define KEY_RESET_GAME 'r' // Renommé pour éviter conflit avec ncurses
#define KEY_SPEED_UP '+'
#define KEY_SPEED_DOWN '-'

// Structure pour gérer l'affichage optimisé
typedef struct
{
    int derniere_mise_a_jour[TAILLE_PLAN][TAILLE_PLAN];
    int frame_courante;
    int affichage_optimise;
} GestionAffichage;

// Fonctions d'initialisation/fermeture
void initialiser_affichage();
void terminer_affichage();
int verifier_taille_terminal(); // Retourne 1 si OK, 0 si trop petit

// Fonctions d'affichage de base
void effacer_ecran();
void rafraichir_ecran();

// Affichage du parking
int afficher_plan_complet(PlanParking *plan); // Retourne la dernière ligne utilisée
void afficher_plan_optimise(PlanParking *plan, l_car *vehicules, GestionAffichage *gestion);
void afficher_caractere_colore(char c, int x, int y);

// Menus et HUD
void afficher_menu_principal();
void afficher_menu_modes();
void afficher_hud_jeu(PlanParking *plan, int temps_ecoule);
void afficher_hud_parking(PlanParking *plan, l_car *vehicules);
void afficher_titre_jeu();
PlanParking *afficher_ecran_demarrage();

// Véhicules
void afficher_vehicule(VEHICULE *vehicule);

// Gestion input clavier (MODE NON-BLOQUANT)
int lire_touche_non_bloquant();

// Utilitaires gestion affichage
GestionAffichage *creer_gestion_affichage();
void detruire_gestion_affichage(GestionAffichage **gestion);

#endif
