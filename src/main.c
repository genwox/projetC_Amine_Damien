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
    // TEMPORAIRE: désactivé pour debug
    // if (!verifier_taille_terminal())
    // {
    //     terminer_affichage();
    //     return 1;
    // }

    // 2. Sélection de la difficulté
    int difficulte = afficher_menu_difficulte();

    // 3. Écran de démarrage et chargement du plan
    PlanParking *plan = afficher_ecran_demarrage();
    if (!plan)
    {
        terminer_affichage();
        return 1;
    }

    // Initialiser la difficulté du plan
    plan->difficulte = difficulte;

    // 4. Initialisation du jeu
    srand(time(NULL));
    // PHASE A: Créer liste vide, les voitures seront spawnées progressivement
    l_car *vehicules = nv_liste_car();
    if (!vehicules)
    {
        detruire_plan(&plan);
        terminer_affichage();
        return 1;
    }

    // Créer la file d'attente entrée (capacité 10)
    FileAttenteEntree *file_attente = creer_file_attente(10);
    if (!file_attente)
    {
        detruire_liste_car(&vehicules);
        detruire_plan(&plan);
        terminer_affichage();
        return 1;
    }

    // 5. Boucle de jeu
    executer_boucle_jeu(plan, vehicules, file_attente);

    // 6. Nettoyage
    detruire_file_attente(&file_attente);
    detruire_liste_car(&vehicules);
    detruire_plan(&plan);
    terminer_affichage();

    printf("\nMerci d'avoir utilise le simulateur de parking !\n");

    return 0;
}
