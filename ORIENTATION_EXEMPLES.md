# Exemples d'Orientation Stable

Ce document illustre le fonctionnement du système d'orientation avec des exemples concrets.

## Exemple 1 : Flèche immédiate (RÈGLE A)

```
Plan:
    ....→....
    ....[V]...

Position véhicule : (x=4, y=1)
Cellule actuelle : → (flèche droite)

Résultat : Direction = E (Est)
Pas de calcul de score, suivre immédiatement la flèche.
```

## Exemple 2 : Calcul de scores (RÈGLE B)

```
Plan:
    ←...........
    ............
    ....[V].....
    ............
    ......→.....
    ......→.....

Position véhicule : (x=4, y=2)
Cellule actuelle : . (pas de flèche)
Rayon scan : 8

Flèches détectées dans la zone :
- Ouest : 1 flèche (←)
- Est : 2 flèches (→→)

Passes disponibles (cellules roulables consécutives) :
- Nord : 2
- Sud : 4
- Est : 7
- Ouest : 4

Scores totaux (passes + bonus) :
- Nord : 2 + 0 = 2
- Sud : 4 + 0 = 4
- Est : 7 + 2 = 9  ← MEILLEUR
- Ouest : 4 + 1 = 5

Résultat : Direction = E (Est)
```

## Exemple 3 : Priorité Y puis X (RÈGLE C)

### Cas 3a : Priorité Y (Nord/Sud)

```
Scores :
- Nord : 5
- Sud : 3
- Est : 2
- Ouest : 1

Résultat : Direction = N (Nord)
Car Nord > Sud, priorité Y activée.
```

### Cas 3b : Priorité X quand Y = 0

```
Scores :
- Nord : 0
- Sud : 0
- Est : 4
- Ouest : 2

Résultat : Direction = E (Est)
Car Y (Nord/Sud) = 0, on passe à X (Est/Ouest).
```

## Exemple 4 : Sticky (RÈGLE D)

### Cas 4a : Égalité, garder direction actuelle

```
Direction actuelle : N (Nord)

Scores :
- Nord : 5
- Sud : 5  ← Égalité !
- Est : 2
- Ouest : 1

Résultat : Direction = N (Nord)
Car la direction actuelle (N) a le même score que le meilleur (5).
On la garde pour éviter les changements erratiques.
```

### Cas 4b : Pas d'égalité, changer de direction

```
Direction actuelle : N (Nord)

Scores :
- Nord : 3
- Sud : 7  ← Meilleur !
- Est : 2
- Ouest : 1

Résultat : Direction = S (Sud)
Car Sud a un meilleur score que Nord, on change.
```

## Exemple 5 : Scénario complet sur le plan.txt

```
Plan (extrait) :
║                            ←                                ←                                ║
║                                                                                              ║
║                                                                                              ║
║                            →                               →                                 ║

Véhicule à (x=50, y=2) direction actuelle = E

Scan zone (rayon 8) :
- Flèches Ouest détectées : 2 (←←)
- Flèches Est détectées : 2 (→→)

Passes disponibles :
- Nord : 1 (ligne au-dessus)
- Sud : 1 (ligne en-dessous)
- Est : 10 (longue allée)
- Ouest : 10 (longue allée)

Scores :
- Nord : 1 + 0 = 1
- Sud : 1 + 0 = 1
- Est : 10 + 2 = 12  ← MEILLEUR
- Ouest : 10 + 2 = 12 ← ÉGALITÉ !

Résultat : Direction = E (Est)
Car direction actuelle (E) a le même score que le meilleur (12).
Sticky activé, on garde E.
```

## Visualisation des passes

```
Exemple de comptage de passes pour direction Est :

Position véhicule : V
Direction : Est (→)

V . . . # . . .
  ↑ ↑ ↑ ↑
  1 2 3 X (mur)

Passes = 3 (s'arrête au mur)
```

## Logs de debug (DEBUG_ORIENT = 1)

```bash
# Exemple de log flèche immédiate
[ORIENT] (50,8) FLECHE_IMMEDIATE → -> E

# Exemple de log avec calcul de scores
[ORIENT] (45,12) dir=S->E | passes[N=2 S=4 E=10 O=5] fleches[N=0 S=0 E=3 O=1] scores[N=2 S=4 E=13 O=6]
                 ↑        ↑                                                                    ↑
                 pos   S→E changement                                                   E=13 meilleur
```

## Notes importantes

1. **Pas de Manhattan** : Le système ne calcule JAMAIS la distance vers les places. Il se base uniquement sur les passes et les flèches.

2. **Sticky** : Le système évite les changements constants de direction en gardant la direction actuelle quand les scores sont égaux.

3. **Priorité Y > X** : Privilégie toujours Nord/Sud avant Est/Ouest, ce qui crée un mouvement vertical préférentiel.

4. **Flèches = bonus** : Les flèches dans la zone donnent un bonus au score, mais ne sont pas obligatoires. Un chemin avec beaucoup de passes mais sans flèches peut être choisi.

5. **Rayon limité** : Seules les flèches dans un rayon de 8 cellules sont prises en compte, évitant les calculs coûteux.
