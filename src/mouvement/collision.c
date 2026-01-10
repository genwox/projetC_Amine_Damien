#include "mouvement/collision.h"
#include "mouvement/sprites.h"
#include <wchar.h>
#include <stdlib.h>

// ============================================================================
// DÉTECTION DE COLLISION ET VÉRIFICATIONS DE DÉPLACEMENT
// ============================================================================

// Vérifie si une cellule est roulable
int est_cellule_roulable(PlanParking *plan, int x, int y)
{
    if (x < 0 || y < 0 || x >= plan->largeur || y >= plan->hauteur)
        return 0;

    wchar_t c = plan->plan_statique[y][x];

    /* Allées uniquement (espaces + flèches) */
    return (c == L' ' ||
            c == L'←' || c == L'→' || c == L'↑' || c == L'↓' ||
            c == L'.' ||                                       // Point (route)
            c == L'E' || c == L'S' || c == L'e' || c == L's'); // Marqueurs entrée/sortie
}

int est_cellule_roulable_externe(PlanParking *plan, int x, int y)
{
    return est_cellule_roulable(plan, x, y);
}

int peut_deplacer(VEHICULE *vehicule, PlanParking *plan, int nouveau_x, int nouveau_y)
{
    if (!vehicule || !plan || vehicule->etat != '1')
        return 0;

    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);

    /* Vérification MINIMALE des limites: juste éviter de sortir complètement du plan */
    /* Plus de marges strictes - on autorise les voitures à aller près des bords pour virages */
    if (nouveau_x < 0 || nouveau_y < 0)
        return 0;

    if (nouveau_x + largeur > plan->largeur || nouveau_y + hauteur > plan->hauteur)
        return 0;

    for (int dy = 0; dy < hauteur; dy++)
    {
        for (int dx = 0; dx < largeur; dx++)
        {
            int check_x = nouveau_x + dx;
            int check_y = nouveau_y + dy;

            wchar_t c = plan->plan_statique[check_y][check_x];

            /* Bloquer les bordures et murs */
            if (c == L'╔' || c == L'╗' || c == L'╚' || c == L'╝' ||
                c == L'═' || c == L'║' || c == L'╦' || c == L'╩' || c == L'╬' ||
                c == L'|' || c == L'_' || c == L'-' ||
                c == L'[' || c == L']' || c == L'#')
            {
                return 0;
            }

            /* Tout le reste est autorisé (espaces, flèches, lettres, etc.) */
        }
    }

    return 1;
}

int peut_deplacer_sur_allee(VEHICULE *vehicule, PlanParking *plan, int nouveau_x, int nouveau_y)
{
    if (!vehicule || !plan)
        return 0;

    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule, &largeur, &hauteur);

    /* Vérification MINIMALE des limites */
    if (nouveau_x < 0 || nouveau_y < 0)
        return 0;

    if (nouveau_x + largeur > plan->largeur || nouveau_y + hauteur > plan->hauteur)
        return 0;

    /* Vérifier que TOUTES les cellules du sprite sont roulables OU bordures de place */
    for (int dy = 0; dy < hauteur; dy++)
    {
        for (int dx = 0; dx < largeur; dx++)
        {
            int check_x = nouveau_x + dx;
            int check_y = nouveau_y + dy;

            if (check_x >= 0 && check_y >= 0 && check_x < plan->largeur && check_y < plan->hauteur)
            {
                wchar_t c = plan->plan_statique[check_y][check_x];

                /* Autoriser: allées + bordures de places + flèches de virage */
                int ok = (c == L' ' ||
                         c == L'←' || c == L'→' || c == L'↑' || c == L'↓' ||  // Flèches droites
                         c == L'⮠' || c == L'⮡' || c == L'⮢' || c == L'⮣' ||  // Flèches virage 1
                         c == L'⮤' || c == L'⮥' || c == L'⮦' || c == L'⮧' ||  // Flèches virage 2
                         c == L'.' ||
                         c == L'E' || c == L'S' || c == L'e' || c == L's' ||
                         c == L'║' || c == L'═' || c == L'╦' || c == L'╩'); /* Bordures places */

                if (!ok)
                    return 0;
            }
        }
    }

    return 1; /* Toutes les cellules sont OK */
}

