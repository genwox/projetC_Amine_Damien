#ifndef JEU_H
#define JEU_H
#include "plan.h"
#include "liste_car.h"

extern unsigned long int global_frame_counter;

typedef struct {
    FileAttenteEntree *file_attente;
    int spawn_cd, frame_counter, notification_timeout;
} JeuState;

void executer_boucle_jeu(PlanParking *plan, l_car *vehicules, FileAttenteEntree *file_attente);
#endif
