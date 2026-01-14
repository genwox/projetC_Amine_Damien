#ifndef MOUVEMENT_SPRITES_H
#define MOUVEMENT_SPRITES_H

#include "liste_car.h"

/*
 * Calcule la largeur "à l'écran" d'une chaîne UTF-8.
 * On considère qu'un caractère UTF-8 (même s'il prend plusieurs octets)
 * compte pour 1 colonne.
 *
 * str peut être NULL.
 */
int calculer_largeur_visuelle(const char *str);

/*
 * Récupère la taille réelle du véhicule à partir de son sprite.
 * On parcourt Carrosserie[4][30] pour trouver la largeur/hauteur effectivement utilisées.
 *
 * vehicule, largeur et hauteur doivent être non NULL.
 */
void obtenir_dimensions_vehicule(VEHICULE *vehicule, int *largeur, int *hauteur);

/*
 * Charge le sprite du véhicule selon sa direction (N/S/E/O).
 * Lit le fichier correspondant (car_smallN/S/E/O.txt) et remplit Carrosserie[4][30].
 *
 * vehicule doit être non NULL.
 */
void orienter_carrosserie(VEHICULE *vehicule);

#endif
