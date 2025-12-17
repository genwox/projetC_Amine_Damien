#include "liste_car.h"
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