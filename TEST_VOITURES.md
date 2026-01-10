# Test de visibilité des voitures

## Problème RÉSOLU !

Le problème était qu'il y avait **deux fichiers plan.txt** :
1. `/home/genwox/ING/3A/S1/ProjetC/plan.txt` (ANCIEN, 39 lignes, entrée à colonne 114) ❌
2. `/home/genwox/ING/3A/S1/ProjetC/data/plan.txt` (NOUVEAU, 35 lignes, entrée à colonne 94) ✅

Le programme chargeait l'ancien fichier !

## Solution appliquée

✅ Suppression du `plan.txt` à la racine
✅ Modification du code pour charger `data/plan.txt`
✅ Les voitures spawnent maintenant à la **colonne 94** au lieu de 114

## Position actuelle des voitures

- Position dans le plan : **colonne 94, ligne 26**
- Position à l'écran : **colonne 96, ligne 30** (avec les offsets d'affichage)

## Taille de terminal requise

Pour voir les voitures, votre terminal doit faire **au moins 100 colonnes** de large.

### Vérifier la taille de votre terminal

```bash
tput cols
# Doit afficher au moins 100
```

### Si votre terminal est trop petit

1. **Appuyez sur F11** pour passer en plein écran
2. **Réduisez la taille de la police** dans les paramètres du terminal
3. **Redimensionnez la fenêtre** en étirant vers la droite

## Test

```bash
make run
# Appuyez sur ESPACE
# Les voitures devraient être visibles si votre terminal fait ≥ 100 colonnes !
```

## Vérification rapide

Le diagnostic doit maintenant afficher :
```
Dimensions: 35x150
Places detectees: 18
entree = (94,26)
```

Au lieu de l'ancien :
```
Dimensions: 39x150
Places detectees: 25
entree = (114,34)
```
