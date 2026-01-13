#ifndef LISTE_CAR_H
#define LISTE_CAR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plan.h"

/**
 * Structure représentant un véhicule dans le parking.
 *
 * @field direction   Direction actuelle ('N', 'S', 'E', 'O')
 * @field alignement  Alignement du véhicule dans sa voie
 * @field type        Type de véhicule ('C' pour voiture, etc.)
 * @field etat        État du véhicule ('1' actif, '0' garé, '-' sortie)
 * @field posx        Position X en colonnes
 * @field posy        Position Y en lignes
 * @field vitesse     Vitesse de déplacement (frames entre mouvements)
 * @field code_couleur Code couleur pour l'affichage
 * @field Carrosserie  Sprite visuel du véhicule (4 lignes x 30 caractères)
 * @field tps          Temps passé dans le parking (en frames)
 * @field temps_attente Temps d'attente en file d'entrée (en frames)
 * @field NXT          Pointeur vers le prochain véhicule (liste chaînée)
 */
typedef struct voiture VEHICULE;
struct voiture {
    char direction;
    char alignement;
    char type;
    char etat;
    int posx;
    int posy;
    int vitesse;
    int code_couleur;
    char Carrosserie[4][30];
    unsigned long int tps;
    unsigned long int temps_attente;
    struct voiture *NXT;
};

/**
 * Liste chaînée de véhicules.
 *
 * @field premier Pointeur vers le premier véhicule
 * @field dernier Pointeur vers le dernier véhicule
 * @field longeur Nombre de véhicules dans la liste
 */
typedef struct liste_car {
    VEHICULE *premier;
    VEHICULE *dernier;
    int longeur;
} l_car;

/**
 * File d'attente pour les véhicules à l'entrée du parking.
 *
 * @field premier_attente Pointeur vers le premier véhicule en attente
 * @field dernier_attente Pointeur vers le dernier véhicule en attente
 * @field longueur_attente Nombre de véhicules actuellement en attente
 * @field longueur_max     Capacité maximale de la file
 */
typedef struct file_attente_entree {
    VEHICULE *premier_attente;
    VEHICULE *dernier_attente;
    int longueur_attente;
    int longueur_max;
} FileAttenteEntree;

/**
 * Crée un nouveau véhicule avec les paramètres spécifiés.
 *
 * @param dir   Direction initiale ('N', 'S', 'E', 'O')
 * @param x     Position X de départ
 * @param y     Position Y de départ
 * @param v     Vitesse (frames entre mouvements)
 * @param al    Alignement dans la voie
 * @param type  Type de véhicule
 * @param caro  Sprite du véhicule (tableau de 4 chaînes, peut être NULL)
 * @param color Code couleur pour l'affichage
 * @param etat  État initial du véhicule ('1' actif)
 * @param t     Temps initial
 * @return      Pointeur vers le nouveau véhicule alloué, NULL en cas d'erreur
 */
VEHICULE *nv_vehicule(char dir, int x, int y, int v, char al, char type, char **caro, char color, char etat, char t);

/**
 * Détruit un véhicule et libère sa mémoire.
 *
 * @param v Pointeur vers le pointeur du véhicule (sera mis à NULL après destruction)
 */
void detruire_vehicule(VEHICULE **v);

/**
 * Crée une nouvelle liste de véhicules vide.
 *
 * @return Pointeur vers la liste créée, NULL en cas d'erreur d'allocation
 */
l_car *nv_liste_car();

/**
 * Vérifie si une liste de véhicules est vide.
 *
 * @param lc Liste à vérifier (peut être NULL)
 * @return   1 si la liste est vide ou NULL, 0 sinon
 */
int est_vide_liste_car(l_car *lc);

/**
 * Ajoute un véhicule en tête de liste.
 *
 * @param v  Véhicule à ajouter (doit être non NULL)
 * @param lc Liste destination (doit être non NULL)
 */
void ajouter_tete_liste_car(VEHICULE *v, l_car *lc);

/**
 * Ajoute un véhicule en queue de liste.
 *
 * @param v  Véhicule à ajouter (doit être non NULL)
 * @param lc Liste destination (doit être non NULL)
 */
