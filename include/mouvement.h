#ifndef MOUVEMENT_H
#define MOUVEMENT_H
#include "liste_car.h"
#include "plan.h"

#define LARGEUR_VEHICULE 3
#define HAUTEUR_VEHICULE 4
#define CENTRE_VEHICULE(v, cx, cy) \
    do { int w, h; obtenir_dimensions_vehicule(v, &w, &h); \
         cx = v->posx + w/2; cy = v->posy + h/2; } while(0)

int calculer_largeur_visuelle(const char *str);
void obtenir_dimensions_vehicule(VEHICULE *vehicule, int *largeur, int *hauteur);
int peut_deplacer(VEHICULE *vehicule, PlanParking *plan, int nouveau_x, int nouveau_y);
void suivre_fleches(VEHICULE *vehicule, PlanParking *plan);
const char* obtenir_fichier_sprite(char direction);
void orienter_carrosserie(VEHICULE *vehicule);
int charger_sprite_direction(VEHICULE *vehicule, char direction);
void deplacer_vehicule(VEHICULE *vehicule, PlanParking *plan);
int deplacer_tous_vehicules(l_car *vehicules, PlanParking *plan);
void marquer_vehicule_en_sortie(VEHICULE *vehicule);
void corriger_alignement_vehicule(VEHICULE *vehicule, PlanParking *plan);
#endif
