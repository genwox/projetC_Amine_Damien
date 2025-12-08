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
    // 1. Initialisation
    setlocale(LC_ALL, "");
    initialiser_affichage();

    // Vérifier que le terminal est assez grand
    if (!verifier_taille_terminal())
    {
        terminer_affichage();
        return 1;
    }

    // 2. Écran de démarrage
    PlanParking *plan = afficher_ecran_demarrage();
    if (!plan)
    {
        terminer_affichage();
        return 1;
    }

    // 3. Initialisation du jeu
    srand(time(NULL));
    l_car *vehicules = initialiser_vehicules(plan, 3);
    if (!vehicules)
    {
        detruire_plan(&plan);
        terminer_affichage();
        return 1;
    }

    // 4. Boucle de jeu
    executer_boucle_jeu(plan, vehicules);

    // 5. Nettoyage
    detruire_liste_car(&vehicules);
    detruire_plan(&plan);
    terminer_affichage();

    printf("\nMerci d'avoir utilise le simulateur de parking !\n");

    return 0;
}
