# 🚗 Simulateur de Parking - Projet C

[![Version](https://img.shields.io/badge/version-1.5-blue.svg)](https://github.com)
[![C](https://img.shields.io/badge/language-C-brightgreen.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![License](https://img.shields.io/badge/license-Academic-orange.svg)](LICENSE)

Simulateur interactif de gestion de parking développé en C avec ncurses pour l'affichage terminal. Le projet simule un parking complet avec véhicules autonomes, file d'attente, système de scoring et deux modes de difficulté.

---

## 📋 Table des matières

- [Fonctionnalités](#-fonctionnalités)
- [Architecture](#-architecture)
- [Prérequis](#-prérequis)
- [Installation](#-installation)
- [Utilisation](#-utilisation)
- [Tests](#-tests)
- [Structure du projet](#-structure-du-projet)
- [Documentation](#-documentation)
- [Auteurs](#-auteurs)

---

## ✨ Fonctionnalités

### Gameplay
- 🎮 **Jeu interactif** : Contrôle des barrières entrée/sortie avec clavier
- 🚦 **Deux modes de difficulté** : Normal (débutant) et Hard (expert)
- 🏆 **Système de scoring** : Points gagnés par véhicule stationné
- 💰 **Gestion financière** : Revenus du parking (affichés en euros)
- ⏱️ **File d'attente** : Gestion du timeout des véhicules en attente
- 📊 **HUD complet** : Statistiques en temps réel (score, argent, places, véhicules)

### Simulation
- 🚗 **Véhicules autonomes** : Déplacement automatique avec IA
- 🅿️ **Parking automatique** : Recherche intelligente de places libres
- 🎯 **Détection de collision** : AABB (Axis-Aligned Bounding Box)
- 🔄 **Suivi de flèches** : Navigation directionnelle dans le parking
- 🚧 **Barrières intelligentes** : Contrôle entrée/sortie avec bornes

### Affichage
- 🎨 **Interface ncurses** : Affichage terminal optimisé avec couleurs
- 🌐 **Support UTF-8** : Caractères Unicode pour un rendu élégant
- 📺 **Viewport dynamique** : Caméra suivant les véhicules actifs
- 🖼️ **Sprites ASCII** : Modèles visuels 4x4 pour les voitures
- 🎭 **Orientation visuelle** : Voitures tournent dans la bonne direction

---

## 🏗️ Architecture

Le projet suit une **architecture modulaire** avec séparation claire des responsabilités :

```
Modules principaux :
├── main.c              - Point d'entrée (initialisation)
├── jeu.c               - Boucle de jeu et spawn adaptatif
├── mouvement.c         - Déplacement, collision, flèches
├── affichage.c         - Rendu ncurses avec viewport
├── plan.c              - Chargement et gestion du plan
├── liste_car.c         - Liste chaînée de véhicules
└── matrice.c           - Matrice d'occupation binaire

Sous-modules :
├── mouvement/
│   ├── sprites.c       - Chargement des modèles visuels
│   └── collision.c     - Détection AABB optimisée
```

**Voir [REPORT.md](REPORT.md) pour l'architecture détaillée.**

---

## 🔧 Prérequis

### Système
- **OS** : Linux / WSL / macOS
- **Compilateur** : GCC (version 7.0+)
- **Bibliothèque** : ncursesw (wide-character version)
- **Terminal** : Support UTF-8 (90% des terminaux modernes)

### Installation des dépendances

#### Ubuntu / Debian / WSL
```bash
sudo apt update
sudo apt install build-essential libncursesw5-dev
```

#### macOS
```bash
brew install ncurses
```

#### Fedora / RHEL
```bash
sudo dnf install ncurses-devel gcc make
```

---

## 🚀 Installation

### 1. Cloner le dépôt
```bash
git clone https://github.com/genwox/projetC_Amine_Damien.git
cd ProjetC
```

### 2. Compiler le projet
```bash
make
```

### 3. Lancer le jeu
```bash
make run
```

---

## 🎮 Utilisation

### Lancer le simulateur
```bash
./bin/parking
```

### Contrôles du jeu

| Touche | Action |
|--------|--------|
| `e` | Ouvrir/fermer barrière d'entrée |
| `s` | Ouvrir/fermer barrière de sortie |
| `q` | Quitter le jeu |

### Déroulement du jeu

1. **Menu de démarrage** : Choisir la difficulté (Normal / Hard)
2. **Spawn de véhicules** : Les voitures apparaissent automatiquement à l'entrée
3. **File d'attente** : Les véhicules attendent l'ouverture de la barrière (touche `e`)
4. **Déplacement autonome** : Les voitures suivent les flèches et cherchent une place
5. **Parking automatique** : Une fois garées, elles restent 5-10 secondes
6. **Sortie** : Les voitures sortent en suivant les flèches vers la barrière de sortie (touche `s`)
7. **Scoring** : +100 points et +5,00€ par véhicule servi avec succès

### Modes de difficulté

#### Mode Normal (débutant)
- Spawn : 1 voiture toutes les 6 secondes
- Timeout : 10 secondes en file d'attente
- Adapté aux nouveaux joueurs

#### Mode Hard (expert)
- Spawn : 1 voiture toutes les 2 secondes
- Timeout : 5 secondes en file d'attente
- Rythme rapide, réflexes requis

---

## 🧪 Tests

### Tests unitaires (Unity framework)
```bash
make test              # Exécute tous les tests
make tests             # Compile tous les tests
```

### Tests individuels
```bash
make test-encodage     # Vérification UTF-8
make test-ncurses      # Test interactif ncurses
make test-final        # Test d'intégration complet
```

### Résultats attendus
- ✅ **0 erreur de compilation**
- ✅ **0 warning**
- ✅ **Tests Unity : 100% PASS**

---

## 📁 Structure du projet

```
ProjetC/
├── src/                    # Code source (.c)
│   ├── main.c              # Point d'entrée (102 lignes)
│   ├── jeu.c               # Boucle principale (764 lignes)
│   ├── mouvement.c         # Mouvement et IA (2396 lignes)
│   ├── affichage.c         # Rendu ncurses (895 lignes)
│   ├── plan.c              # Gestion plan (494 lignes)
│   ├── liste_car.c         # Liste chaînée (474 lignes)
│   ├── matrice.c           # Matrice binaire (75 lignes)
│   └── mouvement/          # Sous-modules mouvement
│       ├── sprites.c       # Chargement sprites
│       └── collision.c     # Détection AABB
│
├── include/                # Headers (.h)
│   ├── jeu.h
│   ├── mouvement.h
│   ├── affichage.h
│   ├── plan.h
│   ├── liste_car.h
│   ├── matrice.h
│   └── mouvement/
│       ├── sprites.h
│       └── collision.h
│
├── data/                   # Données du jeu
│   ├── plan.txt            # Plan du parking (UTF-8)
│   └── sprites/            # Modèles de véhicules
│       ├── voiture.txt
│       ├── camion.txt
│       └── suv.txt
│
├── bin/                    # Exécutables (généré)
│   └── parking             # Programme principal
│
├── obj/                    # Fichiers objets (généré)
│   └── *.o
│
├── tests/                  # Tests unitaires
│   ├── test_liste.c
│   ├── test_encodage.c
│   └── test_final.c
│
├── Unity/                  # Framework de tests (submodule)
│
├── Makefile                # Configuration build
├── README.md               # Ce fichier
└── REPORT.md               # Rapport technique détaillé
```

**Lignes de code totales** : ~4674 lignes (hors Unity)

---

## 📚 Documentation

### Pour les développeurs

- **[REPORT.md](REPORT.md)** : Rapport technique complet (1477 lignes)
  - Architecture détaillée
  - Algorithmes critiques
  - Guide de lecture du code
  - Choix techniques
  - Perspectives d'amélioration
  - Chronologie du développement

### Pour la soutenance

- **[SOUTENANCE.md](SOUTENANCE.md)** : Guide complet de préparation à la soutenance
  - Pitch 5 minutes structuré
  - 43 questions techniques du jury avec réponses détaillées
  - Stratégies pour éviter les pièges
  - Basé sur l'analyse approfondie du code

### Pour les utilisateurs

- Ce README : Guide d'installation et d'utilisation
- Commentaires inline dans les headers (`.h`)
- Makefile avec `make help` pour les commandes disponibles

---

## 🔍 Commandes Makefile

| Commande | Description |
|----------|-------------|
| `make` | Compile le projet |
| `make run` | Compile et exécute le simulateur |
| `make clean` | Nettoie les fichiers objets |
| `make distclean` | Nettoyage complet (obj + bin) |
| `make tests` | Compile tous les tests |
| `make test` | Exécute les tests unitaires |
| `make test-encodage` | Test UTF-8 |
| `make test-ncurses` | Test ncurses interactif |
| `make info` | Affiche les infos de compilation |
| `make help` | Affiche l'aide |

---

## 🐛 Dépannage

### Les caractères s'affichent en `�`
**Cause** : Encodage UTF-8 non activé

**Solution** :
```bash
export LANG=fr_FR.UTF-8
export LC_ALL=fr_FR.UTF-8
./bin/parking
```

### Erreur `undefined reference to addstr`
**Cause** : ncursesw non installé ou non lié

**Solution** :
```bash
sudo apt install libncursesw5-dev  # Ubuntu/Debian
make clean && make
```

### Le terminal est trop petit
**Cause** : Le plan nécessite au moins 80x40 caractères

**Solution** : Agrandir la fenêtre du terminal ou utiliser F11 (plein écran)

### Compilation échoue avec `fatal error: ncurses.h`
**Cause** : Bibliothèque ncurses manquante

**Solution** : Installer les dépendances (voir [Prérequis](#-prérequis))

---

## 🎓 Contexte académique

### Objectif pédagogique
Projet de fin d'année en langage C visant à maîtriser :
- Gestion mémoire dynamique (malloc/free)
- Structures de données (listes chaînées, matrices)
- Programmation modulaire (séparation .h/.c)
- Bibliothèque système (ncurses)
- Gestion de projet (Makefile, git)

### Compétences développées
- ✅ Algorithmique (pathfinding, collision)
- ✅ Programmation événementielle (boucle de jeu)
- ✅ Débogage (gdb, valgrind)
- ✅ Documentation technique
- ✅ Travail en équipe (git, revue de code)

---

## 👥 Auteurs

- **Amine** - Développement principal
- **Damien** - Développement principal

Projet réalisé dans le cadre du cours de **Programmation en C** - ESIEA 2024-2025

---

## 📜 Licence

Projet académique - **ESIEA 2025**

Tous droits réservés dans le cadre du règlement universitaire.

---

## 🙏 Remerciements

- **Équipe pédagogique ESIEA** pour l'encadrement et les conseils
- **Communauté ncurses** pour la documentation complète
- **Unity Test Framework** pour les tests unitaires
- **Communauté open source** pour les ressources et tutoriels

---

## 📞 Contact

Pour toute question ou suggestion :
- **GitHub Issues** : [Créer une issue](https://github.com/genwox/projetC_Amine_Damien/issues)
- **Email** : Via plateforme ESIEA

---

**Version** : 1.5-stable
**Dernière mise à jour** : Janvier 2026
**Statut** : ✅ Production Ready

---

## 🚀 Quick Start

```bash
# Installation rapide (Ubuntu/Debian/WSL)
sudo apt install build-essential libncursesw5-dev
git clone https://github.com/genwox/projetC_Amine_Damien.git
cd ProjetC
make run

# Contrôles : e (entrée), s (sortie), q (quitter)
```

**Bon jeu ! 🎮**
