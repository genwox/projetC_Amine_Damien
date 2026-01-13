#ifndef MOUVEMENT_COLLISION_H
#define MOUVEMENT_COLLISION_H

#include "liste_car.h"
#include "plan.h"

int vehicules_en_collision(VEHICULE *v1, VEHICULE *v2);

int peut_deplacer(VEHICULE *vehicule, PlanParking *plan, int nouveau_x, int nouveau_y);

int peut_deplacer_sur_allee(VEHICULE *vehicule, PlanParking *plan, int nouveau_x, int nouveau_y);

int est_cellule_roulable(PlanParking *plan, int x, int y);

int est_cellule_roulable_externe(PlanParking *plan, int x, int y);

int voie_libre_direction(VEHICULE *vehicule_actuel, l_car *tous_vehicules, char direction_cible, PlanParking *plan);

#endif
