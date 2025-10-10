#ifndef PLAN_H
#define PLAN_H

#include "matrice.h"
#include "liste_car.h"

#define MAX_HAUTEUR 100
#define MAX_LARGEUR 150
#define MAX_LIGNE 200

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
#define BG_ROUGE        "\x1b[41m"
#define BG_VERT         "\x1b[42m"
#define BG_JAUNE        "\x1b[43m"
#define BG_BLEU         "\x1b[44m"
#define BG_CYAN         "\x1b[46m"

// Caractères du plan
#define MUR '#'
#define ROUTE ' '
#define PLACE_LIBRE 'P'
#define PLACE_OCCUPEE 'X'
#define ENTREE 'E'
#define SORTIE 'S'
#define BORNE_ENTREE 'T'
#define BORNE_SORTIE 'P' 
#define ALLEE 'A'
#define FLECHE_HAUT '^'
#define FLECHE_BAS 'v'
#define FLECHE_GAUCHE '<'
#define FLECHE_DROITE '>'

// Structure pour gérer le plan du parking
typedef struct plan_parking {
    char plan_statique[MAX_HAUTEUR][MAX_LARGEUR];
    char plan_dynamique[MAX_HAUTEUR][MAX_LARGEUR];
    int hauteur;
    int largeur;
    mat *matrice_occupation;
    
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
void afficher_plan_couleur(PlanParking* plan);
char* obtenir_couleur_caractere(char c, int est_place_libre);

#endif