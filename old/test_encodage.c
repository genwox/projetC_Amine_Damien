#include <stdio.h>
#include <locale.h>

int main()
{
    // Configuration de la locale
    setlocale(LC_ALL, "");

    // Test avec buffer TROP PETIT (200 bytes)
    printf("=== Buffer 200 bytes (TROP PETIT) ===\n");
    FILE* f1 = fopen("plan.txt", "r");
    if (f1) {
        char ligne[200];
        if (fgets(ligne, 200, f1)) {
            printf("Ligne 1: %s\n", ligne);
        }
        fclose(f1);
    }

    printf("\n");

    // Test avec buffer SUFFISANT (600 bytes)
    printf("=== Buffer 600 bytes (SUFFISANT) ===\n");
    FILE* f2 = fopen("plan.txt", "r");
    if (f2) {
        char ligne[600];
        if (fgets(ligne, 600, f2)) {
            printf("Ligne 1: %s\n", ligne);
        }
        fclose(f2);
    }

    // Test de caractères UTF-8 directement
    printf("\n=== Test caractères UTF-8 directs ===\n");
    printf("Double ligne: ════════════════\n");
    printf("Simple ligne: ────────────────\n");
    printf("Coins: ╔═══╗\n");
    printf("       ║   ║\n");
    printf("       ╚═══╝\n");

    return 0;
}
