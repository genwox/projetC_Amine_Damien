#include "affichage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void initialiser_affichage()
{
    printf(CLEAR_SCREEN);
    printf(HIDE_CURSOR);
    printf(CURSOR_HOME);
}

void terminer_affichage()
{
    printf(SHOW_CURSOR);
    printf(RESET_COLOR);
    printf(CLEAR_SCREEN);
}

void effacer_ecran()
{
    printf(CLEAR_SCREEN);
    printf(CURSOR_HOME);
}

void afficher_plan_complet(PlanParking* plan)
{
    GOTO_XY(0, 0);
    
    for (int i = 0; i < TAILLE_PLAN; i++) {
        for (int j = 0; j < TAILLE_PLAN; j++) {
            char c = plan->plan_dynamique[i][j];
            printf("%s%c", obtenir_couleur_caractere(c), c);
        }
        printf(RESET_COLOR "\n");
    }
}

void afficher_plan_optimise(PlanParking* plan, l_car* vehicules, GestionAffichage* gestion)
{
    // Mettre à jour le plan avec les véhicules
    placer_vehicules_sur_plan(plan, vehicules);
    
    // Afficher seulement les cases modifiées
    for (int i = 0; i < TAILLE_PLAN; i++) {
        for (int j = 0; j < TAILLE_PLAN; j++) {
            if (gestion->derniere_mise_a_jour[i][j] != gestion->frame_courante) {
                GOTO_XY(i, j);
                afficher_caractere_colore(plan->plan_dynamique[i][j], i, j);
                gestion->derniere_mise_a_jour[i][j] = gestion->frame_courante;
            }
        }
    }
    
    gestion->frame_courante++;
}

void afficher_caractere_colore(char c, int x, int y)
{
    GOTO_XY(x, y);
    printf("%s%c%s", obtenir_couleur_caractere(c), c, RESET_COLOR);
}

char* obtenir_couleur_caractere(char c)
{
    switch(c) {
        case MUR:               return GRIS;
        case ROUTE:             return BLANC;
        case PLACE_LIBRE:       return BG_VERT BLANC;
        case PLACE_OCCUPEE:     return BG_ROUGE BLANC;
        case ENTREE:            return BG_BLEU BLANC;
        case SORTIE:            return BG_BLEU BLANC;
        case BORNE_ENTREE:      return JAUNE;
        case BORNE_SORTIE:      return JAUNE;
        case BARRIERE_FERMEE:   return ROUGE;
        case BARRIERE_OUVERTE:  return VERT;
        case 'v':               return CYAN;     // Voiture
        case 'c':               return MAGENTA;  // Camion
        default:                return BLANC;
    }
}

void afficher_menu_principal()
{
    effacer_ecran();
    afficher_titre_jeu();
    
    GOTO_XY(15, 15);
    printf("=== SIMULATEUR DE PARKING ===");
    GOTO_XY(17, 20);
    printf("1. Nouvelle partie");
    GOTO_XY(18, 20);
    printf("2. Charger partie");
    GOTO_XY(19, 20);
    printf("3. Options");
    GOTO_XY(20, 20);
    printf("4. Quitter");
    GOTO_XY(22, 15);
    printf("Votre choix: ");
}

void afficher_menu_modes()
{
    effacer_ecran();
    GOTO_XY(10, 15);
    printf("=== SELECTION DU MODE ===");
    GOTO_XY(12, 18);
    printf("1. Mode FLUIDE");
    GOTO_XY(13, 20);
    printf("   - Moins de véhicules");
    GOTO_XY(14, 20);
    printf("   - Circulation normale");
    GOTO_XY(16, 18);
    printf("2. Mode CHARGE");
    GOTO_XY(17, 20);
    printf("   - Plus de véhicules");
    GOTO_XY(18, 20);
    printf("   - Risque de bouchons");
    GOTO_XY(20, 18);
    printf("3. Mode CONDUITE");
    GOTO_XY(21, 20);
    printf("   - Vous conduisez!");
    GOTO_XY(23, 15);
    printf("Votre choix: ");
}

void afficher_hud_jeu(PlanParking* plan, int temps_ecoule)
{
    // Affichage des informations en bas de l'écran
    GOTO_XY(TAILLE_PLAN + 2, 0);
    printf("=== PARKING SIMULATOR ===");
    GOTO_XY(TAILLE_PLAN + 3, 0);
    printf("Places: %s%d%s/%d", VERT, plan->places_libres, RESET_COLOR, plan->places_totales);
    GOTO_XY(TAILLE_PLAN + 4, 0);
    printf("Temps: %02d:%02d", temps_ecoule / 60, temps_ecoule % 60);
    GOTO_XY(TAILLE_PLAN + 5, 0);
    printf("Entree: %s%s%s", plan->barriere_entree_ouverte ? VERT : ROUGE,
           plan->barriere_entree_ouverte ? "OUVERTE" : "FERMEE", RESET_COLOR);
    GOTO_XY(TAILLE_PLAN + 6, 0);
    printf("Sortie: %s%s%s", plan->barriere_sortie_ouverte ? VERT : ROUGE,
           plan->barriere_sortie_ouverte ? "OUVERTE" : "FERMEE", RESET_COLOR);
    GOTO_XY(TAILLE_PLAN + 8, 0);
    printf("Commandes: [Q]uitter [P]ause [R]eset");
}

void afficher_vehicule(VEHICULE* vehicule)
{
    if (!vehicule || vehicule->etat != '1') return;
    
    // Afficher la carrosserie du véhicule
    for (int i = 0; i < 4; i++) {
        GOTO_XY(vehicule->posx + i, vehicule->posy);
        printf("%s%s%s", obtenir_couleur_caractere(vehicule->type), 
               vehicule->Carrosserie[i], RESET_COLOR);
    }
}

void afficher_titre_jeu()
{
    GOTO_XY(2, 10);
    printf("%s", CYAN);
    printf("  ____  _    ____  _  _  ____  _      ____");
    GOTO_XY(3, 10);
    printf(" (  _ \\( \\/)(  _ \\( \\/ )(  _ \\( )    / ___)");
    GOTO_XY(4, 10);
    printf("  )___/ )  (  )   / \\  / )___/ )(   ( (__");
    GOTO_XY(5, 10);
    printf(" (__)  (_/\\_)(___)(__)  (__)  (__)   \\___)");
    GOTO_XY(6, 10);
    printf("                 PARKING                 ");
    printf("%s", RESET_COLOR);
}

GestionAffichage* creer_gestion_affichage()
{
    GestionAffichage* gestion = malloc(sizeof(GestionAffichage));
    if (!gestion) return NULL;
    
    gestion->frame_courante = 1;
    gestion->affichage_optimise = 1;
    
    // Initialiser toutes les cases à 0
    for (int i = 0; i < TAILLE_PLAN; i++) {
        for (int j = 0; j < TAILLE_PLAN; j++) {
            gestion->derniere_mise_a_jour[i][j] = 0;
        }
    }
    
    return gestion;
}

void detruire_gestion_affichage(GestionAffichage** gestion)
{
    if (*gestion) {
        free(*gestion);
        *gestion = NULL;
    }
}