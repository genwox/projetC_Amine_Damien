#ifndef PLAN_H
#define PLAN_H

#include "matrice.h"
#include <wchar.h>
#include <locale.h>

typedef struct liste_car l_car;

/* Dimensions maximales du plan */
#define MAX_HAUTEUR 100    /* Hauteur maximale en lignes */
#define MAX_LARGEUR 150    /* Largeur maximale en colonnes */
#define MAX_LIGNE 600      /* Taille buffer lecture (UTF-8: ~3 bytes/char) */

/* Caractères ASCII représentant les éléments du plan */
#define MUR '#'
#define ROUTE ' '
#define PLACE_LIBRE 'P'
#define PLACE_OCCUPEE 'X'
#define ENTREE 'E'
#define SORTIE 'S'
#define BORNE_ENTREE 'T'
#define BORNE_SORTIE 'B'
#define BARRIERE_FERMEE '-'
#define BARRIERE_OUVERTE '|'
#define ALLEE 'A'
#define FLECHE_HAUT '^'
#define FLECHE_BAS 'v'
#define FLECHE_GAUCHE '<'
#define FLECHE_DROITE '>'

/**
 * Représente une place de parking dans le plan.
 *
 * @field ligne   Ligne de la place
 * @field colonne Colonne de la place
 * @field occupee 1 si occupée, 0 si libre
 */
typedef struct {
    int ligne;
    int colonne;
    int occupee;
} PlaceParking;

/**
 * Représente une flèche directionnelle dans le plan.
 *
 * @field ligne             Ligne de la flèche
 * @field colonne           Colonne de la flèche
 * @field direction_entree  Direction d'entrée dans la flèche
 * @field direction_sortie  Direction de sortie de la flèche
 */
typedef struct {
    int ligne;
    int colonne;
    char direction_entree;
    char direction_sortie;
} FlecheDirection;

/**
 * Structure centrale contenant toute l'état du parking.
 *
 * @field plan_statique           Grille 2D avec caractères UTF-8 (murs, routes, flèches...)
 * @field hauteur                 Hauteur réelle du plan chargé
 * @field largeur                 Largeur réelle du plan chargé
 * @field matrice_occupation      Matrice d'occupation pour détection collision
 * @field entree_x                Position X de l'entrée
 * @field entree_y                Position Y de l'entrée
 * @field sortie_x                Position X de la sortie
 * @field sortie_y                Position Y de la sortie
 * @field borne_entree_x          Position X de la barrière d'entrée
 * @field borne_entree_y          Position Y de la barrière d'entrée
 * @field borne_sortie_x          Position X de la barrière de sortie
 * @field borne_sortie_y          Position Y de la barrière de sortie
 * @field barriere_entree_ouverte 1 si barrière entrée ouverte, 0 sinon
 * @field barriere_sortie_ouverte 1 si barrière sortie ouverte, 0 sinon
 * @field places_libres           Nombre de places actuellement libres
 * @field places_totales          Nombre total de places de parking
 * @field argent_total            Argent gagné cumulé
 * @field score                   Score actuel du joueur
 * @field high_score              Meilleur score enregistré
 * @field vehicules_servis        Nombre de véhicules qui ont payé et sont sortis
 * @field vehicules_perdus        Nombre de véhicules en timeout (pénalité)
 * @field difficulte              Niveau de difficulté (1=Normal, 2=Hard)
 * @field places                  Tableau des places de parking (max 50)
 * @field fleches                 Tableau des flèches directionnelles (max 100)
 * @field nb_fleches              Nombre de flèches détectées dans le plan
 */
typedef struct plan_parking {
    wchar_t plan_statique[MAX_HAUTEUR][MAX_LARGEUR];
    int hauteur;
    int largeur;
    mat *matrice_occupation;
    int entree_x;
    int entree_y;
    int sortie_x;
    int sortie_y;
    int borne_entree_x;
    int borne_entree_y;
    int borne_sortie_x;
    int borne_sortie_y;
    int barriere_entree_ouverte;
    int barriere_sortie_ouverte;
    int places_libres;
    int places_totales;
    unsigned long int argent_total;
    unsigned long int score;
    unsigned long int high_score;
    int vehicules_servis;
    int vehicules_perdus;
    int difficulte;
    PlaceParking places[50];
    FlecheDirection fleches[100];
    int nb_fleches;
} PlanParking;

/**
 * Charge un plan de parking depuis un fichier texte UTF-8.
 * Détecte automatiquement les entrées, sorties, places et flèches.
 *
 * @param fichier_plan Chemin vers le fichier plan (ex: "data/plan.txt")
 * @return             Pointeur vers le plan chargé, NULL en cas d'erreur
 */
PlanParking* charger_plan(const char* fichier_plan);

/**
 * Détruit un plan et libère toute sa mémoire.
 *
 * @param plan Pointeur vers le pointeur du plan (sera mis à NULL)
 */
void detruire_plan(PlanParking** plan);

/**
 * Initialise la matrice d'occupation à partir du plan statique.
 * Marque les murs et obstacles comme occupés.
 *
 * @param plan Plan à initialiser (doit être non NULL avec matrice allouée)
 */
void initialiser_matrice_depuis_plan(PlanParking* plan);

/**
 * Place les véhicules sur le plan (fonction legacy, actuellement vide).
 *
 * @param plan            Plan de destination (doit être non NULL)
 * @param liste_vehicules Liste des véhicules à placer
 */
void placer_vehicules_sur_plan(PlanParking* plan, l_car* liste_vehicules);

/**
 * Bascule l'état de la barrière d'entrée (ouverte <-> fermée).
 *
 * @param plan Plan à modifier (doit être non NULL)
 */
void basculer_barriere_entree(PlanParking* plan);

/**
 * Bascule l'état de la barrière de sortie (ouverte <-> fermée).
 *
 * @param plan Plan à modifier (doit être non NULL)
 */
void basculer_barriere_sortie(PlanParking* plan);

/**
 * Recherche une place de parking à une position donnée.
 *
 * @param plan    Plan où chercher (doit être non NULL)
 * @param ligne   Ligne à tester
 * @param colonne Colonne à tester
 * @return        Index de la place trouvée, -1 si aucune place à cette position
 */
int trouver_place_a_position(PlanParking* plan, int ligne, int colonne);

/**
 * Marque une place comme occupée et décrémente le compteur de places libres.
 *
 * @param plan        Plan à modifier (doit être non NULL)
 * @param index_place Index de la place (doit être dans [0..places_totales-1])
 */
void marquer_place_occupee(PlanParking* plan, int index_place);

/**
 * Marque une place comme libre et incrémente le compteur de places libres.
 *
 * @param plan        Plan à modifier (doit être non NULL)
 * @param index_place Index de la place (doit être dans [0..places_totales-1])
 */
void marquer_place_libre(PlanParking* plan, int index_place);

#endif
