#ifndef MOUVEMENT_H
#define MOUVEMENT_H

#include "liste_car.h"
#include "plan.h"

/* Taille par défaut des voitures (mais on calcule dynamiquement maintenant) */
#define LARGEUR_VEHICULE 3
#define HAUTEUR_VEHICULE 4

/* Calcule le centre de la voiture */
void calculer_centre_vehicule(VEHICULE *vehicule, int *cx, int *cy);

/* Compte la largeur d'une chaîne UTF-8 (un caractère = 1 à 4 octets) */
int calculer_largeur_visuelle(const char *str);

/* Récupère la taille de la voiture à partir de son sprite */
void obtenir_dimensions_vehicule(VEHICULE *vehicule, int *largeur, int *hauteur);

/* Vérifie si la voiture peut aller à cette position */
int peut_deplacer(VEHICULE *vehicule, PlanParking *plan, int nouveau_x, int nouveau_y);

/* Fait tourner la voiture si elle passe sur une flèche */
void suivre_fleches(VEHICULE *vehicule, PlanParking *plan);

/* Charge le bon sprite selon la direction (N/S/E/O) */
void orienter_carrosserie(VEHICULE *vehicule);

/* Déplace une voiture d'une case */
void deplacer_vehicule(VEHICULE *vehicule, PlanParking *plan);

/* Déplace toutes les voitures, retourne 1 si collision */
int deplacer_tous_vehicules(l_car *vehicules, PlanParking *plan);

/* Dit à une voiture qu'elle doit partir */
void marquer_vehicule_en_sortie(VEHICULE *vehicule);

/* Recentre la voiture sur sa voie */
void corriger_alignement_vehicule(VEHICULE *vehicule, PlanParking *plan);

#endif
