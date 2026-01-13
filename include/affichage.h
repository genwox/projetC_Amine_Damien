#ifndef AFFICHAGE_H
#define AFFICHAGE_H

#include <ncurses.h>
#include "plan.h"
#include "liste_car.h"

/* Constantes d'affichage */
#define TAILLE_PLAN MAX_HAUTEUR
#define TERMINAL_MIN_LIGNES 32     /* Hauteur minimale du terminal requise */
#define TERMINAL_MIN_COLONNES 92   /* Largeur minimale du terminal requise */

/* Paires de couleurs ncurses (utilisées avec COLOR_PAIR()) */
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

/* Touches de contrôle du jeu */
#define KEY_QUIT 'q'
#define KEY_PAUSE 'p'
#define KEY_RESET_GAME 'r'
#define KEY_SPEED_UP '+'
#define KEY_SPEED_DOWN '-'

/**
 * Gestion de l'affichage optimisé (tracking des mises à jour).
 * Permet d'éviter de redessiner les zones non modifiées.
 *
 * @field derniere_mise_a_jour Tableau 2D des dernières frames de mise à jour
 * @field frame_courante       Numéro de frame actuel
 * @field affichage_optimise   1 si optimisation active, 0 sinon
 */
typedef struct {
    int derniere_mise_a_jour[TAILLE_PLAN][TAILLE_PLAN];
    int frame_courante;
    int affichage_optimise;
} GestionAffichage;

/**
 * Viewport pour affichage partiel du plan (fenêtre glissante).
 *
 * @field offset_x Décalage X du viewport dans le plan
 * @field offset_y Décalage Y du viewport dans le plan
 * @field largeur  Largeur du viewport en colonnes
 * @field hauteur  Hauteur du viewport en lignes
 */
typedef struct {
    int offset_x;
    int offset_y;
    int largeur;
    int hauteur;
} Viewport;

/**
 * Statistiques du jeu pour l'affichage HUD.
 *
 * @field actifs        Nombre de véhicules actifs dans le parking
 * @field gares         Nombre de véhicules garés
 * @field sortis        Nombre de véhicules sortis avec succès
 * @field score         Score actuel
 * @field places_libres Nombre de places libres
 * @field places_totales Nombre total de places
 */
typedef struct {
    int actifs;
    int gares;
    int sortis;
    int score;
    int places_libres;
    int places_totales;
} StatsJeu;

/**
 * Initialise le système d'affichage ncurses.
 * Configure les couleurs, le mode non-bloquant, cache le curseur.
 */
void initialiser_affichage();

/**
 * Termine proprement le système d'affichage ncurses.
 * Restaure le terminal à son état normal.
 */
void terminer_affichage();

/**
 * Vérifie que le terminal a la taille minimale requise.
 *
 * @return 1 si la taille est suffisante, 0 sinon (affiche un message d'erreur)
 */
int verifier_taille_terminal();

/**
 * Affiche le plan du parking avec viewport optionnel.
 * Lit plan.txt et affiche avec gestion UTF-8.
 *
 * @param plan     Plan à afficher (doit être non NULL)
 * @param viewport Viewport pour affichage partiel (peut être NULL pour tout afficher)
 * @return         0 en cas de succès, code d'erreur sinon
 */
int afficher_plan(PlanParking *plan, Viewport *viewport);

/* Macros de commodité pour affichage plan */
#define afficher_plan_complet(plan) afficher_plan(plan, NULL)
#define afficher_plan_avec_viewport(plan, vp) afficher_plan(plan, vp)

/**
 * Affiche un caractère avec couleur selon son type (mur, route, etc.).
 *
 * @param c Caractère à afficher
 * @param x Position X (ligne)
 * @param y Position Y (colonne)
 */
void afficher_caractere_colore(char c, int x, int y);

/**
 * Calcule un viewport centré sur la zone inférieure du plan.
 *
 * @param plan      Plan de référence (doit être non NULL)
 * @param vehicules Liste des véhicules (peut être NULL, non utilisé actuellement)
 * @param viewport  Viewport à remplir (doit être non NULL)
 */
void calculer_viewport(PlanParking *plan, l_car *vehicules, Viewport *viewport);

/**
 * Affiche le menu principal du simulateur.
 */
void afficher_menu_principal();

/**
 * Affiche le menu de sélection des modes de jeu.
 */
void afficher_menu_modes();

/**
 * Affiche et gère le menu de sélection de difficulté.
 *
 * @return Niveau de difficulté choisi (1 = Normal, 2 = Hard)
 */
int afficher_menu_difficulte();

/**
 * Affiche le HUD de jeu (score, temps, etc.).
 *
 * @param plan         Plan du jeu (doit être non NULL)
 * @param temps_ecoule Temps écoulé en secondes
 */
void afficher_hud_jeu(PlanParking *plan, int temps_ecoule);

/**
 * Affiche le HUD du mode parking (véhicules actifs, places, etc.).
 *
 * @param plan      Plan du parking (doit être non NULL)
 * @param vehicules Liste des véhicules (doit être non NULL)
 */
void afficher_hud_parking(PlanParking *plan, l_car *vehicules);

/**
 * Affiche le titre ASCII art du jeu.
 */
void afficher_titre_jeu();

/**
 * Affiche l'écran de démarrage et charge le plan.
 *
 * @return Pointeur vers le plan chargé, NULL en cas d'erreur
 */
PlanParking *afficher_ecran_demarrage();

/**
 * Affiche la file d'attente des véhicules à l'entrée.
 *
 * @param file     File d'attente à afficher (doit être non NULL)
 * @param spawn_cd Temps avant prochain spawn (en frames)
 * @param plan     Plan du parking (doit être non NULL)
 */
void afficher_file_attente(FileAttenteEntree *file, int spawn_cd, PlanParking *plan);

/**
 * Calcule les statistiques du jeu depuis la liste des véhicules.
 * Compte les véhicules actifs, garés, sortis.
 *
 * @param liste Liste des véhicules (doit être non NULL)
 * @param plan  Plan du parking (doit être non NULL)
 * @param stats Structure où écrire les statistiques (doit être non NULL)
 */
void calculer_stats_jeu(l_car *liste, PlanParking *plan, StatsJeu *stats);

/**
 * Affiche un véhicule à sa position avec offset optionnel.
 *
 * @param vehicule Véhicule à afficher (doit être non NULL)
 * @param offset_x Décalage X pour viewport (0 si pas de viewport)
 * @param offset_y Décalage Y pour viewport (0 si pas de viewport)
 */
void afficher_vehicule(VEHICULE *vehicule, int offset_x, int offset_y);

/**
 * Lit une touche du clavier en mode non-bloquant.
 *
 * @return Code de la touche pressée, ERR si aucune touche
 */
int lire_touche_non_bloquant();

/**
 * Crée une structure de gestion d'affichage optimisé.
 *
 * @return Pointeur vers la structure créée, NULL en cas d'erreur
 */
GestionAffichage *creer_gestion_affichage();

/**
 * Détruit une structure de gestion d'affichage.
 *
 * @param gestion Pointeur vers le pointeur de structure (sera mis à NULL)
 */
void detruire_gestion_affichage(GestionAffichage **gestion);

/**
 * Nettoie les caractères de fin de ligne (\n, \r) d'une chaîne.
 *
 * @param ligne Chaîne à nettoyer (doit être non NULL)
 */
void nettoyer_fin_ligne(char *ligne);

#endif
