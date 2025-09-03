#ifndef LISTE_CAR_H
#define LISTE_CAR_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct voiture VEHICULE;
struct voiture
{
    char direction; /*N => Nord, S => Sud, E => EST, O => OUEST*/
    int posx; /*Position courante coin haut gauche x de la voiture*/
    int posy; /*Position courante coin haut gauche y de la voiture*/
    int vitesse; /*Vitesse du véhicule*/
    char alignement; /*’g’=>gauche ou ’d’=>droite*/
    char type; /*’v’=>voiture, ’c’=>camion, etc.*/
    char Carrosserie[4][30]; /*Carrosserie de la voiture, servira pour
    l’affichage du véhicule à tout moment*/
    int code_couleur; /*Code couleur de la voiture à utiliser lors de
    l’affichage*/
    char etat; /*État du véhicule : ’1’ => actif et ’0’ => inactif*/
    unsigned long int tps;/*pour stocker le temps passé dans le parking*/
    struct voiture* NXT; /*Pointeur vers une prochaine voiture,
    nécessaire pour la liste chaînée*/
};

typedef struct liste_car l_car;
struct liste_car
{
    VEHICULE* premier;
    VEHICULE* dernier;
    int longeur;

};


/* creer un nouveau vehicule*/
VEHICULE* nv_vehicule(char dir, int x, int y, int v, char al, char type, char caro, char color, char etat, char t);

/*creer une nouvelle liste de vehicule*/
l_car* nv_liste_car();

/*detruire un vehicule*/
void detruire_vehicule(VEHICULE** v);


/*detruire liste de voiture */
void detruire_liste_car(l_car** lc);

/*test si la liste de voiture est vide*/
int est_vide_liste_car(l_car* lc);

/*ajoute une voiture au début de la liste */
void ajouter_tete_liste_car(VEHICULE* v, l_car* lc);

/*ajoute un vehicule à la fin de la liste*/
void  ajouter_queue_liste_car(VEHICULE* v, l_car* lc);


#endif