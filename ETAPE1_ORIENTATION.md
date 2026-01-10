# ETAPE 1 : Système d'Orientation Stable (IMPLÉMENTÉ)

## Résumé

Ce document décrit l'implémentation du système d'orientation stable pour les véhicules, basé sur le comptage de "passes" et la détection de flèches dans une zone, **SANS utiliser la distance Manhattan**.

## Modifications apportées

### Fichier modifié : `src/mouvement.c`

#### 1. Constantes ajoutées

```c
#define DEBUG_ORIENT 0      // Activer les logs de debug (0=désactivé, 1=activé)
#define MAX_SCAN 10         // Nombre max de cellules à scanner pour les passes
#define RAYON_SCAN 8        // Rayon de scan pour détecter les flèches
```

#### 2. Fonctions helpers implémentées

- **`est_fleche_simple(wchar_t c)`** : Vérifie si un caractère est une flèche simple (←→↑↓)
- **`sens_fleche(wchar_t c)`** : Retourne la direction d'une flèche ('N', 'S', 'E', 'O')
- **`compter_passes(plan, cx, cy, dir)`** : Compte le nombre de cellules roulables consécutives dans une direction (max MAX_SCAN)
- **`compter_fleches_zone(plan, cx, cy, R, counts[4])`** : Compte les flèches simples dans un rayon R autour de (cx, cy)

#### 3. Fonction principale

- **`choisir_direction_stable(vehicule, plan)`** : Choisit la direction optimale selon les règles A/B/C/D

### Logique de décision implémentée

#### RÈGLE A : Flèche immédiate
Si la cellule actuelle du véhicule contient une flèche simple (←→↑↓), suivre immédiatement cette direction.

#### RÈGLE B : Scan de zone avec calcul de scores
Si pas de flèche immédiate :
1. Scanner une zone de rayon `RAYON_SCAN` (8 cellules) autour du véhicule
2. Compter les flèches simples par direction (bonus_fleches)
3. Compter les "passes" disponibles dans chaque direction (cellules roulables consécutives)
4. Calculer les scores : `score(dir) = passes(dir) + bonus_fleches(dir)`

#### RÈGLE C : Priorité Y puis X
1. **Priorité Y** : Comparer d'abord Nord vs Sud (choisir le meilleur score)
2. Si Y = 0, **Priorité X** : Comparer Est vs Ouest
3. En cas d'égalité : garder direction actuelle si elle est parmi les meilleures (sticky)

#### RÈGLE D : Sticky
Si la direction actuelle a un score égal au meilleur score et > 0, la conserver pour éviter les changements erratiques.

## Intégration

La fonction `choisir_direction_stable()` est appelée dans `deplacer_vehicule_parking_auto()` (ligne ~851) et remplace la logique précédente basée sur les zones de places libres.

## Activation du debug

Pour activer les logs de debug :

1. Éditer `src/mouvement.c` ligne 17 :
   ```c
   #define DEBUG_ORIENT 1  // Changer 0 en 1
   ```

2. Recompiler :
   ```bash
   make clean && make
   ```

3. Lancer le programme :
   ```bash
   make run
   ```

### Format des logs de debug

**Flèche immédiate :**
```
[ORIENT] (x,y) FLECHE_IMMEDIATE ← -> O
```

**Calcul de scores :**
```
[ORIENT] (x,y) dir=S->N | passes[N=5 S=2 E=0 O=0] fleches[N=2 S=1 E=0 O=0] scores[N=7 S=3 E=0 O=0]
```

Explications :
- `(x,y)` : position actuelle du véhicule
- `dir=S->N` : direction actuelle (S) → nouvelle direction (N)
- `passes[...]` : nombre de cellules roulables consécutives par direction
- `fleches[...]` : nombre de flèches détectées dans la zone par direction
- `scores[...]` : scores totaux (passes + fleches) par direction

## Contraintes respectées

✅ Aucun champ ajouté à la struct VEHICULE
✅ Aucune distance Manhattan utilisée
✅ Réutilisation de `est_cellule_roulable()` existant
✅ Utilisation du plan wchar existant
✅ Modifications limitées à l'orientation (pas de spawn, collision, parking)

## Tests suggérés

1. **Test de base** : Lancer avec `make run` et observer le mouvement des véhicules
2. **Test avec debug** : Activer `DEBUG_ORIENT 1` et vérifier les logs dans stderr
3. **Test des flèches** : Vérifier que les véhicules suivent les flèches du plan.txt
4. **Test de stabilité** : Vérifier que les véhicules ne changent pas constamment de direction

## Prochaines étapes possibles

- ETAPE 2 : Gestion du spawn à l'entrée
- ETAPE 3 : Détection et résolution des collisions
- ETAPE 4 : Assignation intelligente des places de parking
