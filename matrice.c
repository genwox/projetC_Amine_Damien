#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "matrice.h"

mat *creer_matrice(int n, int m)
{
    mat *mt = malloc(sizeof(struct matrice));
    mt->n = n;
    mt->m = m;
    mt->tab = malloc(sizeof(struct case_c) * n);

    for (int i = 0; i < n; i++)
        mt->tab[i] = malloc(sizeof(struct case_c) * m);

    return mt;
}
void remplir_case(mat *m, int x, int y)
{
    m->tab[x][y].o = 1;
}

void liberer_case(mat *m, int x, int y)
{
    m->tab[x][y].o = 0;
}

void detruire_matrice(mat **m)
{
    for (int i = 0; i < (*m)->n; i++)
    {
        free((*m)->tab[i]);
        (*m)->tab[i] = NULL;
    }

    free((*m)->tab);
    (*m)->tab = NULL;
    free((*m));
    (*m) = NULL;
}