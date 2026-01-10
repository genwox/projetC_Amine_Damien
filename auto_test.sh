#!/bin/bash
cd data
timeout 8 ../bin/parking > /dev/null 2>../debug.log &
PID=$!
sleep 1
# Simuler appui sur ESPACE pour spawner les voitures
echo " " > /proc/$PID/fd/0 2>/dev/null || true
sleep 5
kill $PID 2>/dev/null || true
wait $PID 2>/dev/null || true
