# Problème : Les voitures ne s'affichent pas

## Diagnostic

✅ Les voitures sont créées correctement (logs le confirment)
✅ Les voitures se déplacent correctement
✅ Les sprites sont chargés

❌ **Problème trouvé** : Les voitures spawnent HORS DE L'ÉCRAN !

## Explication technique

### Taille du plan
- Le fichier `data/plan.txt` fait **35 lignes**
- Avec le décalage d'affichage (+4 pour le titre), ça fait **39 lignes totales**

### Position de spawn
- L'entrée est marquée à la **ligne 27** du plan.txt (symbole 'E')
- Le véhicule spawn à `posy=34` (bas du plan)
- Avec le décalage, ça s'affiche à la ligne **38** (34+4) du terminal

### Taille du terminal
- Un terminal standard fait **24 lignes** de hauteur
- Le plan nécessite **~40 lignes minimum** pour tout voir
- Les lignes 25-40 sont **hors de l'écran visible**

## Solution rapide : Agrandir le terminal

### Option 1 : Agrandir la fenêtre du terminal

**Avant de lancer `make run`, agrandissez votre fenêtre de terminal pour avoir au moins 40-45 lignes de hauteur.**

Pour vérifier la taille actuelle dans le programme, les voitures devraient apparaître si votre terminal a suffisamment de lignes.

### Option 2 : Réduire le plan

Vous pouvez créer un plan plus petit en éditant `data/plan.txt` et en supprimant quelques lignes du haut (titre sortie) ou en déplaçant l'entrée plus haut.

### Option 3 : Modifier le code (plus complexe)

Il faudrait implémenter un système de "viewport" qui scroll automatiquement pour suivre les véhicules.

## Test rapide

```bash
# 1. Agrandissez votre terminal au maximum (hauteur ≥ 40 lignes)
# 2. Lancez :
make run

# 3. Les voitures devraient maintenant être visibles !
```

## Vérification rapide dans le code

Le problème vient de `affichage.c:341` :
```c
mvprintw(vehicule->posy + i + 4, vehicule->posx + 2, "%s", vehicule->Carrosserie[i]);
```

- `vehicule->posy = 34` (position dans le plan)
- `+ i + 4` : décalage pour le titre (+4) et les lignes de la carrosserie (+i)
- Résultat : ligne 38+ → **hors écran si terminal < 40 lignes**

## Solution définitive recommandée

Modifier le spawn pour placer les véhicules plus haut dans le plan, ou implémenter un viewport qui centre automatiquement sur la zone d'action.
