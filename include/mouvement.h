#ifndef MOUVEMENT_H
#define MOUVEMENT_H
#include "liste_car.h"
#include "plan.h"

#define LARGEUR_VEHICULE 3
#define HAUTEUR_VEHICULE 4

/*Calcule la largeur visuelle d'une chaîne UTF-8*/
int calculer_largeur_visuelle(const char *str);

/*Obtient les dimensions réelles d'un véhicule*/
void obtenir_dimensions_vehicule(VEHICULE *vehicule, int *largeur, int *hauteur);

/*Vérifie si une position est libre pour un véhicule*/
int peut_deplacer(VEHICULE *vehicule, PlanParking *plan, int nouveau_x, int nouveau_y);

/* Met à jour la direction du véhicule selon les flèches du plan */
void suivre_fleches(VEHICULE *vehicule, PlanParking *plan);

/* Met à jour l'orientation visuelle de la carrosserie selon la direction */
void orienter_carrosserie(VEHICULE *vehicule);

/* Déplace un véhicule dans sa directio*/
void deplacer_vehicule(VEHICULE *vehicule, PlanParking *plan);

/*Déplace tous les véhicules de la liste*/
void deplacer_tous_vehicules(l_car *vehicules, PlanParking *plan);

#endif