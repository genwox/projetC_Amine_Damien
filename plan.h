#ifndef PLAN_H
#define PLAN_H

#include "matrice.h"
#include "liste_car.h"

#define TAILLE_PLAN 50
#define MAX_LIGNE 100

// Caractères du plan
#define MUR '#'
#define ROUTE ' '
#define PLACE_LIBRE 'L'
#define PLACE_OCCUPEE 'O'
#define ENTREE 'E'
#define SORTIE 'S'
#define BORNE_ENTREE 'T'  // Ticket
#define BORNE_SORTIE 'P'  // Paiement
#define BARRIERE_FERMEE '|'
#define BARRIERE_OUVERTE '-'

// Structure pour gérer le plan du parking
typedef struct plan_parking {
    char plan_statique[TAILLE_PLAN][TAILLE_PLAN];   // Plan de base fixe
    char plan_dynamique[TAILLE_PLAN][TAILLE_PLAN];  // Plan avec véhicules
    int hauteur;
    int largeur;
    mat *matrice_occupation;  // Matrice d'occupation existante
    
    // Coordonnées importantes
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
} PlanParking;

// Fonctions de gestion du plan
PlanParking* charger_plan(const char* fichier_plan);
void detruire_plan(PlanParking** plan);
void initialiser_matrice_depuis_plan(PlanParking* plan);
void copier_plan_statique_vers_dynamique(PlanParking* plan);
void placer_vehicules_sur_plan(PlanParking* plan, l_car* liste_vehicules);
int est_position_libre(PlanParking* plan, int x, int y);
void occuper_place_parking(PlanParking* plan, int x, int y);
void liberer_place_parking(PlanParking* plan, int x, int y);
void basculer_barriere_entree(PlanParking* plan);
void basculer_barriere_sortie(PlanParking* plan);
void afficher_infos_parking(PlanParking* plan);

#endif