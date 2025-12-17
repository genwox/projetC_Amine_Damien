# 🚗 Simulateur de Parking - ESIEA

Projet en C pour simuler la gestion d'un parking avec affichage en console.

## 📁 Structure du projet

```
ProjetC/
├── src/              # Fichiers sources (.c)
│   ├── main.c        # Point d'entrée du programme
│   ├── affichage.c   # Gestion de l'affichage et des couleurs
│   ├── liste_car.c   # Liste chaînée des véhicules
│   ├── matrice.c     # Matrice d'occupation
│   └── plan.c        # Gestion du plan du parking
│
├── include/          # Fichiers d'en-tête (.h)
│   ├── affichage.h
│   ├── liste_car.h
│   ├── matrice.h
│   └── plan.h
│
├── data/             # Fichiers de données
│   ├── plan.txt           # Plan du parking (Unicode)
│   ├── plan_ascii.txt     # Plan ASCII (backup)
│   ├── car.txt            # Modèle voiture
│   ├── camion.txt         # Modèle camion
│   └── suv.txt            # Modèle SUV
│
├── bin/              # Exécutables compilés
│   └── parking       # Programme principal
│
├── obj/              # Fichiers objets (.o)
│
├── tests/            # Tests unitaires
│   ├── test_liste.c
│   ├── test_encodage.c
│   └── test_final.c
│
├── Unity/            # Framework de tests unitaires
│
├── Makefile          # Configuration de compilation
└── README.md         # Cette documentation
```

## 🔧 Prérequis

- **Compilateur** : GCC
- **OS** : Linux / WSL / macOS
- **Terminal** : Supportant UTF-8 et les codes ANSI

## 🚀 Compilation et exécution

### Compiler le projet

```bash
make
```

### Exécuter le programme

```bash
make run
```

Ou directement :

```bash
./bin/parking
```

### Nettoyer les fichiers compilés

```bash
make clean        # Nettoie les fichiers objets et exécutables
make distclean    # Nettoie complètement (y compris répertoires)
```

## 🧪 Tests

### Compiler tous les tests

```bash
make tests
```

### Exécuter les tests unitaires

```bash
make test
```

### Tests d'encodage UTF-8

```bash
make test-encodage
```

### Test final

```bash
make test-final
```

## 📋 Commandes disponibles

| Commande           | Description                              |
|--------------------|------------------------------------------|
| `make`             | Compile le projet                        |
| `make run`         | Compile et exécute le programme          |
| `make tests`       | Compile tous les tests                   |
| `make test`        | Exécute les tests unitaires              |
| `make clean`       | Nettoie les fichiers compilés            |
| `make distclean`   | Nettoie complètement                     |
| `make info`        | Affiche les informations de compilation  |
| `make help`        | Affiche l'aide                           |

## 🎨 Fonctionnalités

### Affichage
- ✅ Support complet UTF-8 avec beaux caractères Unicode (`════`)
- ✅ Couleurs ANSI pour une meilleure lisibilité
- ✅ Affichage optimisé (mise à jour par zones)
- ✅ Plan du parking en vue aérienne

### Gestion du parking
- Places de parking (libres/occupées)
- Bornes d'entrée/sortie
- Barrières automatiques
- Allées de circulation
- Compteur de places disponibles

### Véhicules
- Liste chaînée de véhicules
- Différents types : voitures, camions, SUV
- États : actif/inactif

## 🔧 Corrections appliquées (encodage UTF-8)

### Problème initial
Les caractères Unicode (`════`) s'affichaient comme `�` à cause de :
1. ❌ Absence de `setlocale(LC_ALL, "")`
2. ❌ Buffer trop petit (200 bytes au lieu de 600)
3. ❌ Stockage caractère par caractère coupant les multi-byte UTF-8

### Solutions appliquées

#### 1. Configuration locale (main.c)
```c
#include <locale.h>

int main()
{
    setlocale(LC_ALL, "");  // Active le support UTF-8
    // ...
}
```

#### 2. Augmentation du buffer (plan.h)
```c
#define MAX_LIGNE 600  // Au lieu de 200
```

#### 3. Fonction d'affichage Unicode (plan.c)
```c
void afficher_plan_unicode(const char* fichier_plan)
{
    // Lit et affiche le fichier ligne par ligne
    // Supporte UTF-8 car on ne coupe pas les caractères
}
```

## 📝 Fichiers modifiés

### Fichiers sources
- ✅ `main.c` - Ajout `setlocale()` + `afficher_plan_unicode()`
- ✅ `affichage.c` - Suppression fonction dupliquée
- ✅ `affichage.h` - Ajout `TAILLE_PLAN`
- ✅ `plan.c` - Ajout `afficher_plan_unicode()`
- ✅ `plan.h` - `MAX_LIGNE` à 600, constantes `BARRIERE_*`

### Configuration
- ✅ `Makefile` - Structure organisée avec répertoires

### Données
- ✅ `data/plan.txt` - Version Unicode restaurée
- ✅ `data/plan_ascii.txt` - Version ASCII (backup)

## 🐛 Dépannage

### Les caractères Unicode ne s'affichent pas
```bash
# Vérifier la locale du système
locale

# Vérifier l'encodage du terminal
echo $LANG  # Doit contenir "UTF-8"

# Si nécessaire, exporter UTF-8
export LANG=fr_FR.UTF-8
```

### Utiliser la version ASCII
Si les caractères Unicode posent problème :
```bash
mv data/plan.txt data/plan_unicode.txt
mv data/plan_ascii.txt data/plan.txt
```

## 👥 Auteurs

Projet réalisé dans le cadre du cours de C - ESIEA

## 📄 Licence

Projet académique - ESIEA 2024-2025
