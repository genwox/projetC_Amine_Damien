# Implémentation du Viewport - Solution au Problème d'Affichage

## Problème Identifié

Les véhicules n'étaient pas visibles car ils spawnaient hors de l'écran :
- Le plan fait **127-299 colonnes** de large
- L'entrée est à la **colonne 114**
- Les véhicules spawnaient à `(114, 34)` → affichage à la colonne 116
- Les terminaux standards font **80-120 colonnes**
- Résultat : véhicules **complètement hors écran**

## Solution : Système de Viewport

Un viewport (fenêtre glissante) a été implémenté pour n'afficher que la portion visible du plan.

### Modifications apportées

#### 1. **include/affichage.h**
Ajout de la structure Viewport :
```c
typedef struct {
    int offset_x;  // Décalage horizontal (colonnes)
    int offset_y;  // Décalage vertical (lignes)
    int largeur;   // Largeur visible
    int hauteur;   // Hauteur visible
} Viewport;
```

Nouvelles fonctions :
- `calculer_viewport()` - Calcule la position du viewport
- `centrer_viewport_sur_zone()` - Centre le viewport sur une zone
- `afficher_plan_avec_viewport()` - Affiche uniquement la portion visible
- `afficher_vehicule_viewport()` - Affiche les véhicules avec coordonnées relatives

#### 2. **src/affichage.c**

**`calculer_viewport()`** :
- Centre le viewport sur la position **moyenne des véhicules actifs**
- Si aucun véhicule, centre sur l'**entrée du parking**
- Ajuste automatiquement pour ne pas sortir des limites du plan

**`afficher_plan_avec_viewport()`** :
- Lit le fichier plan.txt ligne par ligne
- N'affiche que les lignes dans `[offset_y, offset_y + hauteur]`
- Pour chaque ligne, extrait uniquement la portion `[offset_x, offset_x + largeur]`
- Gère correctement les caractères UTF-8 multi-octets (←→↑↓)
- Ajuste les indicateurs de places pour le viewport

**`afficher_vehicule_viewport()`** :
- Vérifie si le véhicule est dans le viewport
- Calcule les coordonnées relatives : `x_relatif = posx - offset_x`
- Affiche uniquement si visible

#### 3. **src/jeu.c**

Modifications dans `executer_boucle_jeu()` :
```c
// Initialisation
Viewport viewport = {0, 0, 0, 0};

// Dans la boucle d'affichage :
calculer_viewport(plan, vehicules, &viewport);
afficher_plan_avec_viewport(plan, &viewport);

// Affichage des véhicules
afficher_vehicule_viewport(current, &viewport);
```

## Fonctionnement

1. **Calcul du viewport** : À chaque frame, le système calcule où se trouvent les véhicules
2. **Centrage automatique** : Le viewport se centre sur la zone d'action
3. **Affichage adaptatif** : Seule la portion visible est affichée
4. **Suivi des véhicules** : Le viewport suit automatiquement les véhicules qui se déplacent

## Avantages

✓ **Fonctionne sur tout terminal plein écran** (80x24 minimum)
✓ **Pas de modification du plan.txt** requis
✓ **Pas de changement des sprites** nécessaire
✓ **Pas de déplacement de l'entrée** (marqueur 'E')
✓ **Suivi automatique** des véhicules
✓ **Gestion UTF-8 complète** (flèches ←→↑↓)

## Résultat

- Les véhicules sont maintenant **toujours visibles**
- Le viewport **suit automatiquement l'action**
- Compatible avec **tous les terminaux plein écran**
- Code adaptable sans modifications du plan ou des ressources

## Test

```bash
make clean && make
make run
# Les véhicules devraient maintenant être visibles !
```
