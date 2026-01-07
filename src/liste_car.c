#include "liste_car.h"
#include "plan.h"
#include "affichage.h"
#include "mouvement.h"
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

/* Fonction statique pour trouver la position d'entrée des véhicules */
static void trouver_position_entree(PlanParking *plan, int *x, int *y, char *direction)
{
    if (!plan || !x || !y || !direction)
        return;

    /*
     * CORRECTION SIMPLE: Utiliser directement plan->entree_x/y qui pointe
     * maintenant sur une cellule roulable (espace ou flèche) grâce à la
     * correction dans detecter_entree_sortie_wchar()
     */

    // Spawner directement à l'entrée
    *x = plan->entree_x;
    *y = plan->entree_y;
    *direction = 'O';  // Direction Ouest par défaut

    /* Vérifier les limites */
    if (*x < 0 || *x >= plan->largeur || *y < 0 || *y >= plan->hauteur)
    {
        *x = 10;
        *y = 10;
    }
}

VEHICULE *creer_voiture_aleatoire(PlanParking *plan)
{
    if (!plan)
        return NULL;

    const char *fich_modeles[] =
        {
            "car_smallO.txt",
            "car_smallO.txt",
            "car_smallO.txt"};

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

    // Trouver la position d'entrée en cherchant une flèche près de "ENTREE"
    int x, y;
    char direction;
    trouver_position_entree(plan, &x, &y, &direction);

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

    /* Orienter la carrosserie selon la direction initiale */
    orienter_carrosserie(v);

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
            /* CORRECTION SIMPLE: Espacer les voitures pour éviter collision au spawn */
            /* Décaler chaque voiture de 5 cellules en X et 3 cellules en Y */
            v->posx -= (i * 5);
            v->posy += (i * 3);

            /* Vérifier les limites après décalage */
            if (v->posx < 0)
                v->posx = 0;
            if (v->posy < 0)
                v->posy = 0;
            if (v->posx >= plan->largeur)
                v->posx = plan->largeur - 10;
            if (v->posy >= plan->hauteur)
                v->posy = plan->hauteur - 5;

            ajouter_queue_liste_car(v, vehicules);
        }
    }

    return vehicules;
}