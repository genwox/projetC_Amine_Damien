#ifndef MOUVEMENT_COLLISION_H
#define MOUVEMENT_COLLISION_H

#include "liste_car.h"
#include "plan.h"

/**
 * Détecte une collision entre deux véhicules.
 * Utilise une tolérance de 2 pixels pour éviter les faux positifs.
 *
 * @param v1 Premier véhicule (doit être non NULL et actif)
 * @param v2 Second véhicule (doit être non NULL et actif)
 * @return   1 si collision détectée, 0 sinon
 */
int vehicules_en_collision(VEHICULE *v1, VEHICULE *v2);

/**
 * Vérifie si un véhicule peut se déplacer à une position.
 * Teste uniquement contre le plan statique (mode basique).
 *
 * @param vehicule  Véhicule à tester (doit être non NULL et actif)
 * @param plan      Plan du parking (doit être non NULL)
 * @param nouveau_x Nouvelle position X
 * @param nouveau_y Nouvelle position Y
 * @return          1 si possible, 0 sinon
 */
int peut_deplacer(VEHICULE *vehicule, PlanParking *plan, int nouveau_x, int nouveau_y);

/**
 * Vérifie si un véhicule peut se déplacer sur une allée.
 * Mode strict: autorise uniquement les allées et routes.
 *
 * @param vehicule  Véhicule à tester (doit être non NULL)
 * @param plan      Plan du parking (doit être non NULL)
 * @param nouveau_x Nouvelle position X
 * @param nouveau_y Nouvelle position Y
 * @return          1 si possible, 0 sinon
 */
int peut_deplacer_sur_allee(VEHICULE *vehicule, PlanParking *plan, int nouveau_x, int nouveau_y);

/**
 * Vérifie si une cellule est roulable (route, flèche, entrée, sortie).
 *
 * @param plan Plan du parking (doit être non NULL)
 * @param x    Colonne à tester
 * @param y    Ligne à tester
 * @return     1 si roulable, 0 sinon
 */
int est_cellule_roulable(PlanParking *plan, int x, int y);

/**
 * Wrapper externe pour est_cellule_roulable.
 * Permet un appel depuis d'autres modules.
 *
 * @param plan Plan du parking (doit être non NULL)
 * @param x    Colonne à tester
 * @param y    Ligne à tester
 * @return     1 si roulable, 0 sinon
 */
int est_cellule_roulable_externe(PlanParking *plan, int x, int y);

/**
 * Vérifie si la voie est libre dans une direction sur une distance donnée.
 * Parcourt un rectangle de 8 cases dans la direction cible.
 *
 * @param vehicule_actuel Véhicule dont on teste la voie (doit être non NULL)
 * @param tous_vehicules  Liste de tous les véhicules (doit être non NULL)
 * @param direction_cible Direction à tester ('N', 'S', 'E', 'O')
 * @param plan            Plan du parking (doit être non NULL)
 * @return                1 si voie libre, 0 si véhicule détecté
 */
int voie_libre_direction(VEHICULE *vehicule_actuel, l_car *tous_vehicules,
                         char direction_cible, PlanParking *plan);

/**
 * Valide une position de véhicule selon un mode de vérification.
 * Mode 0 = basique (bloque murs), Mode 1 = strict (allées seulement).
 *
 * @param v    Véhicule à valider (doit être non NULL)
 * @param plan Plan du parking (doit être non NULL)
 * @param x    Position X à tester
 * @param y    Position Y à tester
 * @param mode Mode de validation (0 = basique, 1 = strict allée)
 * @return     1 si position valide, 0 sinon
 */
int position_valide(VEHICULE *v, PlanParking *plan, int x, int y, int mode);

#endif
