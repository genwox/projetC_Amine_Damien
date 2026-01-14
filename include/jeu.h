#ifndef JEU_H
#define JEU_H

#include "plan.h"
#include "liste_car.h"

/* Compteur global de frames (sert pour la synchro / timing du jeu) */
extern unsigned long int global_frame_counter;

/*
 * Petit état du jeu pour éviter d'avoir trop de variables globales.
 * On garde ici ce qui change pendant la boucle principale.
 */
typedef struct
{
    FileAttenteEntree *file_attente; /* voitures en attente à l'entrée */
    int spawn_cd;                    /* cooldown avant le prochain spawn */
    int frame_counter;               /* compteur local de frames */
    int notification_timeout;        /* durée d'affichage des messages */
} JeuState;

/*
 * Boucle principale du jeu (mode parking).
 * Gère le spawn, les déplacements, l'affichage et la fin de partie.
 */
void executer_boucle_jeu(PlanParking *plan, l_car *vehicules, FileAttenteEntree *file_attente);

#endif
