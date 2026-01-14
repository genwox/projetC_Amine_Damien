#include "plan.h"
#include "utils.h"
#include "affichage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void initialiser_plan_parking(PlanParking *plan) {
    plan->places_libres = plan->places_totales = plan->nb_fleches = plan->difficulte = 0;
    plan->barriere_entree_ouverte = plan->barriere_sortie_ouverte = 0;
    plan->entree_x = plan->entree_y = plan->sortie_x = plan->sortie_y = 0;
    plan->borne_entree_x = plan->borne_entree_y = plan->borne_sortie_x = plan->borne_sortie_y = 0;
    for (int i = 0; i < MAX_HAUTEUR; i++)
        for (int j = 0; j < MAX_LARGEUR; j++)
            plan->plan_statique[i][j] = L' ';
}
static void detecter_place_wchar(wchar_t c, PlanParking *plan, int ligne, int colonne) {
    if (c == L'╦') {
        if (plan->places_totales < 50) {
            plan->places[plan->places_totales].ligne = ligne;
            plan->places[plan->places_totales].colonne = colonne;
            plan->places[plan->places_totales].occupee = 0;
        }
        plan->places_libres++; plan->places_totales++;
    }
}
typedef struct { wchar_t caractere; char direction_entree, direction_sortie; } FlecheInfo;
static const FlecheInfo TABLE_FLECHES[] = {
    {L'←', '\0', 'O'},
    {L'→', '\0', 'E'},
    {L'↑', '\0', 'N'},
    {L'↓', '\0', 'S'},
    {L'⮣', 'N', 'E'},
    {L'⮧', 'E', 'S'},
    {L'⮦', 'O', 'S'},
    {L'⮢', 'N', 'O'},
    {L'⮡', 'S', 'E'},
    {L'⮥', 'E', 'N'},
    {L'⮤', 'O', 'N'},
    {L'⮠', 'S', 'O'},
};
#define NB_FLECHES (sizeof(TABLE_FLECHES) / sizeof(FlecheInfo))
static void detecter_fleche_wchar(wchar_t c, PlanParking *plan, int ligne, int colonne) {
    for (size_t i = 0; i < NB_FLECHES; i++) {
        if (TABLE_FLECHES[i].caractere == c) {
            if (plan->nb_fleches < 100) {
                plan->fleches[plan->nb_fleches].ligne = ligne;
                plan->fleches[plan->nb_fleches].colonne = colonne;
                plan->fleches[plan->nb_fleches].direction_entree = TABLE_FLECHES[i].direction_entree;
                plan->fleches[plan->nb_fleches].direction_sortie = TABLE_FLECHES[i].direction_sortie;
                plan->nb_fleches++;
            }
            return;
        }
    }
}
static void detecter_entree_sortie_wchar(const wchar_t *ligne, size_t pos, size_t len,
                                          PlanParking *plan, int ligne_courante)
{
    if (ligne[pos] == L'E')
    {
        int est_entree = (pos + 6 <= len && wcsncmp(&ligne[pos], L"ENTREE", 6) == 0);
        if (!est_entree)
        {
            if (plan->entree_x == 0)
            {
                plan->entree_x = pos;
                plan->entree_y = ligne_courante;
            }
            return;
        }
    }
    if (plan->entree_x == 0 && pos + 6 <= len && wcsncmp(&ligne[pos], L"ENTREE", 6) == 0)
    {
        int offset_x = pos + 7;
        while (offset_x < (int)len)
        {
            wchar_t c = ligne[offset_x];
            if (c == L' ' || c == L'←' || c == L'→' || c == L'↑' || c == L'↓')
            {
                plan->entree_x = offset_x;
                plan->entree_y = ligne_courante;
                break;
            }
            offset_x++;
            if (offset_x > (int)pos + 17)
                break;
        }
        if (plan->entree_x == 0)
        {
            plan->entree_x = pos + 7;
            plan->entree_y = ligne_courante;
        }
    }
    else if (pos + 6 <= len && wcsncmp(&ligne[pos], L"Sortie", 6) == 0)
    {
        int offset_x = pos + 7;
        while (offset_x < (int)len)
        {
            wchar_t c = ligne[offset_x];
            if (c == L' ' || c == L'←' || c == L'→' || c == L'↑' || c == L'↓')
            {
                plan->sortie_x = offset_x;
                plan->sortie_y = ligne_courante;
                break;
            }
            offset_x++;
            if (offset_x > (int)pos + 17)
                break;
        }
        if (plan->sortie_x == 0)
        {
            plan->sortie_x = pos + 7;
            plan->sortie_y = ligne_courante;
        }
    }
    else if (ligne[pos] == L'S')
    {
        int est_isole = 1;
        if (pos > 0)
        {
            wchar_t avant = ligne[pos - 1];
            if (!(avant == L' ' || avant == L'←' || avant == L'→' || avant == L'↑' || avant == L'↓'))
                est_isole = 0;
        }
        if (pos + 1 < len && est_isole)
        {
            wchar_t apres = ligne[pos + 1];
            if (!(apres == L' ' || apres == L'←' || apres == L'→' || apres == L'↑' || apres == L'↓' || apres == L'\0'))
                est_isole = 0;
        }
        if (est_isole)
        {
            plan->sortie_x = pos;
            plan->sortie_y = ligne_courante;
        }
    }
    else if (ligne[pos] == L'[' && pos + 1 < len && ligne[pos + 1] == L'T')
    {
        plan->borne_entree_x = pos;
        plan->borne_entree_y = ligne_courante;
    }
    else if (ligne[pos] == L'[' && pos + 1 < len && ligne[pos + 1] == L'P')
    {
        plan->borne_sortie_x = pos;
        plan->borne_sortie_y = ligne_courante;
    }
}
static void traiter_ligne_plan(const char *ligne, PlanParking *plan,
                                int ligne_courante, int *largeur_max)
{
    wchar_t wligne[MAX_LARGEUR];
    size_t len = mbstowcs(wligne, ligne, MAX_LARGEUR);
    if (len == (size_t)-1)
    {
        return;
    }
    if (len > 0 && wligne[len-1] == L'\n')
    {
        wligne[len-1] = L'\0';
        len--;
    }
    if ((int)len > *largeur_max)
    {
        *largeur_max = (int)len;
    }
    for (size_t i = 0; i < len && i < MAX_LARGEUR; i++)
    {
        wchar_t c = wligne[i];
        plan->plan_statique[ligne_courante][i] = c;
        detecter_place_wchar(c, plan, ligne_courante, i);
        detecter_fleche_wchar(c, plan, ligne_courante, i);
        detecter_entree_sortie_wchar(wligne, i, len, plan, ligne_courante);
    }
}
PlanParking *charger_plan(const char *fichier_plan)
{
    FILE *fichier = fopen(fichier_plan, "r");
    if (!fichier)
    {
        printf("Erreur : impossible d'ouvrir le fichier %s\n", fichier_plan);
        return NULL;
    }
    PlanParking *plan = malloc(sizeof(PlanParking));
    if (!plan)
    {
        fclose(fichier);
        return NULL;
    }
    initialiser_plan_parking(plan);
    char ligne[MAX_LIGNE];
    int ligne_courante = 0;
    int largeur_max = 0;
    while (fgets(ligne, MAX_LIGNE, fichier) && ligne_courante < MAX_HAUTEUR)
    {
        nettoyer_fin_ligne(ligne);
        traiter_ligne_plan(ligne, plan, ligne_courante, &largeur_max);
        ligne_courante++;
    }
    fclose(fichier);
    plan->hauteur = ligne_courante;
    plan->largeur = largeur_max;
    plan->matrice_occupation = creer_matrice(plan->hauteur, plan->largeur);
    if (!plan->matrice_occupation)
    {
        free(plan);
        return NULL;
    }
    initialiser_matrice_depuis_plan(plan);
    plan->argent_total = 0;
    plan->score = 0;
    plan->vehicules_servis = 0;
    plan->vehicules_perdus = 0;
    plan->high_score = 0;
    return plan;
}
void detruire_plan(PlanParking **plan)
{
    if (*plan)
    {
        if ((*plan)->matrice_occupation)
        {
            detruire_matrice(&((*plan)->matrice_occupation));
        }
        free(*plan);
        *plan = NULL;
    }
}
void initialiser_matrice_depuis_plan(PlanParking *plan)
{
    if (!plan || !plan->matrice_occupation)
        return;
    for (int i = 0; i < plan->hauteur; i++)
    {
        for (int j = 0; j < plan->largeur; j++)
        {
            wchar_t c = plan->plan_statique[i][j];
            /* Cases roulables : espaces, flèches, entrée/sortie */
            int est_roulable = (c == L' ' ||
                                c == L'←' || c == L'→' || c == L'↑' || c == L'↓' ||
                                c == L'.' ||
                                c == L'E' || c == L'S' || c == L'e' || c == L's');
            if (est_roulable)
            {
                liberer_case(plan->matrice_occupation, i, j);
            }
            else
            {
                remplir_case(plan->matrice_occupation, i, j);
            }
        }
    }
}
void placer_vehicules_sur_plan(PlanParking *plan, l_car *liste_vehicules)
{
    (void)plan;
    (void)liste_vehicules;
}
int trouver_place_a_position(PlanParking *plan, int ligne, int colonne)
{
    if (!plan)
        return -1;
    for (int i = 0; i < plan->places_totales; i++)
    {
        if (plan->places[i].colonne == colonne &&
            ligne >= plan->places[i].ligne &&
            ligne <= plan->places[i].ligne + 2)
        {
            return i;
        }
    }
    return -1;
}
void marquer_place_occupee(PlanParking *plan, int index_place)
{
    if (!plan || index_place < 0 || index_place >= plan->places_totales)
        return;
    if (!plan->places[index_place].occupee)
    {
        plan->places[index_place].occupee = 1;
        plan->places_libres--;
    }
}
void marquer_place_libre(PlanParking *plan, int index_place)
{
    if (!plan || index_place < 0 || index_place >= plan->places_totales)
        return;
    if (plan->places[index_place].occupee)
    {
        plan->places[index_place].occupee = 0;
        plan->places_libres++;
    }
}
void basculer_barriere_entree(PlanParking *plan)
{
    if (!plan)
        return;
    plan->barriere_entree_ouverte = !plan->barriere_entree_ouverte;
}
void basculer_barriere_sortie(PlanParking *plan)
{
    if (!plan)
        return;
    plan->barriere_sortie_ouverte = !plan->barriere_sortie_ouverte;
}