int vehicules_en_collision(VEHICULE *v1, VEHICULE *v2)
{
    if (!v1 || !v2 || v1->etat != '1' || v2->etat != '1')
        return 0;

    // Obtenir les dimensions des véhicules
    int largeur1, hauteur1, largeur2, hauteur2;
    obtenir_dimensions_vehicule(v1, &largeur1, &hauteur1);
    obtenir_dimensions_vehicule(v2, &largeur2, &hauteur2);

    /* TOLÉRANCE : Réduire les bounding boxes de 2 cellules de chaque côté
     * pour éviter les faux positifs quand les voitures se frôlent.
     * Une vraie collision nécessite un chevauchement d'au moins 2 cellules.
     */
    const int TOLERANCE = 2;

    // Calculer les bounding boxes (rectangles) avec tolérance
    int x1_min = v1->posx + TOLERANCE;
    int x1_max = v1->posx + largeur1 - TOLERANCE;
    int y1_min = v1->posy + TOLERANCE;
    int y1_max = v1->posy + hauteur1 - TOLERANCE;

    int x2_min = v2->posx + TOLERANCE;
    int x2_max = v2->posx + largeur2 - TOLERANCE;
    int y2_min = v2->posy + TOLERANCE;
    int y2_max = v2->posy + hauteur2 - TOLERANCE;

    // Vérifier que les bounding boxes sont valides
    if (x1_max <= x1_min || y1_max <= y1_min || x2_max <= x2_min || y2_max <= y2_min)
        return 0;

    // Test AABB (Axis-Aligned Bounding Box)
    int separated = (x1_max <= x2_min) ||  // v1 complètement à gauche de v2
                    (x2_max <= x1_min) ||  // v2 complètement à gauche de v1
                    (y1_max <= y2_min) ||  // v1 complètement au-dessus de v2
                    (y2_max <= y1_min);    // v2 complètement au-dessus de v1

    return !separated;  // Collision si pas séparés
}

int voie_libre_direction(VEHICULE *vehicule_actuel, l_car *tous_vehicules, char direction_cible, PlanParking *plan)
{
    if (!vehicule_actuel || !tous_vehicules || !plan)
        return 1; // Par défaut, considérer libre

    // Distance de détection dans la direction cible
    const int DISTANCE_DETECTION = 8;

    int largeur, hauteur;
    obtenir_dimensions_vehicule(vehicule_actuel, &largeur, &hauteur);
    int centre_x = vehicule_actuel->posx + largeur / 2;
    int centre_y = vehicule_actuel->posy + hauteur / 2;

    // Calculer la zone à vérifier selon la direction
    int check_x_start, check_x_end, check_y_start, check_y_end;

    switch (direction_cible)
    {
    case 'N':
        check_x_start = centre_x - 2;
        check_x_end = centre_x + 2;
        check_y_start = centre_y - DISTANCE_DETECTION;
        check_y_end = centre_y;
        break;
    case 'S':
        check_x_start = centre_x - 2;
        check_x_end = centre_x + 2;
        check_y_start = centre_y;
        check_y_end = centre_y + DISTANCE_DETECTION;
        break;
    case 'E':
        check_x_start = centre_x;
        check_x_end = centre_x + DISTANCE_DETECTION;
        check_y_start = centre_y - 2;
        check_y_end = centre_y + 2;
        break;
    case 'O':
        check_x_start = centre_x - DISTANCE_DETECTION;
        check_x_end = centre_x;
        check_y_start = centre_y - 2;
        check_y_end = centre_y + 2;
        break;
    default:
        return 1;
    }

    // Parcourir tous les autres véhicules actifs
    VEHICULE *autre = tous_vehicules->premier;
    while (autre != NULL)
    {
        // Ignorer le véhicule actuel et les véhicules garés
        if (autre == vehicule_actuel || autre->etat != '1')
        {
            autre = autre->NXT;
            continue;
        }

        // Obtenir le centre de l'autre véhicule
        int autre_largeur, autre_hauteur;
        obtenir_dimensions_vehicule(autre, &autre_largeur, &autre_hauteur);
        int autre_centre_x = autre->posx + autre_largeur / 2;
        int autre_centre_y = autre->posy + autre_hauteur / 2;

        // Vérifier si l'autre véhicule est dans la zone de détection
        if (autre_centre_x >= check_x_start && autre_centre_x <= check_x_end &&
            autre_centre_y >= check_y_start && autre_centre_y <= check_y_end)
        {
            return 0; // Voie non libre
        }

        autre = autre->NXT;
    }

    return 1; // Voie libre
}
