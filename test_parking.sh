#!/bin/bash
cd data
rm -f ../debug_path.log

# Lancer le simulateur en arrière-plan
timeout 30 ../bin/parking > /dev/null 2>&1 &
PID=$!

# Attendre que le programme démarre
sleep 1

# Spawner plusieurs voitures en envoyant ESPACE plusieurs fois
for i in {1..5}; do
    # Essayer d'envoyer ESPACE
    (echo " " 2>/dev/null) &
    sleep 2
done

# Attendre que les voitures se garent
sleep 15

# Tuer le processus
kill $PID 2>/dev/null || true
wait $PID 2>/dev/null || true

echo "Test terminé. Vérification du fichier de debug..."
if [ -f ../debug_path.log ]; then
    echo "=== Contenu de debug_path.log ==="
    cat ../debug_path.log
else
    echo "ERREUR: debug_path.log n'a pas été créé"
    ls -la ../debug*.log 2>/dev/null || echo "Aucun fichier debug trouvé"
fi
