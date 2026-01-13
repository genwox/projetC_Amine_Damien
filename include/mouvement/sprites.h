#ifndef MOUVEMENT_SPRITES_H
#define MOUVEMENT_SPRITES_H

#include "liste_car.h"

/**
 * Calcule la largeur visuelle d'une chaîne UTF-8 en colonnes.
 * Un caractère UTF-8 multi-octets compte pour 1 colonne.
 *
 * @param str Chaîne à mesurer (peut être NULL)
 * @return    Nombre de colonnes (caractères visuels)
 */
int calculer_largeur_visuelle(const char *str);

/**
 * Calcule les dimensions réelles d'un véhicule depuis son sprite.
 * Parcourt le tableau Carrosserie[4][30] pour trouver la taille effective.
 *
 * @param vehicule Véhicule à mesurer (doit être non NULL)
 * @param largeur  Pointeur où écrire la largeur en colonnes (doit être non NULL)
 * @param hauteur  Pointeur où écrire la hauteur en lignes (doit être non NULL)
 */
void obtenir_dimensions_vehicule(VEHICULE *vehicule, int *largeur, int *hauteur);

/**
 * Charge et oriente le sprite du véhicule selon sa direction actuelle.
 * Lit le fichier car_smallN/S/E/O.txt et remplit Carrosserie[4][30].
 *
 * @param vehicule Véhicule dont charger le sprite (doit être non NULL)
 */
void orienter_carrosserie(VEHICULE *vehicule);

#endif
