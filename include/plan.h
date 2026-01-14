#ifndef PLAN_H
#define PLAN_H

#include "matrice.h"
#include <wchar.h>
#include <locale.h>

typedef struct liste_car l_car;

/* Tailles max pour le parking */
#define MAX_HAUTEUR 100
#define MAX_LARGEUR 150
#define MAX_LIGNE 600  /* buffer pour lire les lignes UTF-8 */

/* Les différents caractères qu'on peut avoir dans le plan */
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

/* Une place de parking avec sa position et son état */
typedef struct {
    int ligne;
    int colonne;
    int occupee;  /* 1 = prise, 0 = libre */
} PlaceParking;

/* Une flèche dans le plan pour guider les voitures */
typedef struct {
    int ligne;
    int colonne;
    char direction_entree;
    char direction_sortie;
} FlecheDirection;

/* Structure principale du parking - contient tout l'état du jeu */
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

/* Charge le plan depuis un fichier txt, retourne NULL si ça plante */
PlanParking* charger_plan(const char* fichier_plan);

/* Libère la mémoire du plan */
void detruire_plan(PlanParking** plan);

/* Remplit la matrice d'occupation à partir du plan */
void initialiser_matrice_depuis_plan(PlanParking* plan);

/* On l'utilise plus vraiment mais on la garde au cas où */
void placer_vehicules_sur_plan(PlanParking* plan, l_car* liste_vehicules);

/* Ouvre/ferme la barrière d'entrée */
void basculer_barriere_entree(PlanParking* plan);

/* Ouvre/ferme la barrière de sortie */
void basculer_barriere_sortie(PlanParking* plan);

/* Cherche si y'a une place à cette position, retourne -1 sinon */
int trouver_place_a_position(PlanParking* plan, int ligne, int colonne);

/* Met une place en "occupée" */
void marquer_place_occupee(PlanParking* plan, int index_place);

/* Libère une place */
void marquer_place_libre(PlanParking* plan, int index_place);

#endif
