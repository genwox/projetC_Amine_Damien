#!/bin/bash

# ============================================================================
# Script de nettoyage des anciens fichiers à la racine
# ============================================================================
# Ce script déplace les anciens fichiers vers un répertoire "old/"
# pour garder une structure propre
# ============================================================================

echo "=========================================="
echo "  NETTOYAGE DES ANCIENS FICHIERS"
echo "=========================================="

# Créer le répertoire old/
mkdir -p old

# Liste des fichiers à déplacer
OLD_FILES=(
    "main.c"
    "affichage.c"
    "affichage.h"
    "liste_car.c"
    "liste_car.h"
    "matrice.c"
    "matrice.h"
    "plan.c"
    "plan.h"
    "plan.txt"
    "plan_ascii.txt"
    "plan_unicode.txt"
    "car.txt"
    "camion.txt"
    "suv.txt"
    "test_encodage.c"
    "test_final.c"
    "makefile.old"
    "prog"
    "parking"
    "test_encodage"
    "test_final"
    "test_simple"
)

# Déplacer les fichiers
count=0
for file in "${OLD_FILES[@]}"; do
    if [ -f "$file" ]; then
        mv "$file" "old/"
        echo "✓ Déplacé : $file → old/"
        ((count++))
    fi
done

echo ""
echo "=========================================="
echo "  $count fichier(s) déplacé(s)"
echo "=========================================="
echo ""
echo "Les nouveaux fichiers sont dans :"
echo "  - src/      (sources .c)"
echo "  - include/  (headers .h)"
echo "  - data/     (données)"
echo "  - bin/      (exécutables)"
echo ""
echo "Pour compiler : make"
echo "Pour exécuter : make run"
echo "=========================================="
