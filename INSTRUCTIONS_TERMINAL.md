# Instructions pour voir les voitures

## Problème

Le plan fait **299 colonnes** de large et l'entrée est à la **colonne 114**.
Pour voir les voitures, votre terminal doit faire **au moins 120 colonnes de large**.

## Solution 1 : Vérifier la taille de votre terminal

```bash
tput cols
```

Si le résultat est **inférieur à 120**, vous devez agrandir votre terminal.

## Solution 2 : Agrandir votre terminal

### Option A : Redimensionner la fenêtre du terminal
1. Cliquez sur les bords de la fenêtre du terminal
2. Étirez la fenêtre vers la droite jusqu'à ce qu'elle fasse au moins 120 colonnes

### Option B : Passer en plein écran
1. Appuyez sur **F11** pour passer en plein écran
2. Ou utilisez le menu de votre terminal pour maximiser la fenêtre

### Option C : Réduire la taille de la police
1. Dans les paramètres de votre terminal, réduisez la taille de la police
2. Cela permet d'afficher plus de colonnes dans la même largeur d'écran

## Solution 3 : Vérifier que ça fonctionne

```bash
# Vérifier la taille
tput cols
# Doit afficher au moins 120

# Lancer le jeu
make run
# Appuyez sur ESPACE
# Les voitures devraient maintenant être visibles !
```

## Si votre écran est trop petit

Si votre écran physique ne permet pas d'avoir 120 colonnes même en plein écran :

1. **Réduire la taille de la police** dans les paramètres du terminal
2. **Utiliser un moniteur externe** plus grand
3. **Déplacer l'entrée** dans le fichier `data/plan.txt` (ligne 27) plus à gauche
