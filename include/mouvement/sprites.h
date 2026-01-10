#ifndef MOUVEMENT_SPRITES_H
#define MOUVEMENT_SPRITES_H

#include "liste_car.h"

// Calcule la largeur visuelle d'une ligne de sprite (pour centrage)
int calculer_largeur_visuelle(const char *str);

// Obtient les dimensions (largeur, hauteur) du sprite d'un véhicule
void obtenir_dimensions_vehicule(VEHICULE *vehicule, int *largeur, int *hauteur);

// Retourne le nom du fichier sprite selon la direction
const char *obtenir_fichier_sprite(char direction);

// Charge un sprite depuis un fichier et l'oriente dans la direction donnée
int charger_sprite_direction(VEHICULE *vehicule, char direction);

// Oriente la carrosserie du véhicule selon sa direction actuelle
void orienter_carrosserie(VEHICULE *vehicule);

#endif
