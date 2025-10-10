#include "liste_car.h"
#include "matrice.h"
#include "plan.h"
#include <stdlib.h>
#include <stdio.h>

int main()
{
    printf("\n");
    printf("%s===============================================%s\n", CYAN, RESET_COLOR);
    printf("%s     SIMULATEUR DE PARKING - VUE AERIENNE    %s\n", JAUNE, RESET_COLOR);
    printf("%s===============================================%s\n", CYAN, RESET_COLOR);
    printf("\n");
    
    // Test liste chaînée
    printf("%s[OK]%s Test liste chainee... ", VERT, RESET_COLOR);
    l_car *l = nv_liste_car();
    if (l) {
        printf("%sOK%s\n", VERT, RESET_COLOR);
        detruire_liste_car(&l);
    } else {
        printf("%sERREUR%s\n", ROUGE, RESET_COLOR);
        return 1;
    }
    
    // Chargement du plan
    printf("%s[OK]%s Chargement du plan... ", VERT, RESET_COLOR);
    PlanParking *plan = charger_plan("plan.txt");
    if (plan) {
        printf("%sOK%s\n", VERT, RESET_COLOR);
        printf("%s[i]%s Dimensions: %dx%d\n", CYAN, RESET_COLOR, plan->hauteur, plan->largeur);
        printf("%s[i]%s Places detectees: %s%d%s\n", CYAN, RESET_COLOR, VERT, plan->places_totales, RESET_COLOR);
        
        printf("\n%sAppuyez sur Entree pour voir le parking...%s", JAUNE, RESET_COLOR);
        getchar();
        
        // Afficher le plan
        afficher_plan_couleur(plan);
        
        printf("\n%sLEGENDE:%s\n", CYAN, RESET_COLOR);
        printf("  %s|P|%s  Place libre (verte)\n", BG_VERT, RESET_COLOR);
        printf("  %s|X|%s  Place occupee (rouge)\n", BG_ROUGE, RESET_COLOR);
        printf("  %s> < ^ v%s  Sens de circulation\n", CYAN, RESET_COLOR);
        printf("  %s[T]%s  Borne ticket (entree)\n", JAUNE, RESET_COLOR);
        printf("  %s[P]%s  Borne paiement (sortie)\n", JAUNE, RESET_COLOR);
        
        printf("\n%s[OK] Tests reussis !%s\n\n", VERT, RESET_COLOR);
        
        detruire_plan(&plan);
    } else {
        printf("%sERREUR%s\n", ROUGE, RESET_COLOR);
        return 1;
    }
    
    return 0;
}