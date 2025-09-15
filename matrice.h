#ifndef MATRICE_H
#define MATRICE_H

typedef struct case_c ca;

struct case_c
{
    int o; /*pour savoir si case occupé ou non: 1 pour oui, 0 pour non  */
};

typedef struct matrice mat;

struct matrice
{
    int n, m; /*longueur et largeur*/
    ca **tab; /*matrice contenant les cases*/
};

mat *creer_matrice(int n, int m);

void remplir_case(mat *m, int x, int y);
void liberer_case(mat *m, int x, int y);

void detruire_matrice(mat **m);

#endif