#include "liste_car.h"
#include "matrice.h"
#include "plan.h"
#include "affichage.h"
#include "jeu.h"
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <time.h>

int main()
{
    setlocale(LC_ALL, "");
    initialiser_affichage();

    int difficulte = afficher_menu_difficulte();

    PlanParking *plan = afficher_ecran_demarrage();
    if (!plan)
    {
        terminer_affichage();
        return 1;
    }

    plan->difficulte = difficulte;

    srand(time(NULL));
    l_car *vehicules = nv_liste_car();
    if (!vehicules)
    {
        detruire_plan(&plan);
        terminer_affichage();
        return 1;
    }

    FileAttenteEntree *file_attente = creer_file_attente(10);
    if (!file_attente)
    {
        detruire_liste_car(&vehicules);
        detruire_plan(&plan);
        terminer_affichage();
        return 1;
    }

    executer_boucle_jeu(plan, vehicules, file_attente);

    detruire_file_attente(&file_attente);
    detruire_liste_car(&vehicules);
    detruire_plan(&plan);
    terminer_affichage();

    printf("\nMerci d'avoir utilise le simulateur de parking !\n");

    return 0;
}
