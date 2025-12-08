#include "liste_car.h"
#include "plan.h"
#include "affichage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VEHICULE *nv_vehicule(char dir, int x, int y, int v, char al, char type, char **caro, char color, char etat, char t)
{
    VEHICULE *vh = malloc(sizeof(struct voiture));
    vh->direction = dir;
    vh->posx = x;
    vh->posy = y;
    vh->vitesse = v;
    vh->alignement = al;
    vh->type = type;

    for (int i = 0; i < 4; i++)
        strcpy(vh->Carrosserie[i], caro[i]);

    vh->code_couleur = color;
    vh->etat = etat;
    vh->tps = t;
    vh->NXT = NULL;

    return vh;
}

l_car *nv_liste_car()
{
    l_car *l = malloc(sizeof(struct liste_car));
    l->premier = NULL;
    l->dernier = NULL;
    l->longeur = 0;
    return l;
}

void detruire_vehicule(VEHICULE **v)
{

    free(*v);
    *v = NULL;
}

int est_vide_liste_car(l_car *lc)
{
    return (lc->longeur == 0 ? 1 : 0);
}

void ajouter_tete_liste_car(VEHICULE *v, l_car *lc)
{
    if (lc == NULL || v == NULL)
        return;

    if (!est_vide_liste_car(lc))
    {
        v->NXT = lc->premier;
        lc->premier = v;
        lc->longeur++;
    }
    else
    {
        lc->premier = v;
        lc->dernier = lc->premier;
        lc->longeur++;
    }
}

void ajouter_queue_liste_car(VEHICULE *v, l_car *lc)
{
    if (lc == NULL || v == NULL)
        return;

    if (est_vide_liste_car(lc))
    {
        ajouter_tete_liste_car(v, lc);
    }
    else
    {
        lc->dernier->NXT = v;
        lc->dernier = v;
        lc->longeur++;
    }
}

void detruire_tete_liste_car(l_car *lc)
{
    if (lc == NULL)
        return;

    VEHICULE *tmp = lc->premier;
    lc->premier = lc->premier->NXT;
    detruire_vehicule(&tmp);
    lc->longeur--;

    if (est_vide_liste_car(lc))
        lc->dernier = NULL;
}

void detruire_queue_liste_car(l_car *lc)
{

    if (lc->premier->NXT == NULL)
    {
        detruire_vehicule(&lc->premier);
        lc->longeur--;
        return;
    }

    VEHICULE *cour = lc->premier;
    while (cour->NXT == NULL && cour->NXT->NXT != NULL)
        cour = cour->NXT;

    VEHICULE *tmp = cour->NXT;
    lc->dernier = cour;
    cour->NXT = NULL;
    detruire_vehicule(&tmp);
    lc->longeur--;
}

void detruire_liste_car(l_car **lc)
{
    while (!est_vide_liste_car(*lc))
        detruire_tete_liste_car(*lc);

    free(*lc);
    *lc = NULL;
}

VEHICULE *creer_voiture_aleatoire(PlanParking *plan)
{
    if (!plan)
        return NULL;

    const char *fich_modeles[] =
        {
            "car_small.txt",
            "camion_small.txt",
            "suv_small.txt"};

    char types[] = {'v', 'c', 's'};

    int type_index = rand() % 3;
    char type = types[type_index];

    char **carrosserie = charger_modele_voiture(fich_modeles[type_index]);
    if (!carrosserie)
    {
        fprintf(stderr, "Erreur : impossible de charger le modèle\n");
        return NULL;
    }

    int couleurs[] =
        {
            COLOR_PAIR_ROUGE,
            COLOR_PAIR_VERT,
            COLOR_PAIR_JAUNE,
            COLOR_PAIR_BLEU,
            COLOR_PAIR_CYAN};

    int couleur = couleurs[rand() % 5];

    // Position à l'entrée
    // Convention: entree_x=colonne, entree_y=ligne (cohérent avec posx=colonne, posy=ligne)
    // Le mot "ENTREE" est détecté, mais le trou est 2 lignes plus bas
    int x = plan->entree_x;      // colonne du mot ENTREE
    int y = plan->entree_y + 1;  // ligne + 1 pour être juste devant le trou

    // Direction selon la position de l'entrée
    char direction = 'N';  // Direction Nord (vers le haut) pour entrer dans le parking

    int vitesse = 1;

    VEHICULE *v = nv_vehicule(
        direction,
        x,
        y,
        vitesse,
        'g',
        type,
        carrosserie,
        couleur,
        '1',
        0);

    for (int i = 0; i < 4; i++)
    {
        free(carrosserie[i]);
    }
    free(carrosserie);
    return v;
}

char **charger_modele_voiture(const char *fich)
{
    FILE *f = fopen(fich, "r");
    if (!f)
    {
        fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", fich);
        return NULL;
    }

    char **modele = malloc(4 * sizeof(char *));
    if (!modele)
    {
        fclose(f);
        return NULL;
    }

    for (int i = 0; i < 4; i++)
    {
        modele[i] = malloc(30 * sizeof(char));
        if (!modele[i])
        {
            for (int j = 0; j < i; j++)
            {
                free(modele[j]);
            }
            free(modele);
            fclose(f);
            return NULL;
        }
        // Lire la ligne ou mettre une ligne vide
        if (fgets(modele[i], 30, f))
        {
            size_t len = strlen(modele[i]);
            if (len > 0 && modele[i][len - 1] == '\n')
            {
                modele[i][len - 1] = '\0';
            }
        }
        else
        {
            modele[i][0] = '\0';
        }
    }

    fclose(f);
    return modele;
}

l_car *initialiser_vehicules(PlanParking *plan, int nombre)
{
    if (!plan || nombre <= 0)
        return NULL;

    l_car *vehicules = nv_liste_car();
    if (!vehicules)
        return NULL;

    for (int i = 0; i < nombre; i++)
    {
        VEHICULE *v = creer_voiture_aleatoire(plan);
        if (v)
        {
            ajouter_queue_liste_car(v, vehicules);
        }
    }

    return vehicules;
}