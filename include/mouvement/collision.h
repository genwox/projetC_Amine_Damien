#ifndef MOUVEMENT_COLLISION_H
#define MOUVEMENT_COLLISION_H

#include "liste_car.h"
#include "plan.h"

/*
 * Gère les tests de collision et de déplacement des véhicules
 * dans le parking (plan + autres voitures).
 */

/* Teste si deux véhicules se chevauchent */
int vehicules_en_collision(VEHICULE *v1, VEHICULE *v2);

/*
 * Vérifie si un véhicule peut se déplacer à une position donnée.
 * Test simple : murs et limites du plan.
 */
int peut_deplacer(VEHICULE *vehicule, PlanParking *plan, int nouveau_x, int nouveau_y);

/*
 * Vérifie si un véhicule peut se déplacer uniquement sur une allée.
 * Mode plus strict : empêche de rouler hors des routes.
 */
int peut_deplacer_sur_allee(VEHICULE *vehicule, PlanParking *plan, int nouveau_x, int nouveau_y);

/*
 * Indique si une cellule du plan est roulable
 * (route, flèche, entrée ou sortie).
 */
int est_cellule_roulable(PlanParking *plan, int x, int y);

/*
 * Version accessible depuis les autres modules.
 * Sert surtout à éviter les dépendances directes.
 */
int est_cellule_roulable_externe(PlanParking *plan, int x, int y);

/*
 * Vérifie si la voie est libre dans une direction donnée.
 * Permet d'éviter les collisions avec les autres véhicules.
 */
int voie_libre_direction(VEHICULE *vehicule_actuel, l_car *tous_vehicules,
                         char direction_cible, PlanParking *plan);

/*
 * Vérifie si une position est valide pour un véhicule.
 * mode = 0 : test simple
 * mode = 1 : uniquement sur les allées
 */
int position_valide(VEHICULE *v, PlanParking *plan, int x, int y, int mode);

#endif
