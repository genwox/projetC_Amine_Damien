#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "matrice.h"

mat *creer_matrice(int n, int m)
{
    mat *mt = malloc(sizeof(mat));
    if (!mt) return NULL;
    
    mt->n = n;
    mt->m = m;
    
    // Allocation du tableau de POINTEURS
    mt->tab = malloc(sizeof(ca*) * n);  // ← CORRECTION : ca* pas case_c
    if (!mt->tab) {
        free(mt);
        return NULL;
    }
    
    // Allocation de chaque ligne
    for (int i = 0; i < n; i++) {
        mt->tab[i] = malloc(sizeof(ca) * m);  // ← ca pas case_c
        if (!mt->tab[i]) {
            // Nettoyage en cas d'erreur
            for (int j = 0; j < i; j++) {
                free(mt->tab[j]);
            }
            free(mt->tab);
            free(mt);
            return NULL;
        }
        
        // Initialiser toutes les cases à 0
        for (int j = 0; j < m; j++) {
            mt->tab[i][j].o = 0;
        }
    }
    
    return mt;
}

void remplir_case(mat *m, int x, int y)
{
    if (m && x >= 0 && x < m->n && y >= 0 && y < m->m) {
        m->tab[x][y].o = 1;
    }
}

void liberer_case(mat *m, int x, int y)
{
    if (m && x >= 0 && x < m->n && y >= 0 && y < m->m) {
        m->tab[x][y].o = 0;
    }
}

void detruire_matrice(mat **m)
{
    if (!m || !*m) return;
    
    for (int i = 0; i < (*m)->n; i++)
    {
        if ((*m)->tab[i]) {
            free((*m)->tab[i]);
            (*m)->tab[i] = NULL;
        }
    }
    
    if ((*m)->tab) {
        free((*m)->tab);
        (*m)->tab = NULL;
    }
    
    free(*m);
    *m = NULL;
}