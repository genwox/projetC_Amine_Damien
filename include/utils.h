#ifndef UTILS_H
#define UTILS_H

#include "plan.h"
#include <wchar.h>

/**
 * Vérifie si une position (x, y) est dans les limites du plan.
 *
 * @param plan Plan à vérifier (peut être NULL)
 * @param x    Colonne à tester
 * @param y    Ligne à tester
 * @return     1 si dans les limites, 0 sinon
 */
int est_dans_limites(PlanParking *plan, int x, int y);

/**
 * Représente une direction avec son symbole et ses deltas.
 *
 * @field symbole Caractère de direction ('N', 'S', 'E', 'O')
 * @field dx      Déplacement en X (-1, 0, +1)
 * @field dy      Déplacement en Y (-1, 0, +1)
 * @field nom     Nom complet ("Nord", "Sud", "Est", "Ouest")
 */
typedef struct {
    char symbole;
    int dx;
    int dy;
    const char* nom;
} Direction;

/**
 * Retourne la structure Direction correspondant à un symbole.
 *
 * @param c Caractère de direction ('N', 'S', 'E', 'O')
 * @return  Pointeur vers la Direction statique, NULL si caractère inconnu
 */
const Direction* get_dir(char c);

/**
 * Convertit une direction en deltas de déplacement.
 *
 * @param direction Direction ('N', 'S', 'E', 'O')
 * @param dx        Pointeur où écrire le delta X (doit être non NULL)
 * @param dy        Pointeur où écrire le delta Y (doit être non NULL)
 */
void obtenir_delta_direction(char direction, int *dx, int *dy);

/**
 * Calcule le nombre d'octets d'un caractère UTF-8.
 *
 * @param byte Premier octet du caractère UTF-8
 * @return     Nombre d'octets (1, 2, 3 ou 4)
 */
int largeur_affichage_utf8(unsigned char byte);

/**
 * Avance un pointeur de chaîne d'un caractère UTF-8.
 *
 * @param str Pointeur vers le pointeur de chaîne (sera modifié)
 * @return    Nombre d'octets avancés
 */
int avancer_utf8_char(const char **str);

/**
 * Recherche une cellule satisfaisant un test dans un rayon donné.
 * Parcourt un carré de côté (2*rayon+1) centré sur (cx, cy).
 *
 * @param plan     Plan où chercher (doit être non NULL)
 * @param cx       Centre X de la zone de recherche
 * @param cy       Centre Y de la zone de recherche
 * @param rayon    Rayon de recherche en cases
 * @param test     Fonction de test sur wchar_t (doit être non NULL)
 * @param result_x Pointeur où écrire X si trouvé (doit être non NULL)
 * @param result_y Pointeur où écrire Y si trouvé (doit être non NULL)
 * @return         1 si une cellule est trouvée, 0 sinon
 */
int trouver_cellule_dans_zone(PlanParking *plan, int cx, int cy, int rayon,
                               int (*test)(wchar_t), int *result_x, int *result_y);

/**
 * Test si un caractère est une flèche de parking (↑ ou ↓).
 *
 * @param c Caractère à tester
 * @return  1 si c'est une flèche parking, 0 sinon
 */
int est_fleche_parking(wchar_t c);

/**
 * Test inline : vérifie si une cellule est passable (route, flèche, point).
 *
 * @param c Caractère à tester
 * @return  1 si passable, 0 sinon
 */
static inline int est_cellule_passable(wchar_t c)
{
    return c == L' ' ||
           c == L'←' || c == L'→' || c == L'↑' || c == L'↓' ||
           c == L'.';
}

/**
 * Test inline : vérifie si une cellule est une allée (passable + bordures).
 *
 * @param c Caractère à tester
 * @return  1 si allée, 0 sinon
 */
static inline int est_cellule_allee(wchar_t c)
{
    return est_cellule_passable(c) ||
           c == L'║' || c == L'═' || c == L'╦' || c == L'╩';
}

/* Macros de vérification des paramètres d'entrée */
#define VERIFIER_ENTREES(v, p)          if (!(v) || !(p)) return 0
#define VERIFIER_ENTREES_VOID(v, p)     if (!(v) || !(p)) return
#define VERIFIER_ENTREES_CHAR(v, p, def) if (!(v) || !(p)) return (def)

#endif
