#!/bin/bash
# Script pour tester automatiquement le parking avec logs

cd "$(dirname "$0")"

# Nettoyer les anciens logs
rm -f orientation.log debug_frame0.log spawn.log diagnostic_plan.log

# Lancer le programme en arrière-plan avec input automatique
(
    sleep 0.5   # Attendre que le programme démarre
    echo " "    # Appuyer sur ESPACE pour démarrer
    sleep 10    # Laisser tourner 10 secondes
    echo "q"    # Quitter
) | timeout 15 ./bin/parking 2> orientation.log

# Afficher un résumé des logs générés
echo "==================================="
echo "Tests terminés. Logs générés :"
echo "==================================="
ls -lh *.log 2>/dev/null | awk '{print $9, "-", $5}'

echo ""
echo "=== DIAGNOSTIC PLAN ==="
if [ -f diagnostic_plan.log ]; then
    head -30 diagnostic_plan.log
else
    echo "Fichier non trouvé"
fi

echo ""
echo "=== LOGS D'ORIENTATION (premiers 50 lignes) ==="
if [ -f orientation.log ]; then
    head -50 orientation.log
else
    echo "Fichier non trouvé"
fi

echo ""
echo "=== LOGS DE SPAWN (premiers 30 lignes) ==="
if [ -f spawn.log ]; then
    head -30 spawn.log
else
    echo "Fichier non trouvé"
fi
