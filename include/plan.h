#ifndef PLAN_H
#define PLAN_H

#include "matrice.h"
#include <wchar.h>
#include <locale.h>

// Forward declaration pour éviter la dépendance circulaire
typedef struct liste_car l_car;

#define MAX_HAUTEUR 100
#define MAX_LARGEUR 150
#define MAX_LIGNE 600  // Augmenté pour supporter les caractères UTF-8 (3 bytes par caractère)

// Caractères du plan
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

// Structure pour une place de parking
typedef struct {
    int ligne;      // Ligne du ╦ (haut de la place)
    int colonne;    // Colonne du ╦
    int occupee;    // 0 = libre, 1 = occupée
} PlaceParking;

// Structure pour une flèche directionnelle
typedef struct {
    int ligne;              // Ligne de la flèche
    int colonne;            // Colonne visuelle de la flèche
    char direction_entree;  // Direction d'entrée ('N', 'S', 'E', 'O', ou '\0' pour flèche simple)
    char direction_sortie;  // Direction de sortie ('N', 'S', 'E', 'O')
} FlecheDirection;

// Structure pour gérer le plan du parking
typedef struct plan_parking {
    wchar_t plan_statique[MAX_HAUTEUR][MAX_LARGEUR];  // wchar_t au lieu de char
    int hauteur;
    int largeur;
    mat *matrice_occupation;

    // Coordonnées importantes (Convention: _x = colonne, _y = ligne)
    int entree_x, entree_y;
    int sortie_x, sortie_y;
    int borne_entree_x, borne_entree_y;
    int borne_sortie_x, borne_sortie_y;

    // État des barrières
    int barriere_entree_ouverte;
    int barriere_sortie_ouverte;

    // Compteurs
    int places_libres;
    int places_totales;

    // État du jeu
    unsigned long int argent_total;      // Argent en centimes (500 = 5.00€)
    unsigned long int score;              // Score actuel
    int vehicules_servis;                 // Voitures entrées avec succès
    int vehicules_perdus;                 // Voitures parties (timeout)
    unsigned long int high_score;         // Meilleur score de la session
    int difficulte;                       // 0 = Normal, 1 = Hard

    // Positions des places de parking (pour affichage couleur)
    PlaceParking places[50];  // Max 50 places

    // Positions des flèches directionnelles (pour le déplacement)
    FlecheDirection fleches[100];  // Max 100 flèches
    int nb_fleches;                // Nombre de flèches détectées
} PlanParking;

// Chargement et destruction
PlanParking* charger_plan(const char* fichier_plan);  // Charge plan.txt, détecte places/flèches
void detruire_plan(PlanParking** plan);               // Libère mémoire du plan

// Initialisation
void initialiser_matrice_depuis_plan(PlanParking* plan);  // Crée matrice d'occupation
void placer_vehicules_sur_plan(PlanParking* plan, l_car* liste_vehicules);  // Place véhicules initiaux

// Gestion occupation
int est_position_libre(PlanParking* plan, int x, int y);     // Retourne 1 si libre
void occuper_place_parking(PlanParking* plan, int x, int y); // Marque cellule occupée
void liberer_place_parking(PlanParking* plan, int x, int y); // Marque cellule libre

// Barrières entrée/sortie
void basculer_barriere_entree(PlanParking* plan);   // Ouvre/ferme barrière entrée
void basculer_barriere_sortie(PlanParking* plan);   // Ouvre/ferme barrière sortie

// Places individuelles
int trouver_place_a_position(PlanParking* plan, int ligne, int colonne);  // Retourne index place
void marquer_place_occupee(PlanParking* plan, int index_place);           // Marque place occupée
void marquer_place_libre(PlanParking* plan, int index_place);             // Marque place libre

#endif