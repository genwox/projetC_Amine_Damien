#include "liste_car.h"
#include "plan.h"
#include "affichage.h"
#include "mouvement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
VEHICULE *nv_vehicule(char dir, int x, int y, int v, char al, char type, char **caro, char color, char etat, char t) {
    VEHICULE *vh = malloc(sizeof(struct voiture));
    vh->direction = dir; vh->posx = x; vh->posy = y; vh->vitesse = v;
    vh->alignement = al; vh->type = type; vh->code_couleur = color; vh->etat = etat; vh->tps = t; vh->NXT = NULL;
    for (int i = 0; i < 4; i++) strcpy(vh->Carrosserie[i], caro[i]);
    return vh;
}
l_car *nv_liste_car() {
    l_car *l = malloc(sizeof(struct liste_car));
    l->premier = l->dernier = NULL; l->longeur = 0;
    return l;
}
void detruire_vehicule(VEHICULE **v) { free(*v); *v = NULL; }
int est_vide_liste_car(l_car *lc) { return lc->longeur == 0; }
void ajouter_tete_liste_car(VEHICULE *v, l_car *lc) {
    if (!lc || !v) return;
    v->NXT = lc->premier; lc->premier = v;
    if (!lc->dernier) lc->dernier = v;
    lc->longeur++;
}
void ajouter_queue_liste_car(VEHICULE *v, l_car *lc) {
    if (!lc || !v) return;
    if (est_vide_liste_car(lc)) { ajouter_tete_liste_car(v, lc); return; }
    lc->dernier->NXT = v; lc->dernier = v; lc->longeur++;
}
void detruire_tete_liste_car(l_car *lc) {
    if (!lc) return;
    VEHICULE *tmp = lc->premier;
    lc->premier = lc->premier->NXT;
    detruire_vehicule(&tmp); lc->longeur--;
    if (est_vide_liste_car(lc)) lc->dernier = NULL;
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
void detruire_vehicule_specifique(l_car *lc, VEHICULE *v)
{
    if (!lc || !v || est_vide_liste_car(lc))
        return;
    if (lc->premier == v)
    {
        detruire_tete_liste_car(lc);
        return;
    }
    VEHICULE *prev = lc->premier;
    while (prev != NULL && prev->NXT != v)
        prev = prev->NXT;
    if (prev == NULL || prev->NXT != v)
        return;
    prev->NXT = v->NXT;
    if (lc->dernier == v)
        lc->dernier = prev;
    detruire_vehicule(&v);
    lc->longeur--;
}
void detruire_liste_car(l_car **lc)
{
    while (!est_vide_liste_car(*lc))
        detruire_tete_liste_car(*lc);
    free(*lc);
    *lc = NULL;
}
static void trouver_position_entree(PlanParking *plan, int *x, int *y, char *direction)
{
    if (!plan || !x || !y || !direction)
        return;
    *x = plan->entree_x;
    *y = plan->entree_y;
    *direction = 'O';
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
    int x, y;
    char direction;
    trouver_position_entree(plan, &x, &y, &direction);
    int vitesse = 2;
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
    orienter_carrosserie(v);
    return v;
}
char **charger_modele_voiture(const char *fich)
{
    char **modele = malloc(4 * sizeof(char *));
    if (!modele)
    {
        fprintf(stderr, "Erreur: allocation memoire echec\n");
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
            return NULL;
        }
    }
    // Utiliser helper pour charger depuis fichier
    char temp[4][30];
    FILE *f = fopen(fich, "r");
    if (!f)
    {
        fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", fich);
        for (int i = 0; i < 4; i++) free(modele[i]);
        free(modele);
        return NULL;
    }
    for (int i = 0; i < 4; i++)
    {
        if (fgets(temp[i], 30, f) != NULL)
        {
            size_t len = strlen(temp[i]);
            if (len > 0 && temp[i][len - 1] == '\n')
            {
                temp[i][len - 1] = '\0';
            }
        }
        else
        {
            temp[i][0] = '\0';
        }
        strcpy(modele[i], temp[i]);
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
            v->posx -= (i * 5);
            v->posy += (i * 3);
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
FileAttenteEntree* creer_file_attente(int longueur_max)
{
    if (longueur_max <= 0)
        return NULL;
    FileAttenteEntree *file = malloc(sizeof(FileAttenteEntree));
    if (!file)
        return NULL;
    file->premier_attente = NULL;
    file->dernier_attente = NULL;
    file->longueur_attente = 0;
    file->longueur_max = longueur_max;
    return file;
}
void detruire_file_attente(FileAttenteEntree **file)
{
    if (!file || !*file)
        return;
    VEHICULE *courant = (*file)->premier_attente;
    while (courant != NULL)
    {
        VEHICULE *suivant = courant->NXT;
        detruire_vehicule(&courant);
        courant = suivant;
    }
    free(*file);
    *file = NULL;
}
int file_attente_est_vide(FileAttenteEntree *file)
{
    if (!file)
        return 1;
    return (file->longueur_attente == 0);
}
int file_attente_est_pleine(FileAttenteEntree *file)
{
    if (!file)
        return 1;
    return (file->longueur_attente >= file->longueur_max);
}
int ajouter_a_file_attente(FileAttenteEntree *file, VEHICULE *v, unsigned long frame)
{
    if (!file || !v)
        return 0;
    if (file_attente_est_pleine(file))
        return 0;
    v->etat = '2';
    v->temps_attente = frame;
    v->NXT = NULL;
    if (file_attente_est_vide(file))
    {
        file->premier_attente = v;
        file->dernier_attente = v;
    }
    else
    {
        file->dernier_attente->NXT = v;
        file->dernier_attente = v;
    }
    file->longueur_attente++;
    return 1;
}
VEHICULE* retirer_de_file_attente(FileAttenteEntree *file)
{
    if (!file || file_attente_est_vide(file))
        return NULL;
    VEHICULE *v = file->premier_attente;
    file->premier_attente = v->NXT;
    if (file->premier_attente == NULL)
        file->dernier_attente = NULL;
    v->etat = '1';
    v->temps_attente = 0;
    v->NXT = NULL;
    file->longueur_attente--;
    return v;
}
void supprimer_vehicule_file(FileAttenteEntree *file, VEHICULE *v)
{
    if (!file || !v || file_attente_est_vide(file))
        return;
    if (file->premier_attente == v)
    {
        file->premier_attente = v->NXT;
        if (file->premier_attente == NULL)
            file->dernier_attente = NULL;
        detruire_vehicule(&v);
        file->longueur_attente--;
        return;
    }
    VEHICULE *prev = file->premier_attente;
    while (prev != NULL && prev->NXT != v)
        prev = prev->NXT;
    if (prev == NULL || prev->NXT != v)
        return;
    prev->NXT = v->NXT;
    if (file->dernier_attente == v)
        file->dernier_attente = prev;
    detruire_vehicule(&v);
    file->longueur_attente--;
}
void parcourir_liste(l_car *liste, void (*callback)(VEHICULE*, void*), void *ctx)
{
    if (!liste || !callback) return;
    VEHICULE *v = liste->premier;
    while (v != NULL) {
        callback(v, ctx);
        v = v->NXT;
    }
}
VEHICULE* trouver_vehicule(l_car *liste, int (*test)(VEHICULE*, void*), void *ctx)
{
    if (!liste || !test) return NULL;
    VEHICULE *v = liste->premier;
    while (v != NULL) {
        if (test(v, ctx)) return v;
        v = v->NXT;
    }
    return NULL;
}
int compter_vehicules(l_car *liste, int (*filtre)(VEHICULE*, void*), void *ctx)
{
    if (!liste) return 0;
    int compteur = 0;
    VEHICULE *v = liste->premier;
    while (v != NULL) {
        if (!filtre || filtre(v, ctx)) compteur++;
        v = v->NXT;
    }
    return compteur;
}