void ajouter_queue_liste_car(VEHICULE *v, l_car *lc);

/**
 * Supprime et détruit le véhicule en tête de liste.
 *
 * @param lc Liste à modifier (doit être non NULL et non vide)
 */
void detruire_tete_liste_car(l_car *lc);

/**
 * Supprime et détruit le véhicule en queue de liste.
 *
 * @param lc Liste à modifier (doit être non NULL et non vide)
 */
void detruire_queue_liste_car(l_car *lc);

/**
 * Recherche et supprime un véhicule spécifique dans la liste.
 *
 * @param lc Liste où chercher (doit être non NULL)
 * @param v  Véhicule à supprimer (doit être non NULL)
 */
void detruire_vehicule_specifique(l_car *lc, VEHICULE *v);

/**
 * Détruit une liste entière et tous ses véhicules.
 *
 * @param lc Pointeur vers le pointeur de liste (sera mis à NULL après destruction)
 */
void detruire_liste_car(l_car **lc);

/**
 * Crée un véhicule aléatoire à une position d'entrée du plan.
 *
 * @param plan Plan du parking (doit être non NULL avec entree_x/entree_y valides)
 * @return     Pointeur vers le véhicule créé, NULL en cas d'erreur
 */
VEHICULE *creer_voiture_aleatoire(PlanParking *plan);

/**
 * Charge un modèle de sprite de véhicule depuis un fichier.
 *
 * @param fich Chemin du fichier de sprite (doit exister)
 * @return     Tableau de 4 chaînes allouées contenant le sprite, NULL en cas d'erreur
 */
char **charger_modele_voiture(const char *fich);

/**
 * Initialise une liste avec un nombre donné de véhicules aléatoires.
 *
 * @param plan Plan du parking (doit être non NULL)
 * @param nb   Nombre de véhicules à créer (doit être >= 0)
 * @return     Liste de véhicules créée et peuplée, NULL en cas d'erreur
 */
l_car *initialiser_vehicules(PlanParking *plan, int nb);

/**
 * Crée une nouvelle file d'attente pour l'entrée du parking.
 *
 * @param longueur_max Capacité maximale de la file (doit être > 0)
 * @return             Pointeur vers la file créée, NULL en cas d'erreur
 */
FileAttenteEntree* creer_file_attente(int longueur_max);

/**
 * Détruit une file d'attente et tous les véhicules qu'elle contient.
 *
 * @param file Pointeur vers le pointeur de file (sera mis à NULL après destruction)
 */
void detruire_file_attente(FileAttenteEntree **file);

/**
 * Ajoute un véhicule à la file d'attente d'entrée.
 *
 * @param file  File d'attente (doit être non NULL et non pleine)
 * @param v     Véhicule à ajouter (doit être non NULL)
 * @param frame Frame actuelle du jeu (pour timestamp)
 * @return      1 en cas de succès, 0 si la file est pleine ou paramètres invalides
 */
int ajouter_a_file_attente(FileAttenteEntree *file, VEHICULE *v, unsigned long frame);

/**
 * Retire et retourne le premier véhicule de la file d'attente.
 *
 * @param file File d'attente (doit être non NULL et non vide)
 * @return     Pointeur vers le véhicule retiré, NULL si file vide ou invalide
 */
VEHICULE* retirer_de_file_attente(FileAttenteEntree *file);

/**
 * Vérifie si la file d'attente est pleine.
 *
 * @param file File à vérifier (doit être non NULL)
 * @return     1 si pleine, 0 sinon
 */
int file_attente_est_pleine(FileAttenteEntree *file);

/**
 * Vérifie si la file d'attente est vide.
 *
 * @param file File à vérifier (peut être NULL)
 * @return     1 si vide ou NULL, 0 sinon
 */
int file_attente_est_vide(FileAttenteEntree *file);

/**
 * Supprime un véhicule spécifique de la file d'attente.
 *
 * @param file File d'attente (doit être non NULL)
 * @param v    Véhicule à supprimer (doit être non NULL et présent dans la file)
 */
void supprimer_vehicule_file(FileAttenteEntree *file, VEHICULE *v);

#endif
