#ifndef MOUVEMENT_SPRITES_H
#define MOUVEMENT_SPRITES_H

#include "liste_car.h"

int calculer_largeur_visuelle(const char *str);

void obtenir_dimensions_vehicule(VEHICULE *vehicule, int *largeur, int *hauteur);

const char *obtenir_fichier_sprite(char direction);

int charger_sprite_direction(VEHICULE *vehicule, char direction);

void orienter_carrosserie(VEHICULE *vehicule);

#endif
