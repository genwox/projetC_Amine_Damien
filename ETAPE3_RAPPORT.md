# ETAPE 3 - PARKING AU BON ENDROIT
## Rapport d'implémentation

### Objectif
Implémenter un système de parking correct où les véhicules se garent sur la bonne rangée en fonction des flèches de parking (↓ et ↑).

### Analyse préliminaire

#### Structure du parking détectée
```
Ligne Y-1: ╦ (haut de place - référence utilisée dans plan->places[])
Ligne Y  : ║ + flèches ↓↑ (milieu de la place, dans l'allée)
Ligne Y+1: ╩ (bas de place)
```

#### Positions des flèches dans plan.txt
- Ligne 14 (index 13): ↓ et ↑ à colonnes 13, 18, 44, 49, 76, 81
- Ligne 19 (index 18): ↓ et ↑ aux mêmes colonnes
- Ligne 24 (index 23): ↓ et ↑ aux mêmes colonnes

#### Positions des places (╦)
- Ligne 13 (index 12): colonnes 2, 29, 33, 61, 65, 92 (6 places)
- Ligne 18 (index 17): colonnes 2, 29, 33, 61, 65, 92 (6 places)
- Ligne 23 (index 22): colonnes 2, 29, 33, 61, 65, 92 (6 places)
Total: 18 places

### Problème identifié

**Ancienne logique (CASSÉE)**:
```c
int dy = plan->places[i].ligne - fleche_y;
if (dir_parking == 'S' && dy < 0) continue;  // ↓ voulait places EN-DESSOUS
if (dir_parking == 'N' && dy > 0) continue;  // ↑ voulait places AU-DESSUS
```

**Pourquoi c'était cassé**:
- Flèche ↓ à ligne 13 cherchait places avec dy > 0 (en dessous)
- Mais les places sont à ligne 12 (dy = -1, AU-DESSUS!)
- Résultat: les flèches ↓ ne trouvaient JAMAIS de places

### Solution implémentée

#### Nouveau principe: Mapping par rangée
Les flèches sont **DANS** la rangée de parking, pas entre deux rangées.

**Nouvelle logique**:
```c
/* Calculer la ligne de la rangée : flèche ligne Y → places ligne Y-1 */
int ligne_rangee = fleche_y - 1;

for (int i = 0; i < plan->places_totales; i++)
{
    if (plan->places[i].occupee != 0)
        continue;

    /* La place doit être sur la MÊME RANGÉE (ligne Y-1) */
    if (plan->places[i].ligne != ligne_rangee)
        continue;  /* Pas sur la bonne rangée */

    /* Distance horizontale uniquement */
    int dx = abs(plan->places[i].colonne - fleche_x);

    /* Garder la place la plus proche horizontalement (±30 colonnes max) */
    if (dx < dist_min && dx <= 30)
    {
        dist_min = dx;
        place_trouvee = i;
    }
}
```

#### Changements clés

1. **Abandon de la distance verticale**: Plus de vérification dy
2. **Mapping strict par ligne**: `ligne_rangee = fleche_y - 1`
3. **Distance horizontale augmentée**: ±20 → ±30 colonnes pour couvrir toute la rangée
4. **Même logique pour ↓ et ↑**: Les deux flèches cherchent sur la même rangée

#### Placement du véhicule
```c
vehicule->posx = plan->places[place_trouvee].colonne - largeur / 2;
vehicule->posy = plan->places[place_trouvee].ligne + 1;  // Sous le ╦
vehicule->direction = (fleche_trouvee == L'↓') ? 'S' : 'N';
```

### Améliorations du système de debug

Ajout d'un système de logs fichier pour éviter l'interférence avec ncurses:

```c
// Fichier de debug
static FILE *debug_file = NULL;
#define DEBUG_LOG(...)                                      \
    do                                                      \
    {                                                       \
        if (!debug_file)                                    \
            debug_file = fopen("../debug_path.log", "a");  \
        if (debug_file)                                     \
        {                                                   \
            fprintf(debug_file, __VA_ARGS__);               \
            fflush(debug_file);                             \
        }                                                   \
    } while (0)
```

Messages de debug:
- `[PARK_ARROW]`: Flèche détectée avec position et ligne de rangée
- `[PARK_FOUND]`: Place trouvée avec distance horizontale
- `[PARK_FAIL]`: Aucune place libre sur la rangée
- `[SPAWN_PARK]`: Confirmation du parking automatique

### Fichiers modifiés

**src/mouvement.c**:
- Lignes 25-39: Ajout du système DEBUG_LOG
- Lignes 968-999: Réécriture complète de la logique de recherche de place
- Lignes 976-980: Remplacement fprintf(stderr) → DEBUG_LOG (3 occurrences)

### Résultats attendus

#### Avant (CASSÉ)
- Flèches ↑: fonctionnaient par chance sur certaines rangées
- Flèches ↓: NE FONCTIONNAIENT JAMAIS

#### Après (CORRIGÉ)
- Flèche à ligne Y trouve places à ligne Y-1
- ↓ et ↑ fonctionnent de la même manière (même rangée)
- Les véhicules se garent sur la bonne rangée horizontale
- Distance horizontale ±30 couvre toutes les places d'une rangée

### Tests de validation

Pour tester:
```bash
cd data
../bin/parking
# Appuyer sur ESPACE pour spawner des véhicules
# Observer les véhicules se garer automatiquement
# Vérifier ../debug_path.log pour les logs détaillés
```

Vérifications attendues:
1. Véhicules se garent sur les 3 rangées (lignes 12, 17, 22)
2. Flèches ↓ et ↑ fonctionnent toutes les deux
3. Pas de parkings sur la mauvaise rangée
4. Les 18 places peuvent être remplies

### Limitations actuelles (non implémentées)

1. **Pas de vérification de compatibilité directionnelle**:
   - Un véhicule allant vers l'Est peut se garer sur une place accessible depuis l'Ouest
   - Solution future: ajouter un champ `acces_direction` dans PlaceParking

2. **Pas de distinction sémantique ↓ vs ↑**:
   - Les deux flèches fonctionnent de la même manière
   - Seule la direction finale du véhicule change ('S' vs 'N')

3. **Pas de vérification de chemin accessible**:
   - Le système "téléporte" le véhicule sur la place
   - Pas de vérification qu'il existe un chemin roulable

### Prochaines étapes possibles

1. Ajouter détection du côté d'accès des places
2. Vérifier compatibilité direction véhicule / accès place
3. Implémenter une approche progressive au lieu de téléportation
4. Ajouter logs de statistiques (taux d'occupation, temps de parking)

---

**Date d'implémentation**: 2026-01-07
**Status**: ✅ ETAPE 3 COMPLÈTE
**Fichiers affectés**: src/mouvement.c
**Lignes modifiées**: ~50 lignes
