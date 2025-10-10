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
    plan->places_libres = 0;
    plan->places_totales = 0;
    plan->barriere_entree_ouverte = 0;
    plan->barriere_sortie_ouverte = 0;
    plan->entree_x = 0;
    plan->entree_y = 0;
    plan->sortie_x = 0;
    plan->sortie_y = 0;
    plan->borne_entree_x = 0;
    plan->borne_entree_y = 0;
    plan->borne_sortie_x = 0;
    plan->borne_sortie_y = 0;
    
    // Initialiser toute la grille
    for (int i = 0; i < MAX_HAUTEUR; i++) {
        for (int j = 0; j < MAX_LARGEUR; j++) {
            plan->plan_statique[i][j] = ' ';
            plan->plan_dynamique[i][j] = ' ';
        }
    }
    
    // Lecture du fichier
    char ligne[MAX_LIGNE];
    int ligne_courante = 0;
    int largeur_max = 0;
    
    while (fgets(ligne, MAX_LIGNE, fichier) && ligne_courante < MAX_HAUTEUR) {
        int len = (int)strlen(ligne);
        
        if (len > 0 && ligne[len-1] == '\n') {
            ligne[len-1] = '\0';
            len--;
        }
        
        if (len > largeur_max) {
            largeur_max = len;
        }
        
        for (int j = 0; j < len && j < MAX_LARGEUR; j++) {
            char c = ligne[j];
            plan->plan_statique[ligne_courante][j] = c;
            
            // Détecter les places |P|
            if (c == '|' && j+2 < len && ligne[j+1] == 'P' && ligne[j+2] == '|') {
                plan->places_libres++;
                plan->places_totales++;
            }
            
            // Détecter entrée/sortie
            if (c == 'E' && strstr(ligne, "ENTREE")) {
                plan->entree_x = ligne_courante;
                plan->entree_y = j;
            }
            else if (c == '[' && j+1 < len && ligne[j+1] == 'T') {
                plan->borne_entree_x = ligne_courante;
                plan->borne_entree_y = j;
            }
            else if (c == '[' && j+1 < len && ligne[j+1] == 'P') {
                plan->borne_sortie_x = ligne_courante;
                plan->borne_sortie_y = j;
            }
        }
        ligne_courante++;
    }
    
    fclose(fichier);
    
    plan->hauteur = ligne_courante;
    plan->largeur = largeur_max;
    
    // Créer matrice d'occupation
    plan->matrice_occupation = creer_matrice(plan->hauteur, plan->largeur);
    if (!plan->matrice_occupation) {
        free(plan);
        return NULL;
    }
    
    initialiser_matrice_depuis_plan(plan);
    copier_plan_statique_vers_dynamique(plan);
    
    return plan;
}

void detruire_plan(PlanParking** plan)
{
    if (*plan) {
        if ((*plan)->matrice_occupation) {
            detruire_matrice(&((*plan)->matrice_occupation));
        }
        free(*plan);
        *plan = NULL;
    }
}

void initialiser_matrice_depuis_plan(PlanParking* plan)
{
    if (!plan || !plan->matrice_occupation) return;
    
    for (int i = 0; i < plan->hauteur; i++) {
        for (int j = 0; j < plan->largeur; j++) {
            char c = plan->plan_statique[i][j];
            if (c == '#' || c == '|' || c == '_' || c == PLACE_OCCUPEE) {
                remplir_case(plan->matrice_occupation, i, j);
            } else {
                liberer_case(plan->matrice_occupation, i, j);
            }
        }
    }
}

void copier_plan_statique_vers_dynamique(PlanParking* plan)
{
    if (!plan) return;
    
    for (int i = 0; i < plan->hauteur; i++) {
        for (int j = 0; j < plan->largeur; j++) {
            plan->plan_dynamique[i][j] = plan->plan_statique[i][j];
        }
    }
}

