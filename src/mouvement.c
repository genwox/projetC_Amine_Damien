#include "mouvement.h"
#include <string.h>

int calculer_largeur_visuelle(const char *str)
{
    if (!str)
        return 0;

    int largeur = 0;
    const char *p = str;

    while (*p)
    {
        if ((*p & 0x80) == 0)
        {
            /* ASCII (1 byte)*/
            largeur++;
            p++;
        }
        else if ((*p & 0xE0) == 0xC0)
        {
            largeur++;
            p += 2;
        }
        else if ((*p & 0xF0) == 0xE0)
        {
            largeur++;
            p += 3;
        }
        else
        {
            p++;
        }
    }

    return largeur;
}

void obtenir_dimensions_vehicule(VEHICULE *vehicule, int *largeur, int *hauteur)
{
    if (!vehicule)
    {
        *largeur = 0;
        *hauteur = 0;
        return;
    }

    int max_largeur = 0;
    int lignes_utilises = 0;

    /*Parcourir les 4 lignes de la carrosserie*/
    for (int i = 0; i < 4; i++)
    {
        if (strlen(vehicule->Carrosserie[i]) > 0)
        {
            int l = calculer_largeur_visuelle(vehicule->Carrosserie[i]);
            if (l > max_largeur)
            {
                max_largeur = l;
            }
            lignes_utilises = i + 1;
        }
    }
    *largeur = max_largeur;
    *hauteur = lignes_utilises;
}

int peut_deplacer(VEHICULE *vehicule, PlanParking *plan, int nouveau_x, int nouveau_y)
{
    if (!vehicule || !plan || vehicule->etat != '1')
        return 0;

    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);

    /*Vérifier les limites du plan*/
    if (nouveau_x < 0 || nouveau_y < 0)
        return 0;

    if (nouveau_x + largeur >= plan->largeur || nouveau_y + hauteur >= plan->hauteur)
        return 0;

    for (int dy = 0; dy < hauteur; dy++)
    {
        for (int dx = 0; dx < largeur; dx++)
        {
            int check_x = nouveau_x + dx;
            int check_y = nouveau_y + dy;

            char case_plan = plan->plan_statique[check_y][check_x];

            /*Bloquer uniquement les bordures/murs UTF-8 et certains caractères*/
            unsigned char c = (unsigned char)case_plan;

            /* Bloquer les caractères UTF-8 (bytes > 127) qui sont les bordures ╔║═╗ etc. */
            if (c > 127)
            {
                return 0;
            }

            /* Bloquer les caractères ASCII qui sont des obstacles */
            if (case_plan == '|' || case_plan == '_' || case_plan == '-' ||
                case_plan == '[' || case_plan == ']' || case_plan == MUR)
            {
                return 0;
            }

            /* Tout le reste est autorisé (espaces, flèches, lettres, etc.) */
        }
    }

    return 1;
}

void suivre_fleches(VEHICULE *vehicule, PlanParking *plan)
{
    if (!vehicule || !plan || vehicule->etat != '1')
        return;

    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);

    /* Vérifier uniquement le CENTRE du véhicule pour éviter les fausses détections */
    int centre_x = vehicule->posx + largeur / 2;
    int centre_y = vehicule->posy + hauteur / 2;

    if (centre_x < 0 || centre_y < 0 || centre_x >= plan->largeur || centre_y >= plan->hauteur)
        return;

    char c = plan->plan_statique[centre_y][centre_x];

    /* Changer la direction selon la flèche au centre */
    switch (c)
    {
    case '^':
        vehicule->direction = 'N'; /* Nord - vers le haut */
        break;
    case 'v':
        vehicule->direction = 'S'; /* Sud - vers le bas */
        break;
    case '<':
        vehicule->direction = 'O'; /* Ouest - vers la gauche */
        break;
    case '>':
        vehicule->direction = 'E'; /* Est - vers la droite */
        break;
    default:
        /* Pas de flèche, garder la direction actuelle */
        break;
    }
}

void orienter_carrosserie(VEHICULE *vehicule)
{
    if (!vehicule)
        return;

    /* Créer une carrosserie avec avant/arrière visibles selon la direction */
    switch (vehicule->direction)
    {
    case 'N': /* Nord - vers le haut (avant en haut) */
        strcpy(vehicule->Carrosserie[0], "/^\\");  /* Avant pointu vers haut */
        strcpy(vehicule->Carrosserie[1], "[#]");  /* Corps */
        strcpy(vehicule->Carrosserie[2], "[_]");  /* Arrière plat */
        break;
    case 'S': /* Sud - vers le bas (avant en bas) */
        strcpy(vehicule->Carrosserie[0], "[_]");  /* Arrière plat */
        strcpy(vehicule->Carrosserie[1], "[#]");  /* Corps */
        strcpy(vehicule->Carrosserie[2], "\\v/");  /* Avant pointu vers bas */
        break;
    case 'E': /* Est - vers la droite (avant à droite) */
        strcpy(vehicule->Carrosserie[0], " __");  /* Toit */
        strcpy(vehicule->Carrosserie[1], "[#>");  /* Corps + avant pointu droite */
        strcpy(vehicule->Carrosserie[2], " ==");  /* Bas */
        break;
    case 'O': /* Ouest - vers la gauche (avant à gauche) */
        strcpy(vehicule->Carrosserie[0], "__ ");  /* Toit */
        strcpy(vehicule->Carrosserie[1], "<#]");  /* Avant pointu gauche + corps */
        strcpy(vehicule->Carrosserie[2], "== ");  /* Bas */
        break;
    }
}

void deplacer_vehicule(VEHICULE *vehicule, PlanParking *plan)
{
    if (!vehicule || !plan || vehicule->etat != '1')
        return;

    /* D'abord, vérifier si on doit suivre une flèche */
    char ancienne_direction = vehicule->direction;
    suivre_fleches(vehicule, plan);

    /* Si la direction a changé, orienter la carrosserie */
    if (ancienne_direction != vehicule->direction)
    {
        orienter_carrosserie(vehicule);
    }

    int nouveau_x = vehicule->posx;
    int nouveau_y = vehicule->posy;

    switch (vehicule->direction)
    {
    case 'N':
        nouveau_y -= vehicule->vitesse;
        break;
    case 'S':
        nouveau_y += vehicule->vitesse;
        break;
    case 'E':
        nouveau_x += vehicule->vitesse;
        break;
    case 'O':
        nouveau_x -= vehicule->vitesse;
        break;
    default:
        return;
    }

    if (peut_deplacer(vehicule, plan, nouveau_x, nouveau_y))
    {
        vehicule->posx = nouveau_x;
        vehicule->posy = nouveau_y;
    }
    /* Sinon le véhicule reste bloqué sur place */
}

void deplacer_tous_vehicules(l_car *vehicules, PlanParking *plan)
{
    if (!vehicules || est_vide_liste_car(vehicules))
        return;

    VEHICULE *curr = vehicules->premier;
    while (curr != NULL)
    {
        if (curr->etat == '1')
        {
            deplacer_vehicule(curr, plan);
        }
        curr = curr->NXT;
    }
}