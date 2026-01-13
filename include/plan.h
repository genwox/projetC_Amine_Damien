#ifndef PLAN_H
#define PLAN_H
#include "matrice.h"
#include <wchar.h>
#include <locale.h>

typedef struct liste_car l_car;

#define MAX_HAUTEUR 100
#define MAX_LARGEUR 150
#define MAX_LIGNE 600
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

typedef struct { int ligne, colonne, occupee; } PlaceParking;
typedef struct { int ligne, colonne; char direction_entree, direction_sortie; } FlecheDirection;

typedef struct plan_parking {
    wchar_t plan_statique[MAX_HAUTEUR][MAX_LARGEUR];
    int hauteur, largeur;
    mat *matrice_occupation;
    int entree_x, entree_y, sortie_x, sortie_y;
    int borne_entree_x, borne_entree_y, borne_sortie_x, borne_sortie_y;
    int barriere_entree_ouverte, barriere_sortie_ouverte;
    int places_libres, places_totales;
    unsigned long int argent_total, score, high_score;
    int vehicules_servis, vehicules_perdus, difficulte;
    PlaceParking places[50];
    FlecheDirection fleches[100];
    int nb_fleches;
} PlanParking;

PlanParking* charger_plan(const char* fichier_plan);
void detruire_plan(PlanParking** plan);
void initialiser_matrice_depuis_plan(PlanParking* plan);
void placer_vehicules_sur_plan(PlanParking* plan, l_car* liste_vehicules);
int est_position_libre(PlanParking* plan, int x, int y);
void occuper_place_parking(PlanParking* plan, int x, int y);
void liberer_place_parking(PlanParking* plan, int x, int y);
void basculer_barriere_entree(PlanParking* plan);
void basculer_barriere_sortie(PlanParking* plan);
int trouver_place_a_position(PlanParking* plan, int ligne, int colonne);
void marquer_place_occupee(PlanParking* plan, int index_place);
void marquer_place_libre(PlanParking* plan, int index_place);
#endif
