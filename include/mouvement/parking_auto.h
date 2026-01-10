#ifndef MOUVEMENT_PARKING_AUTO_H
#define MOUVEMENT_PARKING_AUTO_H

#include "liste_car.h"
#include "plan.h"

// Marque un véhicule comme étant en mode sortie
void marquer_vehicule_en_sortie(VEHICULE *vehicule);

// Trouve une place de parking libre proche du véhicule
int trouver_place_libre_proche(VEHICULE *vehicule, PlanParking *plan);

// Déplace un véhicule vers une place de parking cible
void deplacer_vers_place(VEHICULE *vehicule, PlanParking *plan, int index_place);

#endif
