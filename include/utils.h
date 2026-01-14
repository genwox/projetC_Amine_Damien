#ifndef UTILS_H
#define UTILS_H

#include "plan.h"
#include <wchar.h>

/*
 * Fonctions utilitaires (petits tests + directions + UTF-8).
 * Sert un peu partout dans le projet.
 */

/* Vérifie si (x,y) est dans le plan */
int est_dans_limites(PlanParking *plan, int x, int y);

/* Infos simples sur une direction (symbole + déplacement) */
typedef struct
{
    char symbole;    /* 'N', 'S', 'E', 'O' */
    int dx;          /* -1, 0, +1 */
    int dy;          /* -1, 0, +1 */
    const char *nom; /* "Nord", "Sud", ... */
} Direction;

/* Récupère les infos d'une direction à partir de son symbole */
const Direction *get_dir(char c);

/* Convertit une direction ('N','S','E','O') en (dx,dy) */
void obtenir_delta_direction(char direction, int *dx, int *dy);

/* Helpers UTF-8 (pour les caractères spéciaux du plan) */
int largeur_affichage_utf8(unsigned char byte);
int avancer_utf8_char(const char **str);

/*
 * Cherche une cellule qui passe un test dans un carré autour de (cx,cy).
 * Pratique pour retrouver une flèche/route proche si on est décalé.
 */
int trouver_cellule_dans_zone(PlanParking *plan, int cx, int cy, int rayon,
                              int (*test)(wchar_t), int *result_x, int *result_y);

/* Test : est-ce une flèche de parking (↑ ou ↓) */
int est_fleche_parking(wchar_t c);

/* Cellule "roulable" (route / flèches / point) */
static inline int est_cellule_passable(wchar_t c)
{
    return c == L' ' ||
           c == L'←' || c == L'→' || c == L'↑' || c == L'↓' ||
           c == L'.';
}

/* Cellule d'allée (roulable + certaines bordures utilisées dans le plan) */
static inline int est_cellule_allee(wchar_t c)
{
    return est_cellule_passable(c) ||
           c == L'║' || c == L'═' || c == L'╦' || c == L'╩';
}

/* Macros de vérification simple (évite de répéter les mêmes if) */
#define VERIFIER_ENTREES(v, p) \
    if (!(v) || !(p))          \
    return 0
#define VERIFIER_ENTREES_VOID(v, p) \
    if (!(v) || !(p))               \
    return
#define VERIFIER_ENTREES_CHAR(v, p, def) \
    if (!(v) || !(p))                    \
    return (def)

#endif
