#ifndef UTILS_H
#define UTILS_H

#include "plan.h"

/* Vérification de limites - Élimine 11+ duplications */
int est_dans_limites(PlanParking *plan, int x, int y);

/* Direction handling - Élimine 6+ switch statements */
void obtenir_delta_direction(char direction, int *dx, int *dy);

/* UTF-8 helpers - Élimine 2-3 duplications */
int largeur_affichage_utf8(unsigned char byte);
int avancer_utf8_char(const char **str);

/* Macro pour vérifications NULL standardisées */
#define VERIFIER_ENTREES(v, p) \
    if (!(v) || !(p)) return 0

#define VERIFIER_ENTREES_VOID(v, p) \
    if (!(v) || !(p)) return

#define VERIFIER_ENTREES_CHAR(v, p, def) \
    if (!(v) || !(p)) return (def)

#endif