void placer_vehicules_sur_plan(PlanParking* plan, l_car* liste_vehicules)
{
    if (!plan || !liste_vehicules || est_vide_liste_car(liste_vehicules)) {
        return;
    }
    
    copier_plan_statique_vers_dynamique(plan);
    
    VEHICULE* vehicule_courant = liste_vehicules->premier;
    while (vehicule_courant) {
        if (vehicule_courant->etat == '1') {
            int x = vehicule_courant->posx;
            int y = vehicule_courant->posy;
            
            if (x >= 0 && x < plan->hauteur && y >= 0 && y < plan->largeur) {
                plan->plan_dynamique[x][y] = vehicule_courant->type;
            }
        }
        vehicule_courant = vehicule_courant->NXT;
    }
}

int est_position_libre(PlanParking* plan, int x, int y)
{
    if (!plan || x < 0 || x >= plan->hauteur || y < 0 || y >= plan->largeur) {
        return 0;
    }
    return (plan->matrice_occupation->tab[x][y].o == 0);
}

void occuper_place_parking(PlanParking* plan, int x, int y)
{
    if (!plan || x < 0 || x >= plan->hauteur || y < 0 || y >= plan->largeur) {
        return;
    }
    
    if (plan->plan_statique[x][y] == PLACE_LIBRE) {
        plan->plan_statique[x][y] = PLACE_OCCUPEE;
        remplir_case(plan->matrice_occupation, x, y);
        plan->places_libres--;
    }
}

void liberer_place_parking(PlanParking* plan, int x, int y)
{
    if (!plan || x < 0 || x >= plan->hauteur || y < 0 || y >= plan->largeur) {
        return;
    }
    
    if (plan->plan_statique[x][y] == PLACE_OCCUPEE) {
        plan->plan_statique[x][y] = PLACE_LIBRE;
        liberer_case(plan->matrice_occupation, x, y);
        plan->places_libres++;
    }
}

void basculer_barriere_entree(PlanParking* plan)
{
    if (!plan) return;
    plan->barriere_entree_ouverte = !plan->barriere_entree_ouverte;
}

void basculer_barriere_sortie(PlanParking* plan)
{
    if (!plan) return;
    plan->barriere_sortie_ouverte = !plan->barriere_sortie_ouverte;
}

void afficher_infos_parking(PlanParking* plan)
{
    if (!plan) return;
    
    printf("\n");
    printf("%s===== PARKING INFO =====%s\n", CYAN, RESET_COLOR);
    printf("Places: %s%d%s/%d\n", VERT, plan->places_libres, RESET_COLOR, plan->places_totales);
    printf("Entree: %s%s%s\n", plan->barriere_entree_ouverte ? VERT : ROUGE, 
           plan->barriere_entree_ouverte ? "OUVERTE" : "FERMEE", RESET_COLOR);
    printf("Sortie: %s%s%s\n", plan->barriere_sortie_ouverte ? VERT : ROUGE, 
           plan->barriere_sortie_ouverte ? "OUVERTE" : "FERMEE", RESET_COLOR);
    printf("%s========================%s\n", CYAN, RESET_COLOR);
}

char* obtenir_couleur_caractere(char c, int est_place_libre)
{
    switch(c) {
        case '#':
        case '=':
            return GRIS;
        case 'P':
            return est_place_libre ? BG_VERT : BG_ROUGE;
        case '|':
        case '_':
            return est_place_libre ? VERT : ROUGE;
        case '>':
        case '<':
        case '^':
        case 'v':
            return CYAN;
        case 'E':
            return BG_JAUNE;
        case 'T':
        case '[':
        case ']':
            return JAUNE;
        case 'A':
        case 'L':
            return CYAN;
        default:
            return RESET_COLOR;
    }
}

void afficher_plan_couleur(PlanParking* plan)
{
    if (!plan) return;
    
    system("clear");
    
    printf("\n%s", BG_CYAN);
    printf("                    SIMULATEUR DE PARKING                    ");
    printf("%s\n\n", RESET_COLOR);
    
    for (int i = 0; i < plan->hauteur; i++) {
        for (int j = 0; j < plan->largeur; j++) {
            char c = plan->plan_statique[i][j];
            
            // Places |P| en vert
            int est_place_libre = (c == 'P' && j > 0 && plan->plan_statique[i][j-1] == '|');
            
            printf("%s%c%s", obtenir_couleur_caractere(c, est_place_libre), c, RESET_COLOR);
        }
        printf("\n");
    }
    
    afficher_infos_parking(plan);
}