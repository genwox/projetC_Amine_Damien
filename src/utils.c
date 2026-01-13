#include "utils.h"
#include <stddef.h>
static const Direction DIRECTIONS[] = {
    {'N', 0, -1, "Nord"}, {'S', 0, 1, "Sud"},
    {'E', 1, 0, "Est"}, {'O', -1, 0, "Ouest"}
};
const Direction* get_dir(char c)
{
    for (int i = 0; i < 4; i++) {
        if (DIRECTIONS[i].symbole == c) return &DIRECTIONS[i];
    }
    return NULL;
}
int dx_from_dir(char c)
{
    const Direction *d = get_dir(c);
    return d ? d->dx : 0;
}
int dy_from_dir(char c)
{
    const Direction *d = get_dir(c);
    return d ? d->dy : 0;
}
int est_dans_limites(PlanParking *plan, int x, int y)
{
    if (!plan)
        return 0;
    return (x >= 0 && x < plan->largeur && y >= 0 && y < plan->hauteur);
}
void obtenir_delta_direction(char direction, int *dx, int *dy)
{
    if (!dx || !dy)
        return;
    *dx = 0;
    *dy = 0;
    switch (direction)
    {
    case 'N':
        *dy = -1;
        break;
    case 'S':
        *dy = 1;
        break;
    case 'E':
        *dx = 1;
        break;
    case 'O':
        *dx = -1;
        break;
    }
}
int largeur_affichage_utf8(unsigned char byte)
{
    if ((byte & 0x80) == 0)
        return 1;
    if ((byte & 0xE0) == 0xC0)
        return 2;
    if ((byte & 0xF0) == 0xE0)
        return 3;
    if ((byte & 0xF8) == 0xF0)
        return 4;
    return 1;
}
int avancer_utf8_char(const char **str)
{
    if (!str || !*str || !**str)
        return 0;
    unsigned char byte = (unsigned char)**str;
    int bytes = largeur_affichage_utf8(byte);
    *str += bytes;
    return bytes;
}
int est_fleche_parking(wchar_t c)
{
    return (c == L'↑' || c == L'↓');
}
int est_fleche_circulation_horizontal(wchar_t c)
{
    return (c == L'←' || c == L'→');
}
int trouver_cellule_dans_zone(PlanParking *plan, int cx, int cy, int rayon,
                               int (*test)(wchar_t), int *result_x, int *result_y)
{
    if (!plan || !test || !result_x || !result_y)
        return 0;
    for (int dy = -rayon; dy <= rayon; dy++)
    {
        for (int dx = -rayon; dx <= rayon; dx++)
        {
            int check_x = cx + dx;
            int check_y = cy + dy;
            if (est_dans_limites(plan, check_x, check_y))
            {
                wchar_t c = plan->plan_statique[check_y][check_x];
                if (test(c))
                {
                    *result_x = check_x;
                    *result_y = check_y;
                    return 1;
                }
            }
        }
    }
    return 0;
}
