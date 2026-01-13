#ifndef MATRICE_H
#define MATRICE_H

/**
 * Représente une case de la matrice d'occupation.
 *
 * @field o État d'occupation (1 = occupé, 0 = libre)
 */
typedef struct case_c {
    int o;
} ca;

/**
 * Matrice 2D pour gérer l'occupation du parking.
 *
 * @field n   Nombre de lignes
 * @field m   Nombre de colonnes
 * @field tab Tableau 2D dynamique de cases
 */
typedef struct matrice {
    int n;
    int m;
    ca **tab;
} mat;

/**
 * Crée une matrice d'occupation de taille n x m.
 * Toutes les cases sont initialisées à 0 (libres).
 *
 * @param n Nombre de lignes (doit être > 0)
 * @param m Nombre de colonnes (doit être > 0)
 * @return  Pointeur vers la matrice créée, NULL en cas d'erreur d'allocation
 */
mat *creer_matrice(int n, int m);

/**
 * Marque une case comme occupée (o = 1).
 *
 * @param m Matrice à modifier (doit être non NULL)
 * @param x Ligne (doit être dans [0..n-1])
 * @param y Colonne (doit être dans [0..m-1])
 */
void remplir_case(mat *m, int x, int y);

/**
 * Marque une case comme libre (o = 0).
 *
 * @param m Matrice à modifier (doit être non NULL)
 * @param x Ligne (doit être dans [0..n-1])
 * @param y Colonne (doit être dans [0..m-1])
 */
void liberer_case(mat *m, int x, int y);

/**
 * Détruit une matrice et libère toute sa mémoire.
 *
 * @param m Pointeur vers le pointeur de matrice (sera mis à NULL)
 */
void detruire_matrice(mat **m);

#endif
