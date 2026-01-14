#ifndef MATRICE_H
#define MATRICE_H

/* Une case de la matrice (o = 1 si occupé, 0 sinon) */
typedef struct case_c {
    int o;
} ca;

/* Matrice 2D pour savoir où c'est occupé */
typedef struct matrice {
    int n;  /* nb lignes */
    int m;  /* nb colonnes */
    ca **tab;
} mat;

/* Crée une matrice n x m, tout à 0 au départ */
mat *creer_matrice(int n, int m);

/* Met une case à 1 (occupé) */
void remplir_case(mat *m, int x, int y);

/* Met une case à 0 (libre) */
void liberer_case(mat *m, int x, int y);

/* Libère la mémoire de la matrice */
void detruire_matrice(mat **m);

#endif
