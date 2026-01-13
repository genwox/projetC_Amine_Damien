#ifndef JEU_H
#define JEU_H

#include "plan.h"
#include "liste_car.h"

/* Compteur global de frames pour synchronisation */
extern unsigned long int global_frame_counter;

/**
 * État du jeu pour la boucle principale.
 *
 * @field file_attente         File d'attente des véhicules à l'entrée
 * @field spawn_cd             Cooldown avant prochain spawn (en frames)
 * @field frame_counter        Compteur de frames local
 * @field notification_timeout Timeout des notifications affichées
 */
typedef struct {
    FileAttenteEntree *file_attente;
    int spawn_cd;
    int frame_counter;
    int notification_timeout;
} JeuState;

/**
 * Exécute la boucle de jeu principale du mode parking.
 * Gère spawn, déplacements, collisions, affichage, game over.
 *
 * @param plan         Plan du parking (doit être non NULL)
 * @param vehicules    Liste des véhicules actifs (doit être non NULL)
 * @param file_attente File d'attente d'entrée (doit être non NULL)
 */
void executer_boucle_jeu(PlanParking *plan, l_car *vehicules, FileAttenteEntree *file_attente);

#endif
