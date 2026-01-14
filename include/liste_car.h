#ifndef LISTE_CAR_H
#define LISTE_CAR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plan.h"

/* Structure d'une voiture */
typedef struct voiture VEHICULE;
struct voiture {
    char direction;    /* N, S, E ou O */
    char alignement;
    char type;
    char etat;         /* '1' = roule, '0' = garé, '-' = parti */
    int posx;
    int posy;
    int vitesse;
    int code_couleur;
    char Carrosserie[4][30];  /* le dessin de la voiture */
    unsigned long int tps;
    unsigned long int temps_attente;
    struct voiture *NXT;      /* pour la liste chaînée */
};

/* Liste chaînée de voitures */
typedef struct liste_car {
    VEHICULE *premier;
    VEHICULE *dernier;
    int longeur;  /* oui on a fait une typo mais on la garde */
} l_car;

/* File d'attente à l'entrée */
typedef struct file_attente_entree {
    VEHICULE *premier_attente;
    VEHICULE *dernier_attente;
    int longueur_attente;
    int longueur_max;
} FileAttenteEntree;

/* Crée une nouvelle voiture */
VEHICULE *nv_vehicule(char dir, int x, int y, int v, char al, char type, char **caro, char color, char etat, char t);

/* Supprime une voiture */
void detruire_vehicule(VEHICULE **v);

/* Crée une liste vide */
l_car *nv_liste_car();

/* Vérifie si la liste est vide */
int est_vide_liste_car(l_car *lc);

/* Ajoute une voiture au début */
void ajouter_tete_liste_car(VEHICULE *v, l_car *lc);

/* Ajoute une voiture à la fin */
void ajouter_queue_liste_car(VEHICULE *v, l_car *lc);

/* Supprime la première voiture */
void detruire_tete_liste_car(l_car *lc);

/* Supprime la dernière voiture */
void detruire_queue_liste_car(l_car *lc);

/* Cherche et supprime une voiture précise */
void detruire_vehicule_specifique(l_car *lc, VEHICULE *v);

/* Détruit toute la liste */
void detruire_liste_car(l_car **lc);

/* Crée une voiture random à l'entrée */
VEHICULE *creer_voiture_aleatoire(PlanParking *plan);

/* Charge le sprite depuis un fichier */
char **charger_modele_voiture(const char *fich);

/* Crée une liste avec des voitures dedans */
l_car *initialiser_vehicules(PlanParking *plan, int nb);

/* Crée la file d'attente */
FileAttenteEntree* creer_file_attente(int longueur_max);

/* Détruit la file d'attente */
void detruire_file_attente(FileAttenteEntree **file);

/* Ajoute une voiture dans la file */
int ajouter_a_file_attente(FileAttenteEntree *file, VEHICULE *v, unsigned long frame);

/* Retire la première voiture de la file */
VEHICULE* retirer_de_file_attente(FileAttenteEntree *file);

/* Vérifie si la file est pleine */
int file_attente_est_pleine(FileAttenteEntree *file);

/* Vérifie si la file est vide */
int file_attente_est_vide(FileAttenteEntree *file);

/* Enlève une voiture précise de la file */
void supprimer_vehicule_file(FileAttenteEntree *file, VEHICULE *v);

#endif
