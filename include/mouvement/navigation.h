#ifndef MOUVEMENT_NAVIGATION_H
#define MOUVEMENT_NAVIGATION_H

#include "liste_car.h"
#include "plan.h"

// Suit les flèches du plan et ajuste la direction du véhicule
void suivre_fleches(VEHICULE *vehicule, PlanParking *plan);

// Corrige l'alignement du véhicule sur une flèche proche
void corriger_alignement_vehicule(VEHICULE *vehicule, PlanParking *plan);

#endif
