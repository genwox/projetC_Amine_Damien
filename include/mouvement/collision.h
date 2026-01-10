#ifndef MOUVEMENT_COLLISION_H
#define MOUVEMENT_COLLISION_H

#include "liste_car.h"
#include "plan.h"

// Détecte si deux véhicules sont en collision (AABB)
int vehicules_en_collision(VEHICULE *v1, VEHICULE *v2);

// Vérifie si un véhicule peut se déplacer à une nouvelle position (limites + murs)
int peut_deplacer(VEHICULE *vehicule, PlanParking *plan, int nouveau_x, int nouveau_y);

// Vérifie si un véhicule peut se déplacer sur une allée (contrôle strict)
int peut_deplacer_sur_allee(VEHICULE *vehicule, PlanParking *plan, int nouveau_x, int nouveau_y);

// Vérifie si une cellule est roulable
int est_cellule_roulable(PlanParking *plan, int x, int y);

// Vérifie si une cellule est roulable (version externe, appelable depuis jeu.c)
int est_cellule_roulable_externe(PlanParking *plan, int x, int y);

// Vérifie si la voie est libre dans une direction pour éviter les collisions
int voie_libre_direction(VEHICULE *vehicule_actuel, l_car *tous_vehicules, char direction_cible, PlanParking *plan);

#endif
