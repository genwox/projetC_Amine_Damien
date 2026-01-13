#include "mouvement/sprites.h"
#include <stdio.h>
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
void orienter_carrosserie(VEHICULE *vehicule)
{
    if (!vehicule)
        return;

    // Déterminer le fichier sprite basé sur la direction
    const char *fichier;
    switch (vehicule->direction)
    {
    case 'N': fichier = "car_smallN.txt"; break;
    case 'S': fichier = "car_smallS.txt"; break;
    case 'E': fichier = "car_smallE.txt"; break;
    case 'O': fichier = "car_smallO.txt"; break;
    default:  fichier = "car_smallS.txt"; break;
    }

    // Charger le sprite depuis le fichier
    FILE *f = fopen(fichier, "r");
    if (!f) return;

    for (int i = 0; i < 4; i++) {
        if (fgets(vehicule->Carrosserie[i], 30, f) != NULL) {
            size_t len = strlen(vehicule->Carrosserie[i]);
            if (len > 0 && vehicule->Carrosserie[i][len - 1] == '\n') {
                vehicule->Carrosserie[i][len - 1] = '\0';
            }
        } else {
            vehicule->Carrosserie[i][0] = '\0';
        }
    }
    fclose(f);
}
