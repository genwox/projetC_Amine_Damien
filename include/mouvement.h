#ifndef MOUVEMENT_H
#define MOUVEMENT_H

#include "liste_car.h"
#include "plan.h"

/* Dimensions par défaut des véhicules (non utilisées actuellement, calcul dynamique) */
#define LARGEUR_VEHICULE 3
#define HAUTEUR_VEHICULE 4

/**
 * Calcule le centre d'un véhicule.
 *
 * @param vehicule Véhicule à mesurer (doit être non NULL)
 * @param cx       Pointeur où stocker le centre X (doit être non NULL)
 * @param cy       Pointeur où stocker le centre Y (doit être non NULL)
 */
void calculer_centre_vehicule(VEHICULE *vehicule, int *cx, int *cy);

/**
 * Calcule la largeur visuelle d'une chaîne UTF-8.
 * Un caractère UTF-8 peut occuper 1 à 4 octets mais compte pour 1 colonne.
 *
 * @param str Chaîne à mesurer (peut être NULL)
 * @return    Largeur en colonnes (nombre de caractères UTF-8)
 */
int calculer_largeur_visuelle(const char *str);

/**
 * Calcule les dimensions réelles d'un véhicule à partir de son sprite.
 *
 * @param vehicule Véhicule à mesurer (doit être non NULL)
 * @param largeur  Pointeur où écrire la largeur (doit être non NULL)
 * @param hauteur  Pointeur où écrire la hauteur (doit être non NULL)
 */
void obtenir_dimensions_vehicule(VEHICULE *vehicule, int *largeur, int *hauteur);

/**
 * Vérifie si un véhicule peut se déplacer à une position donnée.
 * Teste uniquement les collisions avec le plan statique.
 *
 * @param vehicule  Véhicule à tester (doit être non NULL et actif)
 * @param plan      Plan du parking (doit être non NULL)
 * @param nouveau_x Nouvelle position X
 * @param nouveau_y Nouvelle position Y
 * @return          1 si le déplacement est possible, 0 sinon
 */
int peut_deplacer(VEHICULE *vehicule, PlanParking *plan, int nouveau_x, int nouveau_y);

/**
 * Fait suivre les flèches directionnelles au véhicule.
 * Change la direction du véhicule si une flèche est détectée proche.
 *
 * @param vehicule Véhicule à guider (doit être non NULL)
 * @param plan     Plan du parking (doit être non NULL)
 */
void suivre_fleches(VEHICULE *vehicule, PlanParking *plan);

/**
 * Charge et oriente le sprite du véhicule selon sa direction.
 * Lit le fichier car_smallN/S/E/O.txt correspondant.
 *
 * @param vehicule Véhicule dont charger le sprite (doit être non NULL)
 */
void orienter_carrosserie(VEHICULE *vehicule);

/**
 * Déplace un véhicule d'une case selon sa direction.
 * Gère le pathfinding, la détection de places, les barrières.
 *
 * @param vehicule Véhicule à déplacer (doit être non NULL)
 * @param plan     Plan du parking (doit être non NULL)
 */
void deplacer_vehicule(VEHICULE *vehicule, PlanParking *plan);

/**
 * Déplace tous les véhicules actifs de la liste.
 * Détecte les sorties, gère les collisions, supprime les sortis.
 *
 * @param vehicules Liste des véhicules (doit être non NULL)
 * @param plan      Plan du parking (doit être non NULL)
 * @return          1 si collision détectée, 0 sinon
 */
int deplacer_tous_vehicules(l_car *vehicules, PlanParking *plan);

/**
 * Marque un véhicule comme devant sortir du parking.
 * Change sa cible vers la sortie.
 *
 * @param vehicule Véhicule à marquer (doit être non NULL)
 */
void marquer_vehicule_en_sortie(VEHICULE *vehicule);

/**
 * Corrige l'alignement d'un véhicule par rapport aux flèches.
 * Ajuste la position pour centrer le véhicule sur sa voie.
 *
 * @param vehicule Véhicule à corriger (doit être non NULL)
 * @param plan     Plan du parking (doit être non NULL)
 */
void corriger_alignement_vehicule(VEHICULE *vehicule, PlanParking *plan);

#endif
