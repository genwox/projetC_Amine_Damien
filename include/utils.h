#ifndef UTILS_H
#define UTILS_H
#include "plan.h"
#include <wchar.h>

int est_dans_limites(PlanParking *plan, int x, int y);
typedef struct { char symbole; int dx, dy; const char* nom; } Direction;
const Direction* get_dir(char c);
int dx_from_dir(char c);
int dy_from_dir(char c);
void obtenir_delta_direction(char direction, int *dx, int *dy);
int largeur_affichage_utf8(unsigned char byte);
int avancer_utf8_char(const char **str);
int trouver_cellule_dans_zone(PlanParking *plan, int cx, int cy, int rayon, int (*test)(wchar_t), int *result_x, int *result_y);
int est_fleche_parking(wchar_t c);
int est_fleche_circulation_horizontal(wchar_t c);

static inline int est_cellule_passable(wchar_t c) {
    return c == L' ' || c == L'←' || c == L'→' || c == L'↑' || c == L'↓' || c == L'.';
}
static inline int est_cellule_allee(wchar_t c) {
    return est_cellule_passable(c) || c == L'║' || c == L'═' || c == L'╦' || c == L'╩';
}

#define VERIFIER_ENTREES(v, p) if (!(v) || !(p)) return 0
#define VERIFIER_ENTREES_VOID(v, p) if (!(v) || !(p)) return
#define VERIFIER_ENTREES_CHAR(v, p, def) if (!(v) || !(p)) return (def)
#endif
