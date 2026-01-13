#ifndef LISTE_CAR_H
#define LISTE_CAR_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plan.h"

typedef struct voiture VEHICULE;
struct voiture {
    char direction, alignement, type, etat;
    int posx, posy, vitesse, code_couleur;
    char Carrosserie[4][30];
    unsigned long int tps, temps_attente;
    struct voiture *NXT;
};

typedef struct liste_car { VEHICULE *premier, *dernier; int longeur; } l_car;
typedef struct file_attente_entree {
    VEHICULE *premier_attente, *dernier_attente;
    int longueur_attente, longueur_max;
} FileAttenteEntree;

VEHICULE *nv_vehicule(char dir, int x, int y, int v, char al, char type, char **caro, char color, char etat, char t);
void detruire_vehicule(VEHICULE **v);
l_car *nv_liste_car();
int est_vide_liste_car(l_car *lc);
void ajouter_tete_liste_car(VEHICULE *v, l_car *lc);
void ajouter_queue_liste_car(VEHICULE *v, l_car *lc);
void detruire_tete_liste_car(l_car *lc);
void detruire_queue_liste_car(l_car *lc);
void detruire_vehicule_specifique(l_car *lc, VEHICULE *v);
void detruire_liste_car(l_car **lc);
VEHICULE *creer_voiture_aleatoire(PlanParking *plan);
char **charger_modele_voiture(const char *fich);
l_car *initialiser_vehicules(PlanParking *plan, int nb);
FileAttenteEntree* creer_file_attente(int longueur_max);
void detruire_file_attente(FileAttenteEntree **file);
int ajouter_a_file_attente(FileAttenteEntree *file, VEHICULE *v, unsigned long frame);
VEHICULE* retirer_de_file_attente(FileAttenteEntree *file);
int file_attente_est_pleine(FileAttenteEntree *file);
int file_attente_est_vide(FileAttenteEntree *file);
void supprimer_vehicule_file(FileAttenteEntree *file, VEHICULE *v);
void parcourir_liste(l_car *liste, void (*callback)(VEHICULE*, void*), void *ctx);
VEHICULE* trouver_vehicule(l_car *liste, int (*test)(VEHICULE*, void*), void *ctx);
int compter_vehicules(l_car *liste, int (*filtre)(VEHICULE*, void*), void *ctx);
#endif
