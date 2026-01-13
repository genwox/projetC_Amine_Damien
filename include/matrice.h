#ifndef MATRICE_H
#define MATRICE_H

typedef struct case_c { int o; } ca;
typedef struct matrice { int n, m; ca **tab; } mat;

mat *creer_matrice(int n, int m);
void remplir_case(mat *m, int x, int y);
void liberer_case(mat *m, int x, int y);
void detruire_matrice(mat **m);
#endif
