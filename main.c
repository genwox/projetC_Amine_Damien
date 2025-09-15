#include "liste_car.h"
#include "matrice.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    l_car *l = nv_liste_car();

    detruire_liste_car(&l);
    return 0;
}