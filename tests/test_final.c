#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

int main()
{
    // Configuration OBLIGATOIRE pour UTF-8
    setlocale(LC_ALL, "");

    // Test du plan Unicode
    printf("\n=== TEST PLAN UNICODE (plan.txt) ===\n");
    FILE* f1 = fopen("plan.txt", "r");
    if (f1) {
        char ligne[600];
        int count = 0;
        while (fgets(ligne, 600, f1) && count < 5) {
            printf("%s", ligne);
            count++;
        }
        fclose(f1);
    }

    printf("\n\n");

    // Test du plan ASCII
    printf("=== TEST PLAN ASCII (plan_ascii.txt) ===\n");
    FILE* f2 = fopen("plan_ascii.txt", "r");
    if (f2) {
        char ligne[600];
        int count = 0;
        while (fgets(ligne, 600, f2) && count < 5) {
            printf("%s", ligne);
            count++;
        }
        fclose(f2);
    }

    printf("\n");
    return 0;
}
