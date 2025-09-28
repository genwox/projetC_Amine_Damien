#include "plan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PlanParking* charger_plan(const char* fichier_plan)
{
    FILE* fichier = fopen(fichier_plan, "r");
    if (!fichier) {
        printf("Erreur : impossible d'ouvrir le fichier %s\n", fichier_plan);
        return NULL;
    }
    
    PlanParking* plan = malloc(sizeof(PlanParking));
    if (!plan) {
        fclose(fichier);
        return NULL;
    }
    
    // Initialisation
    plan->hauteur = TAILLE_PLAN;
    plan->largeur = TAILLE_PLAN;
    plan->places_libres = 0;
    plan->places_totales = 0;
    plan->barriere_entree_ouverte = 0;
    plan->barriere_sortie_ouverte = 0;
    
    // Lecture du fichier ligne par ligne
    char ligne[MAX_LIGNE];
    int ligne_courante = 0;
    
    while (fgets(ligne, MAX_LIGNE, fichier) && ligne_courante < TAILLE_PLAN) {
        for (int j = 0; j < TAILLE_PLAN && j < strlen(ligne); j++) {
            char c = ligne[j];
            plan->plan_statique[ligne_courante][j] = c;
            
            // Mémoriser les positions importantes
            switch(c) {
                case ENTREE:
                    plan->entree_x = ligne_courante;
                    plan->entree_y = j;
                    break;
                case SORTIE:
                    plan->sortie_x = ligne_courante;
                    plan->sortie_y = j;
                    break;
                case BORNE_ENTREE:
                    plan->borne_entree_x = ligne_courante;
                    plan->borne_entree_y = j;
                    break;
                case BORNE_SORTIE:
                    plan->borne_sortie_x = ligne_courante;
                    plan->borne_sortie_y = j;
                    break;
                case PLACE_LIBRE:
                    plan->places_libres++;
                    plan->places_totales++;
                    break;
            }
        }
        ligne_courante++;
    }
    
    fclose(fichier);
    
    // Créer la matrice d'occupation
    plan->matrice_occupation = creer_matrice(TAILLE_PLAN, TAILLE_PLAN);
    initialiser_matrice_depuis_plan(plan);
    
    // Copier le plan statique vers dynamique
    copier_plan_statique_vers_dynamique(plan);
    
    return plan;
}

void detruire_plan(PlanParking** plan)
{
    if (*plan) {
        detruire_matrice(&((*plan)->matrice_occupation));
        free(*plan);
        *plan = NULL;
    }
}

void initialiser_matrice_depuis_plan(PlanParking* plan)
{
    for (int i = 0; i < TAILLE_PLAN; i++) {
        for (int j = 0; j < TAILLE_PLAN; j++) {
            char c = plan->plan_statique[i][j];
            if (c == MUR || c == PLACE_OCCUPEE) {
                remplir_case(plan->matrice_occupation, i, j);
            } else {
                liberer_case(plan->matrice_occupation, i, j);
            }
        }
    }
}

void copier_plan_statique_vers_dynamique(PlanParking* plan)
{
    for (int i = 0; i < TAILLE_PLAN; i++) {
        for (int j = 0; j < TAILLE_PLAN; j++) {
            plan->plan_dynamique[i][j] = plan->plan_statique[i][j];
        }
    }
}

void placer_vehicules_sur_plan(PlanParking* plan, l_car* liste_vehicules)
{
    if (!liste_vehicules || est_vide_liste_car(liste_vehicules)) {
        return;
    }
    
    // D'abord, restaurer le plan statique
    copier_plan_statique_vers_dynamique(plan);
    
    // Puis placer chaque véhicule
    VEHICULE* vehicule_courant = liste_vehicules->premier;
    while (vehicule_courant) {
        if (vehicule_courant->etat == '1') { // Véhicule actif
            int x = vehicule_courant->posx;
            int y = vehicule_courant->posy;
            
            // Placer le véhicule (ici on utilise son type comme caractère)
            if (x >= 0 && x < TAILLE_PLAN && y >= 0 && y < TAILLE_PLAN) {
                plan->plan_dynamique[x][y] = vehicule_courant->type;
            }
        }
        vehicule_courant = vehicule_courant->NXT;
    }
}

int est_position_libre(PlanParking* plan, int x, int y)
{
    if (x < 0 || x >= TAILLE_PLAN || y < 0 || y >= TAILLE_PLAN) {
        return 0;
    }
    return (plan->matrice_occupation->tab[x][y].o == 0);
}

void occuper_place_parking(PlanParking* plan, int x, int y)
{
    if (x >= 0 && x < TAILLE_PLAN && y >= 0 && y < TAILLE_PLAN) {
        if (plan->plan_statique[x][y] == PLACE_LIBRE) {
            plan->plan_statique[x][y] = PLACE_OCCUPEE;
            remplir_case(plan->matrice_occupation, x, y);
            plan->places_libres--;
        }
    }
}

void liberer_place_parking(PlanParking* plan, int x, int y)
{
    if (x >= 0 && x < TAILLE_PLAN && y >= 0 && y < TAILLE_PLAN) {
        if (plan->plan_statique[x][y] == PLACE_OCCUPEE) {
            plan->plan_statique[x][y] = PLACE_LIBRE;
            liberer_case(plan->matrice_occupation, x, y);
            plan->places_libres++;
        }
    }
}

void basculer_barriere_entree(PlanParking* plan)
{
    plan->barriere_entree_ouverte = !plan->barriere_entree_ouverte;
}

void basculer_barriere_sortie(PlanParking* plan)
{
    plan->barriere_sortie_ouverte = !plan->barriere_sortie_ouverte;
}

void afficher_infos_parking(PlanParking* plan)
{
    printf("=== INFOS PARKING ===\n");
    printf("Places libres: %d/%d\n", plan->places_libres, plan->places_totales);
    printf("Barrière entrée: %s\n", plan->barriere_entree_ouverte ? "OUVERTE" : "FERMEE");
    printf("Barrière sortie: %s\n", plan->barriere_sortie_ouverte ? "OUVERTE" : "FERMEE");
    printf("====================\n");
}