# RAPPORT TECHNIQUE - Simulateur de Parking

**Projet:** Gestionnaire de parking automatisé avec interface ncurses
**Auteur:** [Votre Nom]
**Date:** Janvier 2026
**Langage:** C
**Bibliothèque:** ncursesw (support UTF-8)

---

## Table des matières

1. [Vue d'ensemble](#1-vue-densemble)
2. [Architecture du code](#2-architecture-du-code)
3. [Modules critiques](#3-modules-critiques)
4. [Choix techniques](#4-choix-techniques)
5. [Points sensibles](#5-points-sensibles)
6. [Guide de lecture du code](#6-guide-de-lecture-du-code)
7. [Tests et validation](#7-tests-et-validation)
8. [Perspectives d'amélioration](#8-perspectives-damélioration)
9. [Annexes](#annexes)

---

## 1. Vue d'ensemble

### 1.1 Présentation du projet

Le simulateur de parking est un programme en C qui gère automatiquement un parking virtuel avec:
- **Navigation intelligente** des véhicules via un système de flèches
- **Parking automatique** sur détection de places libres
- **File d'attente** avec gestion des timeouts
- **Deux modes de difficulté** (Normal et Hard)
- **Interface graphique** complète via ncurses
- **Affichage en temps réel** des statistiques et du score

### 1.2 Fonctionnalités principales

| Fonctionnalité | Description |
|----------------|-------------|
| **Spawn adaptatif** | Génération de véhicules ajustée selon la file d'attente |
| **Navigation par flèches** | Système de direction basé sur des flèches UTF-8 dans le plan |
| **Parking automatique** | Détection et stationnement sur places libres (↑/↓) |
| **Système de score** | Points gagnés/perdus selon performances |
| **Viewport dynamique** | Affichage centré sur les véhicules actifs |
| **Détection de collision** | AABB (Axis-Aligned Bounding Box) avec tolérance |
| **File d'attente** | Jusqu'à 10 véhicules avec timeout configurable |

### 1.3 Technologies utilisées

- **Langage:** C (standard C11)
- **Bibliothèque graphique:** ncursesw (wide-character pour UTF-8)
- **Encodage:** UTF-8 pour les caractères spéciaux (box-drawing, flèches)
- **Build system:** Makefile GNU
- **Gestion de version:** Git
- **Framework de test:** Unity (tests unitaires)

---

## 2. Architecture du code

### 2.1 Cartographie des fichiers

| Fichier | Lignes | Rôle | Responsabilités |
|---------|--------|------|-----------------|
| **src/main.c** | 72 | Point d'entrée | Initialisation, sélection difficulté, nettoyage |
| **src/jeu.c** | 505 | Boucle de jeu | Spawn, file d'attente, timeouts, game loop |
| **src/mouvement.c** | 1886 | Navigation | Suivi de flèches, changement de direction, parking |
| **src/affichage.c** | 895 | Interface ncurses | Menus, HUD, rendu plan/véhicules, viewport |
| **src/plan.c** | 640 | Gestion plan | Chargement plan.txt, détection places/flèches |
| **src/liste_car.c** | 474 | Structures de données | Liste chaînée, file d'attente |
| **src/matrice.c** | 75 | Matrice d'occupation | Gestion mémoire matrice dynamique |
| **src/mouvement/sprites.c** | 131 | Sprites véhicules | Chargement carrosseries, orientation |
| **src/mouvement/collision.c** | 230 | Détection collision | AABB, vérifications déplacement |

**Total:** ~4908 lignes de code (hors tests et Unity)

### 2.2 Dépendances entre modules

```
main.c
├── affichage.c (initialisation ncurses)
├── plan.c (chargement plan.txt)
├── jeu.c (boucle principale)
│   ├── affichage.c (rendu)
│   ├── mouvement.c (déplacement véhicules)
│   │   ├── mouvement/sprites.c
│   │   └── mouvement/collision.c
│   ├── liste_car.c (gestion liste/file)
│   └── plan.c (occupation places)
└── liste_car.c (init liste)
```

### 2.3 Structures de données principales

#### 2.3.1 VEHICULE (liste_car.h)

```c
typedef struct voiture {
    int posx, posy;              // Position (coin haut-gauche)
    char direction;              // 'N', 'S', 'E', 'O'
    char etat;                   // '1' actif, '0' garé
    char Carrosserie[4][30];     // Sprite visuel (4 lignes)
    unsigned long int tps;       // Timestamp parking
    // ... autres champs
    struct voiture *NXT, *PRV;   // Liens liste doublement chaînée
} VEHICULE;
```

**Usage:** Chaque véhicule est un maillon d'une liste chaînée avec son sprite visuel.

#### 2.3.2 PlanParking (plan.h)

```c
typedef struct plan_parking {
    wchar_t plan_statique[MAX_HAUTEUR][MAX_LARGEUR];  // Plan UTF-8
    int largeur, hauteur;                               // Dimensions réelles

    // Places de parking
    PlaceParking places[MAX_PLACES];
    int places_totales, places_libres;

    // Flèches de circulation
    FlecheCirculation fleches[MAX_FLECHES];
    int nb_fleches;

    // Entrée/Sortie
    int entree_x, entree_y, sortie_x, sortie_y;
    int barriere_entree_ouverte, barriere_sortie_ouverte;

    // Scoring
    long score, high_score, argent_total;
    int vehicules_servis, vehicules_perdus;
    int difficulte;  // 0=NORMAL, 1=HARD
} PlanParking;
```

**Usage:** Structure centrale contenant tout l'état du parking.

#### 2.3.3 FileAttenteEntree (liste_car.h)

```c
typedef struct {
    VEHICULE *premier_attente;
    VEHICULE *dernier_attente;
    int longueur_attente;
    int longueur_max;  // Capacité max (10)
} FileAttenteEntree;
```

**Usage:** File FIFO pour les véhicules en attente d'entrée.

---

## 3. Modules critiques

### 3.1 Système de mouvement (mouvement.c + sous-modules)

#### 3.1.1 Anticipation des flèches

**Algorithme:**
```c
void suivre_fleches(VEHICULE *vehicule, PlanParking *plan) {
    // 1. Calculer le centre du véhicule
    int centre_x = vehicule->posx + largeur/2;
    int centre_y = vehicule->posy + hauteur/2;

    // 2. Anticiper 4 cellules devant
    int anticipation = 4;

    // 3. Trouver la flèche LA PLUS PROCHE (distance Manhattan)
    for (chaque flèche dans plan->fleches) {
        int dist = abs(fx - centre_x) + abs(fy - centre_y);
        if (est_devant && dist < meilleure_distance) {
            meilleur_index = i;
            meilleure_distance = dist;
        }
    }

    // 4. Appliquer la direction de la flèche la plus proche
    if (meilleur_index >= 0) {
        vehicule->direction = plan->fleches[meilleur_index].direction_sortie;
        orienter_carrosserie(vehicule);  // Recharger sprite
    }
}
```

**Pourquoi 4 cellules ?**
- Trop petit (2-3) : les voitures ratent les virages
- Trop grand (6+) : détections multiples, hésitations
- 4 cellules = compromis optimal observé lors des tests

#### 3.1.2 Détection de collision (AABB)

**Principe:** Axis-Aligned Bounding Box avec tolérance de 2 cellules.

```c
int vehicules_en_collision(VEHICULE *v1, VEHICULE *v2) {
    const int TOLERANCE = 2;

    // Réduire les bounding boxes pour éviter faux positifs
    int x1_min = v1->posx + TOLERANCE;
    int x1_max = v1->posx + largeur1 - TOLERANCE;
    // ... idem pour y et v2

    // Test de séparation
    int separated = (x1_max <= x2_min) ||  // v1 à gauche de v2
                    (x2_max <= x1_min) ||  // v2 à gauche de v1
                    (y1_max <= y2_min) ||  // v1 au-dessus de v2
                    (y2_max <= y1_min);    // v2 au-dessus de v1

    return !separated;  // Collision si PAS séparés
}
```

**Tolérance de 2 cellules:** Permet aux voitures de se "frôler" sans collision, évite les faux positifs aux intersections.

#### 3.1.3 Parking automatique

**Détection:**
1. Scanner zone ±3 cellules autour du centre du véhicule
2. Chercher flèche ↑ ou ↓ (flèches de parking)
3. Vérifier que direction véhicule = direction flèche
4. Chercher place libre sur la rangée (ligne Y-1)
5. Téléporter véhicule sur la place la plus proche

**Code clé:**
```c
if (c == L'↑' || c == L'↓') {
    char dir_parking = (c == L'↓') ? 'S' : 'N';
    if (vehicule->direction == dir_parking) {
        // Chercher place libre sur ligne_rangee = fleche_y - 1
        // Téléporter: vehicule->posx = place.colonne - largeur/2
        //             vehicule->posy = place.ligne + 1
        marquer_place_occupee(plan, place_trouvee);
        vehicule->etat = '0';  // Désactiver
    }
}
```

### 3.2 Système d'affichage (affichage.c)

#### 3.2.1 ncurses et UTF-8

**Problème:** ncurses standard ne gère pas bien l'UTF-8.

**Solution:**
- Utiliser `ncursesw` (wide-character)
- Appeler `setlocale(LC_ALL, "")` dans main() **AVANT** initscr()
- Utiliser `addstr()` au lieu de `printw()` pour éviter les corruptions

**Configuration critique:**
```c
void initialiser_affichage() {
    initscr();                   // Init ncurses
    raw();                       // Pas de buffering
    noecho();                    // Pas d'écho clavier
    curs_set(0);                 // Curseur invisible
    keypad(stdscr, TRUE);        // Touches spéciales
    nodelay(stdscr, TRUE);       // Input non-bloquant

    start_color();               // Activer couleurs
    init_pair(COLOR_PAIR_ROUGE, COLOR_RED, COLOR_BLACK);
    // ... 8 autres paires
}
```

#### 3.2.2 Viewport dynamique

**Principe:** Centrer l'affichage sur les véhicules actifs.

```c
void calculer_viewport(PlanParking *plan, l_car *vehicules, Viewport *viewport) {
    // 1. Calculer centre de gravité des véhicules actifs
    int centre_x = 0, centre_y = 0, count = 0;
    for (chaque véhicule actif) {
        centre_x += vehicule->posx;
        centre_y += vehicule->posy;
        count++;
    }
    if (count > 0) {
        centre_x /= count;
        centre_y /= count;
    }

    // 2. Centrer viewport sur ce point
    centrer_viewport_sur_zone(centre_x, centre_y, ...);
}
```

**Avantage:** Suit automatiquement l'action, même sur grand plan.

### 3.3 Boucle de jeu (jeu.c)

#### 3.3.1 Spawn adaptatif

**Idée:** Ralentir le spawn si file d'attente se remplit.

```c
int calculer_spawn_interval(FileAttenteEntree *file, int difficulte) {
    int longueur = file->longueur_attente;

    if (difficulte == HARD) {
        if (longueur <= 3) return 20;   // 2s
        if (longueur <= 6) return 35;   // 3.5s
        return 55;                      // 5.5s
    } else {  // NORMAL
        if (longueur <= 3) return 30;   // 3s
        if (longueur <= 6) return 50;   // 5s
        return 80;                      // 8s
    }
}
```

**Résultat:** Évite la surcharge, donne du temps au joueur.

#### 3.3.2 Système de timeout

**Principe:** Véhicules en attente trop longtemps → pénalité + retrait.

```c
void traiter_timeout_attente(...) {
    unsigned long timeout = plan->difficulte ? TIMEOUT_HARD : TIMEOUT_NORMAL;
    unsigned long penalite = plan->difficulte ? PENALITE_HARD : PENALITE_NORMAL;

    for (chaque véhicule dans file_attente) {
        if (frame - v->temps_attente >= timeout) {
            // Retirer de la file
            // Appliquer pénalité au score
            plan->score -= penalite;
            plan->vehicules_perdus++;
            detruire_vehicule(&v);
        }
    }
}
```

---

## 4. Choix techniques

### 4.1 Structures de données

#### Liste chaînée vs tableau dynamique

**Choix:** Liste doublement chaînée pour les véhicules.

**Avantages:**
- Insertion/suppression O(1) en tête/queue
- Pas de réallocation coûteuse
- Taille dynamique (nombre de véhicules variable)

**Inconvénients:**
- Accès aléatoire O(n)
- Overhead mémoire (2 pointeurs par élément)

**Justification:** Le nombre de véhicules est faible (~10-20), les insertions/suppressions fréquentes. La liste chaînée est adaptée.

#### Matrice statique vs dynamique

**Choix:** Matrice statique de `wchar_t` pour le plan.

```c
wchar_t plan_statique[MAX_HAUTEUR][MAX_LARGEUR];  // 100 x 200
```

**Raison:** Dimensions connues à la compilation, évite allocation dynamique complexe pour wchar_t.

### 4.2 Algorithmes

#### Pathfinding simplifié

**Pas de A\*** ou Dijkstra, pourquoi ?

Le plan contient **déjà** les chemins via les flèches. L'algorithme suit simplement:
1. Détecter flèche la plus proche devant
2. Prendre sa direction
3. Répéter

**Avantage:** Simplicité, performance O(n) où n = nombre de flèches (~50).

#### Détection AABB optimisée

**Pourquoi AABB et pas pixel-perfect ?**

- AABB = rectangles alignés sur axes → calcul ultra-rapide
- Tolérance de 2 cellules → évite faux positifs
- Suffisant pour un jeu avec grille

**Complexité:** O(1) par paire de véhicules, O(n²) pour tous (n petit).

---

## 5. Points sensibles

### 5.1 Code à NE PAS modifier

#### Initialisation ncurses (main.c)

```c
setlocale(LC_ALL, "");          // UTF-8
initialiser_affichage();        // APRÈS setlocale!
```

**Si modifié:** Les caractères UTF-8 s'affichent en `�`.

#### Chargement UTF-8 du plan (plan.c)

```c
fgets(ligne, sizeof(ligne), fichier);
mbstowcs(plan->plan_statique[i], ligne, MAX_LARGEUR);
```

**Fragile:** Nécessite `setlocale()` appelé avant, sinon corruption.

#### Logique de collision stricte (mouvement/collision.c)

Tolérance TOLERANCE=2 finement ajustée. Modifier → faux positifs ou collisions manquées.

### 5.2 Zones critiques

#### Détection de flèches (mouvement.c:1092)

```c
void suivre_fleches(VEHICULE *vehicule, PlanParking *plan)
```

**Récemment corrigée:** Passée de "première flèche trouvée" à "flèche la plus proche". **Ne pas réintroduire** l'ancienne logique.

#### Spawn avec vérification (jeu.c:traiter_entree_vehicules)

**Vérifications critiques:**
1. Barrière ouverte
2. File non vide
3. Places libres > 0
4. Entrée libre (pas de véhicule à proximité)

**Ordre important:** Si changé, spawn buggé.

---

## 6. Guide de lecture du code

### 6.1 Par où commencer ?

**Pour comprendre le flux:**
1. `main.c` → Voir initialisation, appel à `executer_boucle_jeu()`
2. `jeu.c:executer_boucle_jeu()` → Boucle principale
3. `mouvement.c:deplacer_tous_vehicules()` → Déplacement

**Pour comprendre les structures:**
1. `plan.h` → PlanParking, PlaceParking, FlecheCirculation
2. `liste_car.h` → VEHICULE, l_car, FileAttenteEntree

**Pour comprendre l'affichage:**
1. `affichage.c:initialiser_affichage()` → Config ncurses
2. `affichage.c:afficher_plan_avec_viewport()` → Rendu

### 6.2 Ordre de lecture recommandé

**Niveau 1 (structures):**
- `include/plan.h`
- `include/liste_car.h`
- `include/matrice.h`

**Niveau 2 (init et main):**
- `src/main.c`
- `src/plan.c` (chargement plan.txt)
- `src/liste_car.c` (liste chaînée)

**Niveau 3 (logique métier):**
- `src/jeu.c` (boucle, spawn, timeouts)
- `src/mouvement.c` (navigation, parking)
- `src/mouvement/collision.c` (détection)
- `src/mouvement/sprites.c` (carrosseries)

**Niveau 4 (affichage):**
- `src/affichage.c` (ncurses, menus, HUD)

---

## 7. Tests et validation

### 7.1 Scénarios de test

| Scénario | Procédure | Résultat attendu |
|----------|-----------|------------------|
| **Parking complet** | Laisser tourner jusqu'à 0 places libres | Spawn s'arrête, timeout déclenche |
| **Collision volontaire** | Fermer barrière entrée alors que voiture arrive | GAME OVER + écran de stats |
| **Sortie des véhicules** | Attendre que véhicules garés sortent | Places libérées, score augmente |
| **Mode HARD** | Sélectionner difficulté Hard | Spawn plus rapide, timeout plus court |
| **File d'attente pleine** | Ne pas ouvrir barrière entrée | File se remplit, spawn ralentit |

### 7.2 Vérification de non-régression

**Checkpoints fonctionnels:**
- ✅ Menu difficulté s'affiche
- ✅ Plan se charge sans corruption UTF-8
- ✅ Voitures tournent aux flèches
- ✅ Parking automatique fonctionne
- ✅ Barrières ouvrent/ferment (E/S)
- ✅ Score s'incrémente
- ✅ Collision détectée → game over

---

## 8. Perspectives d'amélioration

### 8.1 Optimisations possibles

1. **Cache de flèches:** Précalculer les plus proches pour chaque cellule → réduire recherche O(n) à O(1)
2. **Quadtree spatial:** Diviser le plan en zones → détection de collision O(log n)
3. **Thread séparé:** Affichage dans un thread, logique dans un autre → 120 FPS au lieu de 60

### 8.2 Fonctionnalités futures

- **Niveaux multiples:** Charger différents plans (plan2.txt, plan3.txt)
- **Types de véhicules:** Camions (2x plus lents), motos (2x plus rapides)
- **Power-ups:** Bonus temporaires (spawn freeze, score x2)
- **Sauvegarde:** Sauvegarder high score dans un fichier

### 8.3 Limites actuelles

- **Plan fixe:** Dimensions MAX_HAUTEUR=100 x MAX_LARGEUR=200 en dur
- **Pas de réseau:** Jeu local uniquement
- **Pas de son:** Bibliothèque ncurses sans audio

---

## Annexes

### Glossaire des termes

| Terme | Définition |
|-------|------------|
| **AABB** | Axis-Aligned Bounding Box - Rectangle de collision aligné sur les axes |
| **ncurses** | Bibliothèque C pour interfaces texte (TUI) |
| **Wide-character** | Support caractères multi-octets (UTF-8) |
| **Viewport** | Zone visible de l'écran (fenêtre d'affichage) |
| **Spawn** | Génération/création d'une entité (véhicule) |
| **Lane-lock** | Verrou de maintien de voie (anti-oscillation) |
| **Manhattan distance** | Distance `|x1-x2| + |y1-y2|` sur grille |

### Convention de nommage

- **Fonctions:** `snake_case` (ex: `calculer_viewport`)
- **Variables:** `snake_case` (ex: `centre_x`, `largeur`)
- **Structures:** `PascalCase` (ex: `PlanParking`, `VEHICULE`)
- **Constantes:** `UPPER_SNAKE_CASE` (ex: `MAX_HAUTEUR`, `SPAWN_RAPIDE`)
- **Macros:** `UPPER_SNAKE_CASE` (ex: `COLOR_PAIR_ROUGE`)

### Commandes essentielles

```bash
# Compilation
make                 # Compiler le projet
make clean           # Nettoyer les .o
make run             # Compiler et lancer

# Tests
make test            # Tests unitaires
make test-encodage   # Test UTF-8
make test-ncurses    # Test interactif ncurses

# Vérification
make info            # Infos de compilation
git log --oneline    # Historique commits
wc -l src/*.c        # Compter lignes de code
```

### Bibliographie

- **ncurses Programming HOWTO** - https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
- **The C Programming Language** - Kernighan & Ritchie
- **UTF-8 and Unicode** - https://utf8everywhere.org/
- **Game Programming Patterns** - Robert Nystrom

---

**Fin du rapport - Version 1.0 - Janvier 2026**
