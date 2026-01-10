#ifndef JEU_H
#define JEU_H

#include "plan.h"
#include "liste_car.h"

// Compteur global de frames pour le tracking du temps
extern unsigned long int global_frame_counter;

// Exécute la boucle de jeu principale
void executer_boucle_jeu(PlanParking *plan, l_car *vehicules, FileAttenteEntree *file_attente);

#endif
