#ifndef LISTE_CAR_H
#define LISTE_CAR_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plan.h"

typedef struct voiture VEHICULE;
struct voiture
{
    char direction;          /*N => Nord, S => Sud, E => EST, O => OUEST*/
    int posx;                /*Position coin haut gauche: INDICE CELLULE (colonne wchar dans plan_statique[][])*/
    int posy;                /*Position coin haut gauche: INDICE CELLULE (ligne wchar dans plan_statique[][])*/
    int vitesse;             /*Vitesse du véhicule (en cellules par frame)*/
    char alignement;         /*'g'=>gauche ou 'd'=>droite*/
    char type;               /*'v'=>voiture, 'c'=>camion, etc.*/
    char Carrosserie[4][30]; /*Carrosserie de la voiture, servira pour
    l'affichage du véhicule à tout moment*/
    int code_couleur;        /*Code couleur de la voiture à utiliser lors de
           l'affichage*/
    char etat;               /*État du véhicule : '1' => actif, '0' => inactif, '2' => en attente*/
    unsigned long int tps;   /*pour stocker le temps passé dans le parking*/
    unsigned long int temps_attente;  /*Frame d'entrée en file (0 = pas en attente)*/
    struct voiture *NXT;     /*Pointeur vers une prochaine voiture,
        nécessaire pour la liste chaînée*/
};

typedef struct liste_car l_car;
struct liste_car
{
    VEHICULE *premier;
    VEHICULE *dernier;
    int longeur;
};

// File d'attente des véhicules à l'entrée
typedef struct file_attente_entree FileAttenteEntree;
struct file_attente_entree
{
    VEHICULE *premier_attente;        // Premier véhicule en attente
    VEHICULE *dernier_attente;        // Dernier véhicule en attente
    int longueur_attente;             // Nombre de véhicules en attente
    int longueur_max;                 // Capacité max (10)
};

/* creer un nouveau vehicule*/
VEHICULE *nv_vehicule(char dir, int x, int y, int v, char al, char type, char **caro, char color, char etat, char t);

/*creer une nouvelle liste de vehicule*/
l_car *nv_liste_car();

/*detruire un vehicule*/
void detruire_vehicule(VEHICULE **v);

/*test si la liste de voiture est vide*/
int est_vide_liste_car(l_car *lc);

/*ajoute une voiture au début de la liste */
void ajouter_tete_liste_car(VEHICULE *v, l_car *lc);

/*ajoute un vehicule à la fin de la liste*/
void ajouter_queue_liste_car(VEHICULE *v, l_car *lc);

void detruire_tete_liste_car(l_car *lc);

void detruire_queue_liste_car(l_car *lc);

/*detruire un vehicule spécifique dans la liste (au milieu)*/
void detruire_vehicule_specifique(l_car *lc, VEHICULE *v);

/*detruire liste de voiture */
void detruire_liste_car(l_car **lc);

/*Génère une voiture de manière aléatoires*/
VEHICULE *creer_voiture_aleatoire(PlanParking *plan);

/*Charge le modèle visuel depuis un fichier txt*/
char **charger_modele_voiture(const char *fich);

/*Initialise une liste de véhicules aléatoires*/
l_car *initialiser_vehicules(PlanParking *plan, int nombre);

// Gestion de la file d'attente
FileAttenteEntree* creer_file_attente(int longueur_max);
void detruire_file_attente(FileAttenteEntree **file);
int ajouter_a_file_attente(FileAttenteEntree *file, VEHICULE *v, unsigned long frame);
VEHICULE* retirer_de_file_attente(FileAttenteEntree *file);
int file_attente_est_pleine(FileAttenteEntree *file);
int file_attente_est_vide(FileAttenteEntree *file);
void supprimer_vehicule_file(FileAttenteEntree *file, VEHICULE *v);

#endif