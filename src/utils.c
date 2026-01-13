#include "utils.h"
#include <stddef.h>

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
