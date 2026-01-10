#ifndef MOUVEMENT_DEPLACEMENT_H
#define MOUVEMENT_DEPLACEMENT_H

#include "liste_car.h"
#include "plan.h"

// Déplace un véhicule selon sa direction actuelle (mode simplifié)
void deplacer_vehicule(VEHICULE *vehicule, PlanParking *plan);

// Déplace un véhicule avec système de parking automatique
void deplacer_vehicule_parking_auto(VEHICULE *vehicule, PlanParking *plan, l_car *tous_vehicules);

// Déplace tous les véhicules et détecte les collisions
int deplacer_tous_vehicules(l_car *vehicules, PlanParking *plan);

#endif
