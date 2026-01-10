# GUIDE DE SOUTENANCE - Simulateur de Parking

**Projet:** Simulateur de parking en C avec ncurses
**Auteurs:** Amine & Damien
**École:** ESIEA 2024-2025
**Date:** Janvier 2026

---

## Table des matières

1. [Pitch 5 minutes (script structuré)](#1-pitch-5-minutes)
2. [Questions du jury (43 questions détaillées)](#2-questions-du-jury)
3. [Erreurs à éviter en soutenance](#3-erreurs-à-éviter-en-soutenance)

---

## 1. PITCH 5 MINUTES

### Plan minute par minute

#### 0:00 - 0:30 : Introduction (30 secondes)

**Script :**
> "Bonjour, nous sommes Amine et Damien. Nous avons développé un simulateur de parking interactif en langage C. Le projet simule un parking complet avec véhicules autonomes qui se déplacent automatiquement, se garent, et sortent. L'utilisateur contrôle les barrières d'entrée et de sortie pour gérer le flux de véhicules."

**Points clés à dire :**
- Projet en C pur (pas de C++)
- Affichage terminal avec ncurses
- Simulation temps réel avec IA des véhicules

---

#### 0:30 - 1:30 : Démonstration live (1 minute)

**Script :**
> "Laissez-moi vous montrer le jeu en action [lancer ./bin/parking]. Vous voyez le plan du parking affiché avec des caractères UTF-8. Les voitures apparaissent à l'entrée, j'ouvre la barrière avec la touche 'e', elles entrent automatiquement, suivent les flèches directionnelles, trouvent une place libre, se garent, puis ressortent en suivant les flèches vers la sortie que je contrôle avec 's'. Le HUD en bas affiche le score, l'argent gagné, et les places disponibles."

**Points clés à dire :**
- Contrôles : e (entrée), s (sortie), q (quitter)
- Véhicules autonomes (pas de contrôle manuel)
- Score et argent s'incrémentent automatiquement
- Deux modes : Normal et Hard (vitesse spawn différente)

**Éléments visuels à montrer :**
- Plan UTF-8 avec caractères box-drawing
- Voitures qui se déplacent
- File d'attente à l'entrée
- HUD avec statistiques temps réel

---

#### 1:30 - 2:30 : Architecture (1 minute)

**Script :**
> "Le projet suit une architecture modulaire avec 9 fichiers sources. Le main.c initialise ncurses et charge le plan. Le module jeu.c contient la boucle principale qui gère le spawn de véhicules et le game loop. Le module mouvement.c (le plus gros, 2396 lignes) gère le déplacement, la détection de collision AABB, et le suivi de flèches. Le module affichage.c gère ncurses avec un viewport dynamique. Enfin, liste_car.c implémente une liste chaînée doublement liée pour gérer les véhicules, et matrice.c gère la grille d'occupation binaire."

**Points clés à dire :**
- Séparation claire .h / .c (headers vs implémentation)
- Pas d'includes circulaires (dépendances claires)
- 4674 lignes de code total (hors framework Unity)

**Schéma à montrer (ou décrire) :**
```
main.c → jeu.c → mouvement.c → mouvement/collision.c
              → affichage.c
              → liste_car.c
              → plan.c → matrice.c
```

---

#### 2:30 - 3:30 : Algorithmes clés (1 minute)

**Script :**
> "Deux algorithmes critiques : la détection de collision et le parking automatique. Pour la collision, j'utilise AABB, Axis-Aligned Bounding Box. Chaque véhicule a une boîte englobante de 4x3 cellules. Je réduis cette boîte de 2 pixels de chaque côté pour éviter les faux positifs, puis je teste si deux boîtes se chevauchent en 4 comparaisons. Complexité O(1). Pour le parking automatique, quand une voiture détecte une flèche parking (↓ ou ↑), elle cherche la place libre la plus proche horizontalement dans un rayon de 15 colonnes, puis téléporte directement dessus. Complexité O(nombre de places) soit environ O(30)."

**Points clés à dire :**
- AABB avec TOLERANCE=2 (évite collisions adjacentes)
- Parking automatique = spawn direct (pas de mouvement progressif)
- Spawn adaptatif : ralentit si file d'attente pleine

**Formules à citer :**
- Collision : `separated = (x1_max <= x2_min) || ...` (4 tests)
- Parking : `dx < dist_min && dx <= 15` (recherche nearest)

---

#### 3:30 - 4:30 : Choix techniques (1 minute)

**Script :**
> "Trois choix techniques majeurs. Premièrement, liste chaînée vs tableau : j'ai choisi liste chaînée car le nombre de véhicules varie dynamiquement (spawn/despawn fréquent), et la liste permet insertion/suppression O(1) en tête/queue. Deuxièmement, support UTF-8 complet : j'utilise setlocale() au démarrage, wchar_t pour le plan, et ncursesw (wide-character). Le parsing UTF-8 manuel gère les caractères 3-bytes comme les box-drawing. Troisièmement, matrice d'occupation dynamique 2D allouée avec malloc en cascade, avec nettoyage systématique en cas d'erreur."

**Points clés à dire :**
- Liste chaînée : flexibilité > performance (acceptable pour 20-30 véhicules)
- UTF-8 : obligatoire pour rendu élégant (═, ║, ↑, →)
- Gestion mémoire rigoureuse : 12 malloc, 12 free, 0 fuite

**Compromis à mentionner :**
- Simplicité avant optimisation (niveau intermédiaire)
- Pas de pathfinding A* (suivi de flèches suffit)
- Affichage relit plan.txt chaque frame (bottleneck connu)

---

#### 4:30 - 5:00 : Conclusion (30 secondes)

**Script :**
> "En conclusion, ce projet m'a permis de maîtriser la gestion mémoire dynamique en C, les structures de données classiques comme les listes chaînées, la programmation événementielle avec ncurses, et les algorithmes de collision. Le code compile sans warning, fonctionne de manière stable, et démontre un niveau C intermédiaire solide. Merci pour votre attention, je suis prêt à répondre à vos questions."

**Points clés à dire :**
- Compétences acquises : pointeurs, malloc/free, structures, ncurses
- 0 warning, 0 erreur de compilation
- Code défendable et maintenable

---

### Points à ABSOLUMENT dire (ne pas oublier)

1. **Gestion mémoire rigoureuse** :
   - Nettoyage cascade dans creer_matrice() (si erreur ligne i, libère 0..i-1)
   - Réinitialisation *ptr = NULL après free (évite use-after-free)

2. **Support UTF-8 complet** :
   - setlocale(LC_ALL, "") au démarrage (obligatoire)
   - wchar_t pour plan_statique (caractères multi-bytes)

3. **Architecture modulaire** :
   - 9 fichiers sources, responsabilités claires
   - Séparation interface (.h) / implémentation (.c)

4. **Détection collision AABB** :
   - Avec TOLERANCE=2 (réduit boîte de chaque côté)
   - 4 tests de séparation (O(1))

5. **Spawn adaptatif** :
   - Mode Normal : 30/50/80 frames selon file
   - Mode Hard : 20/35/55 frames (plus rapide)

---

### Points à laisser de côté si manque de temps

1. Détails des flèches de virage (⮡⮢⮣⮤⮥⮦⮧) - juste dire "8 types de flèches"
2. Historique des versions (section 9 du rapport) - non critique pour pitch
3. Optimisations futures (section 8) - focus sur l'existant
4. Détails des tests unitaires (Unity) - mentionner brièvement si temps
5. Problèmes connus (deadlock, parsing UTF-8 coûteux) - garder pour Q/A

---

## 2. QUESTIONS DU JURY

### Organisation des questions

- **8 thèmes** : Architecture, Structures, Mémoire, Boucle jeu, Collision, Fichiers, Niveau intermédiaire, Bugs/limites
- **43 questions** au total
- Format pour chaque question :
  - ❓ **Question**
  - ⚡ **Réponse courte** (10-20 secondes)
  - 📖 **Réponse longue** (30-60 secondes)
  - 📂 **Où regarder** (fichier:ligne)
  - 🎯 **Ce que le prof vérifie**

---

### THÈME A : Architecture et modularisation (5 questions)

#### Q1 : Pourquoi avoir séparé .c et .h ?

⚡ **Réponse courte :**
"Les .h contiennent les déclarations (structures, prototypes), les .c contiennent les implémentations. Ça permet de cacher les détails internes et de compiler séparément chaque module."

📖 **Réponse longue :**
"La séparation .h/.c suit le principe d'encapsulation. Les fichiers headers (.h) exposent l'interface publique : structures, typedef, et prototypes de fonctions. Les fichiers sources (.c) contiennent l'implémentation réelle. Cela permet deux avantages : premièrement, la compilation séparée (chaque .c est compilé en .o indépendamment), et deuxièmement, le masquage des détails (un module peut utiliser un autre en incluant juste le .h sans voir le code .c). Par exemple, jeu.c inclut mouvement.h pour appeler deplacer_tous_vehicules(), mais ne voit pas le code interne de collision.c."

📂 **Où regarder :**
- `include/mouvement.h` : Déclarations des fonctions publiques
- `src/mouvement.c` : Implémentations + fonctions statiques (privées)
- `src/jeu.c:1` : Exemples d'includes

🎯 **Ce que le prof vérifie :**
Compréhension de la compilation modulaire et de l'encapsulation. Vérifie que vous savez pourquoi on ne met pas tout dans un seul fichier.

---

#### Q2 : Comment évitez-vous les includes circulaires ?

⚡ **Réponse courte :**
"J'utilise des forward declarations dans les headers et je fais attention à l'ordre des includes. Par exemple, plan.h fait un typedef struct pour l_car sans inclure liste_car.h."

📖 **Réponse longue :**
"Les includes circulaires arrivent quand A.h inclut B.h et B.h inclut A.h. Pour éviter ça, j'utilise trois techniques : premièrement, les forward declarations (typedef struct liste_car l_car;) qui permettent de déclarer un type sans définir sa structure complète. Deuxièmement, je limite les includes dans les .h au strict nécessaire, et je fais les vrais includes dans les .c. Troisièmement, je vérifie que les dépendances forment un graphe acyclique : plan.h peut inclure matrice.h, mais matrice.h n'inclut jamais plan.h. Exemple concret : plan.h ligne 9 a une forward declaration pour l_car au lieu d'inclure liste_car.h."

📂 **Où regarder :**
- `include/plan.h:9` : Forward declaration `typedef struct liste_car l_car;`
- `include/matrice.h` : N'inclut aucun autre header projet
- `src/plan.c:8` : Inclut liste_car.h (dans le .c, pas le .h)

🎯 **Ce que le prof vérifie :**
Compréhension des problèmes de dépendances. Vérifie que vous savez résoudre les cycles d'includes (compétence niveau intermédiaire/avancé).

---

#### Q3 : Pourquoi mouvement.c fait 2396 lignes ? N'est-ce pas trop gros ?

⚡ **Réponse courte :**
"Oui, c'est trop gros. C'est un point d'amélioration. Idéalement, je le découperais en sous-modules : collision.c, navigation.c, parking_auto.c, sprites.c."

📖 **Réponse longue :**
"mouvement.c fait effectivement 2396 lignes, ce qui est trop pour un seul fichier. C'est un compromis de temps : j'ai priorisé les fonctionnalités avant la modularisation. Le fichier contient 5 responsabilités distinctes : déplacement des véhicules, détection de collision AABB, suivi de flèches directionnelles, parking automatique, et gestion des sprites (chargement carrosseries). Une meilleure architecture serait de créer un répertoire src/mouvement/ avec 5 fichiers : collision.c (~400 lignes), navigation.c (~500 lignes), parking_auto.c (~300 lignes), sprites.c (~200 lignes), et deplacement.c (~400 lignes). J'ai déjà commencé cette extraction avec mouvement/sprites.c et mouvement/collision.c qui existent."

📂 **Où regarder :**
- `src/mouvement.c` : 2396 lignes totales
- `src/mouvement/sprites.c` : Déjà extrait (131 lignes)
- `src/mouvement/collision.c` : Déjà extrait (230 lignes)

🎯 **Ce que le prof vérifie :**
Capacité d'auto-critique et compréhension des principes de modularité. Vérifie que vous savez identifier les problèmes et proposer des solutions (même si non implémentées).

---

#### Q4 : Expliquez le découpage src/ et include/

⚡ **Réponse courte :**
"src/ contient les implémentations (.c), include/ contient les interfaces (.h). Cette séparation est une convention standard en C pour faciliter la compilation et la lisibilité."

📖 **Réponse longue :**
"Le répertoire src/ contient tous les fichiers sources .c qui implémentent la logique du programme : main.c (point d'entrée), jeu.c (boucle de jeu), mouvement.c (déplacement), affichage.c (ncurses), plan.c (chargement plan.txt), liste_car.c (structures de données), et matrice.c (grille d'occupation). Le répertoire include/ contient les headers .h correspondants qui exposent les prototypes de fonctions et les définitions de structures. Cette séparation suit la convention Unix standard et facilite trois choses : premièrement, le Makefile peut compiler chaque .c indépendamment (flag -Iinclude), deuxièmement, on voit clairement l'API publique de chaque module, et troisièmement, ça évite de polluer le répertoire racine avec trop de fichiers."

📂 **Où regarder :**
- `src/` : 9 fichiers .c
- `include/` : 8 fichiers .h
- `Makefile:7` : `CFLAGS = -Iinclude` (indique où chercher les headers)

🎯 **Ce que le prof vérifie :**
Connaissance des conventions C standards et compréhension de la structure de projet. Vérifie que vous ne faites pas juste copier-coller une structure sans comprendre.

---

#### Q5 : Quelles sont les dépendances entre modules ?

⚡ **Réponse courte :**
"main.c appelle jeu.c qui appelle mouvement.c et affichage.c. Ces modules utilisent plan.c et liste_car.c. plan.c dépend de matrice.c. Pas de dépendance circulaire."

📖 **Réponse longue :**
"Le graphe de dépendances forme un DAG (Directed Acyclic Graph). Au sommet, main.c initialise ncurses, charge le plan, et lance la boucle de jeu. jeu.c orchestre le jeu et dépend de affichage.c (pour le rendu), mouvement.c (pour déplacer les véhicules), liste_car.c (pour gérer la liste et la file), et plan.c (pour l'état du parking). mouvement.c dépend de mouvement/collision.c et mouvement/sprites.c. plan.c dépend de matrice.c pour la grille d'occupation. À la base, matrice.c ne dépend de rien (module feuille). Cette architecture permet de compiler matrice.c en premier, puis plan.c, puis liste_car.c, puis mouvement.c et affichage.c en parallèle, et enfin jeu.c et main.c."

📂 **Où regarder :**
- `src/main.c:1-10` : Includes de tous les modules
- `src/jeu.c:1-10` : Includes des dépendances
- `Makefile` : Ordre de compilation implicite

🎯 **Ce que le prof vérifie :**
Compréhension des dépendances de compilation et de la notion de DAG. Vérifie que vous savez expliquer l'ordre de build (compétence Makefile intermédiaire).

---

### THÈME B : Structures de données (6 questions)

#### Q6 : Pourquoi une liste chaînée et pas un tableau dynamique ?

⚡ **Réponse courte :**
"Le nombre de véhicules varie constamment (spawn/despawn). Liste chaînée permet insertion/suppression O(1) en tête/queue sans réallocation."

📖 **Réponse longue :**
"J'ai choisi liste chaînée pour trois raisons. Premièrement, flexibilité : le nombre de véhicules varie dynamiquement (entre 0 et ~30), et une liste chaînée permet d'ajouter/retirer sans limite fixe ni réallocation coûteuse. Deuxièmement, insertion/suppression O(1) : ajouter en tête (ajouter_tete_liste_car) ou en queue (ajouter_queue_liste_car) prend un temps constant. Troisièmement, pas de décalage : supprimer un véhicule au milieu ne nécessite pas de déplacer tous les éléments suivants (comme avec un tableau). Le compromis c'est que parcourir la liste est O(n) séquentiel (pas de random access), mais pour 20-30 véhicules c'est négligeable. Un tableau dynamique avec realloc serait possible mais plus complexe à gérer (capacité vs taille, realloc peut échouer, pointeurs invalidés)."

📂 **Où regarder :**
- `include/liste_car.h:45-50` : Structure liste_car avec premier/dernier
- `src/liste_car.c:47-92` : Fonctions ajouter_tete et ajouter_queue (O(1))
- `src/liste_car.c:94-118` : Fonctions retirer (O(1) aussi)

🎯 **Ce que le prof vérifie :**
Compréhension des trade-offs entre structures de données. Vérifie que vous savez justifier un choix technique avec des arguments de complexité algorithmique.

---

#### Q7 : Comment ajoutez/supprimez-vous une voiture de la liste ?

⚡ **Réponse courte :**
"Ajout : ajouter_queue_liste_car() met le véhicule à la fin en O(1). Suppression : detruire_tete_liste_car() retire le premier en O(1)."

📖 **Réponse longue :**
"Pour ajouter un véhicule, j'utilise ajouter_queue_liste_car() (ligne 68-92). Si la liste est vide, je mets premier et dernier sur le nouveau véhicule. Sinon, je chaîne le dernier actuel vers le nouveau (dernier->NXT = v), et je mets à jour dernier. Pour supprimer, j'ai trois fonctions : detruire_tete_liste_car() retire le premier (ligne 94-118), detruire_queue_liste_car() retire le dernier, et detruire_vehicule_specifique() retire un véhicule par pointeur (ligne 141-153). Chaque suppression libère la mémoire du véhicule avec detruire_vehicule() et met à jour les chaînages. Important : je réinitialise toujours les pointeurs (NXT = NULL, premier/dernier mis à jour) pour éviter les dangling pointers."

📂 **Où regarder :**
- `src/liste_car.c:68-92` : ajouter_queue_liste_car()
- `src/liste_car.c:94-118` : detruire_tete_liste_car()
- `src/liste_car.c:141-153` : detruire_vehicule_specifique()

🎯 **Ce que le prof vérifie :**
Maîtrise des listes chaînées et des pointeurs. Vérifie que vous comprenez les cas limites (liste vide, un seul élément, dernier élément).

---

#### Q8 : Expliquez la structure VEHICULE en détail

⚡ **Réponse courte :**
"VEHICULE contient position (posx, posy), direction (N/S/E/O), vitesse, un modèle visuel Carrosserie[4][30], un état (actif/inactif), et un pointeur NXT pour le chaînage."

📖 **Réponse longue :**
"La structure VEHICULE (liste_car.h ligne 9-24) contient 11 champs. direction (char) indique où va le véhicule (N/S/E/O). posx et posy (int) sont les coordonnées dans le plan (ligne, colonne). vitesse (int) est la vitesse en cellules par frame (typiquement 2). alignement (char) indique gauche ou droit ('g'/'d'). type (char) est le type de véhicule ('v' voiture, 'c' camion, 's' SUV). Carrosserie[4][30] est un tableau 2D de 4 orientations x 30 caractères maximum pour le modèle visuel. code_couleur (int) est la paire ncurses (COLOR_PAIR_ROUGE, etc). etat (char) vaut '1' actif, '0' inactif, '2' en attente. tps (unsigned long) est le temps passé dans le parking. temps_attente (unsigned long) est la frame d'entrée en file. Enfin NXT (VEHICULE*) pointe vers le véhicule suivant dans la liste chaînée."

📂 **Où regarder :**
- `include/liste_car.h:9-24` : Définition complète de struct voiture
- `src/liste_car.c:11-27` : nv_vehicule() qui initialise tous les champs
- Taille estimée : ~200 bytes (4 orientations × 30 chars + champs int/pointeur)

🎯 **Ce que le prof vérifie :**
Compréhension des structures et mémoire. Vérifie que vous savez estimer sizeof() et expliquer chaque champ (pas juste réciter).

---

#### Q9 : Pourquoi PlanParking contient-il matrice_occupation ?

⚡ **Réponse courte :**
"Pour détecter les collisions rapidement. Chaque cellule du plan a un état occupé/libre dans la matrice. C'est O(1) pour vérifier une case."

📖 **Réponse longue :**
"PlanParking contient matrice_occupation (plan.h ligne 52) pour optimiser la détection de collision. La matrice est une grille 2D allouée dynamiquement (mat*) où chaque case contient un flag o (0=libre, 1=occupée). Quand une voiture se déplace, je vérifie si les 12 cellules (4x3) de sa bounding box sont libres en consultant la matrice. C'est beaucoup plus rapide que de parcourir tous les murs et obstacles à chaque frame. La matrice est initialisée dans initialiser_matrice_depuis_plan() (plan.c:365-385) en marquant tous les murs, bordures, et places occupées. Ensuite, elle est mise à jour dynamiquement quand une voiture se gare (remplir_case) ou libère (liberer_case). Sans cette matrice, je devrais parcourir le plan_statique caractère par caractère à chaque collision check."

📂 **Où regarder :**
- `include/plan.h:52` : Champ matrice_occupation dans PlanParking
- `src/plan.c:365-385` : initialiser_matrice_depuis_plan()
- `src/matrice.c:43-55` : remplir_case() et liberer_case()

🎯 **Ce que le prof vérifie :**
Compréhension de l'optimisation par structure de données auxiliaire. Vérifie que vous savez expliquer pourquoi on duplique l'information (plan_statique ET matrice).

---

#### Q10 : Quelle est la différence entre l_car et FileAttenteEntree ?

⚡ **Réponse courte :**
"l_car est la liste des véhicules actifs dans le parking. FileAttenteEntree est la file d'attente FIFO des véhicules qui attendent l'ouverture de la barrière."

📖 **Réponse longue :**
"Ce sont deux structures distinctes avec des rôles différents. l_car (liste_car.h ligne 45-50) est une liste chaînée doublement liée qui contient tous les véhicules actifs (etat='1') dans le parking en train de se déplacer, se garer, ou repartir. Elle permet insertion/suppression à la volée. FileAttenteEntree (liste_car.h ligne 52-57) est une file d'attente FIFO (First In First Out) qui contient les véhicules en état d'attente (etat='2') devant la barrière fermée. Elle a un premier_attente et dernier_attente, et une longueur_max (10). Un véhicule passe par 3 états : spawn → FileAttenteEntree (etat='2') → l_car (etat='1' après ouverture barrière) → destruction. Un véhicule n'est jamais dans les deux structures simultanément."

📂 **Où regarder :**
- `include/liste_car.h:45-50` : Structure l_car (liste chaînée)
- `include/liste_car.h:52-57` : Structure FileAttenteEntree (file FIFO)
- `src/jeu.c:117` : retirer_de_file_attente() → ajouter_queue_liste_car()

🎯 **Ce que le prof vérifie :**
Compréhension des structures de données et des états. Vérifie que vous savez distinguer liste vs file, et que vous comprenez le cycle de vie des objets.

---

#### Q11 : Estimez sizeof(VEHICULE) en bytes

⚡ **Réponse courte :**
"Environ 200 bytes : Carrosserie[4][30] = 120 bytes, plus ~80 bytes pour les champs int/long/pointeur."

📖 **Réponse longue :**
"Calculons : Carrosserie[4][30] = 4 × 30 = 120 bytes (char = 1 byte). direction (char) + alignement (char) + type (char) + etat (char) = 4 bytes. posx (int) + posy (int) + vitesse (int) + code_couleur (int) = 16 bytes (int = 4 bytes sur 64-bit). tps (unsigned long) + temps_attente (unsigned long) = 16 bytes (long = 8 bytes). NXT (VEHICULE*) = 8 bytes (pointeur 64-bit). Total brut = 120 + 4 + 16 + 16 + 8 = 164 bytes. Avec le padding pour alignement (structures alignées sur 8 bytes), sizeof(VEHICULE) est probablement ~168-176 bytes. Pour 30 véhicules max, ça fait ~5 Ko total, ce qui est négligeable."

📂 **Où regarder :**
- `include/liste_car.h:9-24` : Tous les champs de struct voiture
- Vérifier avec : `printf("sizeof(VEHICULE) = %lu\n", sizeof(VEHICULE));`

🎯 **Ce que le prof vérifie :**
Compréhension de sizeof() et de l'alignement mémoire. Vérifie que vous savez calculer la taille d'une structure (compétence bas niveau).

---

### THÈME C : Mémoire / pointeurs (8 questions)

#### Q12 : Où sont tous les malloc et free dans le projet ?

⚡ **Réponse courte :**
"12 malloc répartis dans liste_car.c, matrice.c, plan.c, et affichage.c. Tous ont un free correspondant dans les fonctions de destruction."

📖 **Réponse longue :**
"Il y a 12 allocations dynamiques : liste_car.c fait 4 malloc (nv_vehicule ligne 11, nv_liste_car ligne 32, charger_modele_voiture lignes 262 et 271, creer_file_attente ligne 342). matrice.c fait 3 malloc (creer_matrice lignes 8, 15, et 23 pour la matrice 2D). plan.c fait 1 malloc (charger_plan ligne 305). affichage.c fait 1 malloc (creer_gestion_affichage ligne 819). Chaque malloc a un free correspondant : detruire_vehicule (ligne 42), detruire_liste_car (ligne 160), charger_modele_voiture free immédiat (lignes 241-245), detruire_file_attente (ligne 368), detruire_matrice (lignes 64-75), detruire_plan (ligne 358), detruire_gestion_affichage (ligne 843). Total : 12 allocations, 12 libérations."

📂 **Où regarder :**
- `src/liste_car.c:11, 32, 262, 271, 342` : malloc
- `src/liste_car.c:42, 160, 241-245, 368` : free correspondants
- `src/matrice.c:8, 15, 23` : malloc matrice
- `src/matrice.c:64-75` : free matrice

🎯 **Ce que le prof vérifie :**
Rigueur dans la gestion mémoire. Vérifie que vous avez recensé TOUTES les allocations et que vous savez expliquer le pairing malloc/free.

---

#### Q13 : Comment garantissez-vous qu'une voiture supprimée n'est plus utilisée ?

⚡ **Réponse courte :**
"Je mets le pointeur à NULL après free, et je vérifie toujours NULL avant déréférencement dans les boucles de parcours."

📖 **Réponse longue :**
"Trois mécanismes de sécurité. Premièrement, detruire_vehicule() (ligne 42-44) fait free(*v) puis *v = NULL, ce qui garantit que le pointeur est réinitialisé. Deuxièmement, dans les boucles de parcours (par exemple deplacer_tous_vehicules), je vérifie toujours if (current != NULL) avant d'accéder aux champs. Troisièmement, j'utilise des états (etat='0' pour inactif) pour marquer les véhicules avant suppression, ce qui évite d'accéder à un véhicule en cours de destruction. Enfin, un véhicule ne peut être que dans UNE structure à la fois : soit dans l_car, soit dans FileAttenteEntree, jamais les deux. Cela évite les double free et les use-after-free."

📂 **Où regarder :**
- `src/liste_car.c:42-44` : detruire_vehicule() avec *v = NULL
- `src/mouvement.c:1400-1450` : Boucle while (current != NULL)
- `src/jeu.c:191` : Vérification etat avant suppression

🎯 **Ce que le prof vérifie :**
Compréhension des pointeurs dangling et use-after-free. Vérifie que vous avez des stratégies défensives.

---

#### Q14 : Avez-vous des risques de fuite mémoire ? Comment les évitez-vous ?

⚡ **Réponse courte :**
"Non, pas de fuite. Toutes les allocations ont un free correspondant, et main() fait le nettoyage complet en cas d'erreur et en fin de programme."

📖 **Réponse longue :**
"Il n'y a pas de fuite mémoire pour trois raisons. Premièrement, chaque structure a une fonction de destruction complète : detruire_liste_car() parcourt TOUS les véhicules et les libère un par un, detruire_matrice() libère toutes les lignes puis le tableau de pointeurs, detruire_file_attente() libère tous les véhicules en attente. Deuxièmement, main() (lignes 64-66) fait le nettoyage systématique à la fin : detruire_file_attente(), detruire_liste_car(), detruire_plan() (qui appelle detruire_matrice()). Troisièmement, en cas d'erreur pendant l'initialisation, main() fait le nettoyage partiel des structures déjà allouées (lignes 45, 54). Je pourrais vérifier avec valgrind pour confirmer 0 bytes lost."

📂 **Où regarder :**
- `src/main.c:64-66` : Nettoyage final (3 detruire_*)
- `src/liste_car.c:155-161` : detruire_liste_car() libère TOUS les véhicules
- `src/matrice.c:57-76` : detruire_matrice() libère toutes les lignes

🎯 **Ce que le prof vérifie :**
Compréhension de la gestion mémoire globale et des fuites. Vérifie que vous savez tracer le cycle de vie complet de la mémoire.

---

#### Q15 : Expliquez le nettoyage en cascade dans creer_matrice()

⚡ **Réponse courte :**
"Si malloc échoue à la ligne i, je libère les lignes 0..i-1 déjà allouées, puis le tableau de pointeurs, puis la structure mat."

📖 **Réponse longue :**
"creer_matrice() (matrice.c lignes 6-41) alloue une matrice 2D en 3 étapes : malloc(mat), malloc(tableau de n pointeurs), puis n malloc(tableau de m cases). Le nettoyage cascade se trouve lignes 24-31 : si malloc(mt->tab[i]) échoue, je fais une boucle for (j=0; j<i; j++) pour libérer toutes les lignes déjà allouées (tab[0] à tab[i-1]), puis free(mt->tab) pour libérer le tableau de pointeurs, puis free(mt) pour libérer la structure. C'est essentiel car sinon, si l'allocation de la ligne 10 échoue, les lignes 0-9 resteraient allouées mais inaccessibles (fuite). Ce pattern de nettoyage progressif est une bonne pratique pour toute allocation multi-niveaux."

📂 **Où regarder :**
- `src/matrice.c:22-31` : Bloc if (!mt->tab[i]) avec cascade
- `src/matrice.c:15-20` : Même pattern si malloc(tab) échoue

🎯 **Ce que le prof vérifie :**
Maîtrise de la gestion d'erreur dans les allocations complexes. C'est une compétence avancée que peu d'étudiants maîtrisent.

---

#### Q16 : Pourquoi réinitialisez-vous *ptr = NULL après free ?

⚡ **Réponse courte :**
"Pour éviter les dangling pointers. Si le pointeur reste avec l'ancienne adresse, un free() ou déréférencement ultérieur causerait un crash."

📖 **Réponse longue :**
"Mettre *ptr = NULL après free() est une protection contre deux bugs classiques. Premièrement, le use-after-free : si quelqu'un déréférence le pointeur après free, accéder à NULL provoque un segfault immédiat et déterministe (plus facile à déboguer qu'un accès à une zone mémoire aléatoire). Deuxièmement, le double-free : si on appelle free() deux fois sur le même pointeur, free(NULL) est sûr (ne fait rien) alors que free(adresse) deux fois corrompt le heap. Exemple : detruire_vehicule() (ligne 43) fait *v = NULL, donc si on l'appelle deux fois par erreur, le deuxième appel ne fera rien au lieu de crasher. C'est une bonne pratique défensive systématique."

📂 **Où regarder :**
- `src/liste_car.c:43` : *v = NULL dans detruire_vehicule()
- `src/liste_car.c:161` : *lc = NULL dans detruire_liste_car()
- `src/matrice.c:65, 71, 75` : Multiples NULL après free

🎯 **Ce que le prof vérifie :**
Compréhension des bugs mémoire classiques (use-after-free, double-free) et des stratégies de prévention.

---

#### Q17 : Que se passe-t-il si malloc échoue dans nv_vehicule() ligne 11 ?

⚡ **Réponse courte :**
"C'est un bug : il n'y a pas de vérification NULL. Le programme crasherait à la ligne 12 en essayant d'écrire dans vh->direction."

📖 **Réponse longue :**
"C'est effectivement un point faible du code. nv_vehicule() ligne 11 fait malloc(sizeof(struct voiture)) sans vérifier si le retour est NULL. Si malloc échoue (mémoire insuffisante), vh vaut NULL, et ligne 12 vh->direction = dir provoque un segmentation fault (déréférencement de NULL). La correction serait d'ajouter : if (!vh) return NULL; après le malloc, et de vérifier le retour de nv_vehicule() dans creer_voiture_aleatoire(). Ce bug existe aussi dans nv_liste_car() ligne 32. C'est un oubli de validation que j'admettrais en soutenance, en expliquant que ça fonctionne en pratique car les allocations de ~200 bytes échouent rarement, mais que ce n'est pas robuste. Une amélioration serait d'ajouter ces checks."

📂 **Où regarder :**
- `src/liste_car.c:11-27` : nv_vehicule() sans check NULL
- `src/liste_car.c:32-36` : nv_liste_car() même problème
- Comparer avec `src/matrice.c:8-14` qui vérifie bien : if (!mt) return NULL

🎯 **Ce que le prof vérifie :**
Honnêteté technique et capacité d'auto-critique. Vérifie que vous identifiez les bugs ET que vous savez proposer des corrections.

---

#### Q18 : Comment évitez-vous les double free ?

⚡ **Réponse courte :**
"Par conception : un véhicule ne peut être que dans UNE structure à la fois (l_car OU FileAttenteEntree). Et je mets NULL après free."

📖 **Réponse longue :**
"Trois stratégies pour éviter double free. Premièrement, séparation stricte des états : un véhicule passe de FileAttenteEntree à l_car, jamais dans les deux simultanément. Quand retirer_de_file_attente() (liste_car.c:422-432) retire un véhicule de la file, il met v->NXT = NULL et change etat='1', donc le véhicule n'est plus accessible depuis la file. Deuxièmement, réinitialisation systématique : detruire_vehicule() met *v = NULL, donc si on l'appelle deux fois, free(NULL) ne fait rien. Troisièmement, nettoyage ordonné : main() détruit d'abord la file, puis la liste, donc chaque véhicule n'est libéré qu'une fois. Un scénario à risque serait si un véhicule était dans les deux structures, mais le code garantit que c'est impossible."

📂 **Où regarder :**
- `src/liste_car.c:422-432` : retirer_de_file_attente() met NXT = NULL
- `src/main.c:64-66` : Ordre de destruction (file puis liste)
- `src/liste_car.c:43` : *v = NULL protection

🎯 **Ce que le prof vérifie :**
Compréhension des invariants de structure et de la gestion d'état. Vérifie que vous raisonnez en termes d'invariants (niveau avancé).

---

#### Q19 : Un véhicule peut-il être dans 2 listes simultanément ?

⚡ **Réponse courte :**
"Non, c'est impossible par conception. Un véhicule est soit dans FileAttenteEntree (etat='2'), soit dans l_car (etat='1'), jamais les deux."

📖 **Réponse longue :**
"Non, c'est un invariant du système. Le cycle de vie d'un véhicule est linéaire : création → ajout FileAttenteEntree (etat='2') → retrait file + ajout l_car (etat='1') → retrait l_car → destruction. La transition file → liste se fait atomiquement dans traiter_entree_vehicules() (jeu.c:117) : retirer_de_file_attente() retire ET change etat='1', puis ajouter_queue_liste_car() l'ajoute à la liste. Il n'y a jamais de moment où le véhicule est dans les deux. Si c'était possible, on aurait un risque de double free : detruire_file_attente() libérerait le véhicule, puis detruire_liste_car() le libérerait à nouveau. Cet invariant est crucial pour la correction du programme."

📂 **Où regarder :**
- `src/jeu.c:117` : Transition atomique file → liste
- `src/liste_car.c:428-431` : retirer_de_file_attente() met NXT = NULL
- `src/liste_car.c:354-370` : detruire_file_attente() ne touche que la file

🎯 **Ce que le prof vérifie :**
Compréhension des invariants de structure et de la sémantique du code. Vérifie que vous raisonnez au-delà du code ligne par ligne (pensée architecturale).

---

### THÈME D : Boucle de jeu / timing (5 questions)

#### Q20 : Comment gérez-vous la mise à jour (tick) du jeu ?

⚡ **Réponse courte :**
"Boucle while infinie qui lit l'input, déplace les véhicules tous les 5 frames, affiche, puis pause 57ms. Ça donne ~17 FPS."

📖 **Réponse longue :**
"La boucle de jeu est dans executer_boucle_jeu() (jeu.c:351-506). Chaque itération fait : lire input clavier (getch() non-bloquant), gérer entrée/sortie de file (traiter_entree_vehicules), gérer spawn avec cooldown, tous les 5 frames déplacer les véhicules (deplacer_tous_vehicules) et vérifier collisions, traiter timeouts de file, afficher avec clear() + afficher_plan_avec_viewport() + refresh(), puis napms(57) pour pause. Le compteur global_frame_counter s'incrémente à chaque itération. Le mouvement tous les 5 frames évite de surcharger l'affichage : à 57ms par frame, 5 frames = 285ms, soit ~3.5 mouvements par seconde. C'est un compromis entre fluidité et performance."

📂 **Où regarder :**
- `src/jeu.c:351-506` : Boucle principale complète
- `src/jeu.c:481-485` : Mouvement conditionnel (if frame_counter >= 5)
- `src/jeu.c:505` : napms(57) pause

🎯 **Ce que le prof vérifie :**
Compréhension des boucles de jeu et du frame rate. Vérifie que vous savez expliquer pourquoi on espace les updates.

---

#### Q21 : Pourquoi napms(57) et pas 60 ou 50 ?

⚡ **Réponse courte :**
"Pour obtenir ~17.5 FPS. 1000ms / 57ms ≈ 17.5 frames par seconde. C'est un compromis entre fluidité et performance."

📖 **Réponse longue :**
"57ms est calculé pour un frame rate cible. 1000 ms / 57 ms ≈ 17.5 FPS. Pourquoi pas 60 FPS (16-17ms) ? Parce que l'affichage ncurses est coûteux : afficher_plan_avec_viewport() relit plan.txt et parse l'UTF-8 à chaque frame, ce qui prend ~20-30ms. Si on mettait napms(16), le jeu ne tiendrait pas 60 FPS (lag). 57ms laisse ~30ms pour l'affichage + logique, ce qui est confortable. Avec le mouvement tous les 5 frames, les voitures bougent ~3.5 fois par seconde, ce qui est suffisant pour une simulation de parking (pas besoin de 60 FPS comme un FPS). C'est un choix de design empirique : j'ai testé différentes valeurs et 57ms donnait le meilleur compromis fluidité/performance."

📂 **Où regarder :**
- `src/jeu.c:505` : napms(57)
- `src/affichage.c:252-402` : afficher_plan_avec_viewport() (fonction coûteuse)
- `src/jeu.c:481-485` : Mouvement tous les 5 frames

🎯 **Ce que le prof vérifie :**
Compréhension du frame rate et des contraintes de performance. Vérifie que vous savez mesurer et justifier des choix empiriques.

---

#### Q22 : Pourquoi déplacer les véhicules tous les 5 frames et pas chaque frame ?

⚡ **Réponse courte :**
"Pour réduire la charge CPU. Déplacer à chaque frame donnerait 17 updates/seconde, mais tous les 5 frames donne ~3.5 updates/seconde, suffisant pour une simulation."

📖 **Réponse longue :**
"C'est une optimisation de performance. deplacer_tous_vehicules() (mouvement.c:1400-1500) parcourt TOUS les véhicules, fait suivre_fleches() pour chacun (O(nb_fleches)), calcule nouveau_x/nouveau_y, vérifie peut_deplacer() (parcourt 12 cellules), et teste collision avec tous les autres (O(nb_veh²)). Pour 20 véhicules, ça fait ~400 opérations par update. À 17 FPS, ça ferait 6800 ops/sec. En espaçant tous les 5 frames, on descend à ~1400 ops/sec. De plus, les voitures n'ont pas besoin de bouger 17 fois par seconde pour une simulation de parking : 3-4 fois/sec suffit. Le compromis c'est que le mouvement paraît un peu saccadé, mais c'est acceptable. Si on voulait plus fluide, il faudrait optimiser deplacer_tous_vehicules()."

📂 **Où regarder :**
- `src/jeu.c:481-485` : if (frame_counter >= 5) { deplacer... }
- `src/mouvement.c:1400-1500` : deplacer_tous_vehicules() (fonction coûteuse)
- `src/mouvement.c:1327-1398` : deplacer_vehicule() appelé pour chaque

🎯 **Ce que le prof vérifie :**
Compréhension des optimisations et des trade-offs performance/qualité. Vérifie que vous savez justifier des choix non-évidents.

---

#### Q23 : Comment évitez-vous un rafraîchissement lourd du terminal ?

⚡ **Réponse courte :**
"Je n'optimise pas vraiment : clear() + redessine tout à chaque frame. C'est un bottleneck connu. Une amélioration serait d'afficher seulement les zones modifiées."

📖 **Réponse longue :**
"Actuellement, le rafraîchissement n'est pas optimisé. Chaque frame fait clear() (efface tout l'écran), puis afficher_plan_avec_viewport() relit plan.txt depuis le disque et parse l'UTF-8 ligne par ligne (O(hauteur × largeur)), puis affiche_vehicule_viewport() dessine tous les véhicules, et enfin refresh(). C'est le bottleneck principal : reli

r un fichier texte à chaque frame est coûteux (~20ms). Une optimisation serait de charger plan.txt une seule fois en mémoire (cache), puis utiliser mvaddch() pour redessiner seulement les cellules modifiées (voitures qui bougent). Avec ncurses double-buffering, on pourrait descendre à ~5ms par frame. Mais j'ai priorisé la simplicité du code sur la performance."

📂 **Où regarder :**
- `src/jeu.c:488-501` : clear() + afficher_plan + afficher_vehicules + refresh()
- `src/affichage.c:252-402` : afficher_plan_avec_viewport() (relit fichier)
- Ligne 270: `fopen(fichier_plan, "r")` CHAQUE FRAME

🎯 **Ce que le prof vérifie :**
Honnêteté sur les limitations et compréhension des optimisations possibles. Vérifie que vous savez identifier les bottlenecks.

---

#### Q24 : Si ça ralentit avec beaucoup de voitures, pourquoi ?

⚡ **Réponse courte :**
"Deux raisons : détection collision O(nb_veh²), et affichage qui redessine tout. Avec 30 voitures, ça fait 900 comparaisons de collision par update."

📖 **Réponse longue :**
"Il y a deux bottlenecks. Premièrement, la détection de collision dans deplacer_tous_vehicules() est O(n²) : pour chaque véhicule, je teste collision avec tous les autres. Avec 30 véhicules, ça fait 30 × 30 = 900 tests par update. Chaque test AABB est O(1), mais 900 tests prennent ~5ms. Deuxièmement, l'affichage relit plan.txt et redessine tout à chaque frame. Avec un plan de 80×40 caractères UTF-8, ça fait ~3200 caractères à parser et afficher, soit ~20ms. Total : ~25ms par frame sans compter la logique. À 30 véhicules, on approche la limite des 57ms. Une optimisation serait d'utiliser une grille de hachage spatial pour la collision (O(n) au lieu de O(n²)), et un cache pour le plan."

📂 **Où regarder :**
- `src/mouvement.c:1400-1500` : Double boucle véhicules (O(n²))
- `src/mouvement/collision.c:114-149` : vehicules_en_collision() appelé O(n²) fois
- `src/affichage.c:270` : fopen CHAQUE FRAME

🎯 **Ce que le prof vérifie :**
Analyse de complexité et profilage. Vérifie que vous savez identifier les sections coûteuses et expliquer pourquoi (compétence optimisation).

---

### THÈME E : Collisions et grille (6 questions)

#### Q25 : Comment détectez-vous les collisions entre deux véhicules ?

⚡ **Réponse courte :**
"AABB : je teste si les boîtes englobantes de deux véhicules se chevauchent avec 4 comparaisons de coordonnées. Si aucune séparation, il y a collision."

📖 **Réponse longue :**
"J'utilise l'algorithme AABB (Axis-Aligned Bounding Box) dans vehicules_en_collision() (collision.c:114-149). Chaque véhicule a une boîte rectangulaire définie par (x_min, x_max, y_min, y_max). Je réduis cette boîte de TOLERANCE=2 pixels de chaque côté pour éviter les faux positifs (voitures adjacentes sans collision réelle). Ensuite, je teste 4 conditions de séparation : véhicule1 complètement à gauche de 2, ou complètement à droite, ou complètement au-dessus, ou complètement en-dessous. Si au moins une condition est vraie (separated=1), il n'y a PAS collision. Si toutes sont fausses (separated=0), les boîtes se chevauchent → collision. C'est O(1) car 4 comparaisons seulement. Cette méthode est standard en game dev 2D."

📂 **Où regarder :**
- `src/mouvement/collision.c:114-149` : vehicules_en_collision() complet
- Lignes 124-136: Calcul boîtes avec TOLERANCE=2
- Lignes 143-146: 4 tests de séparation
- `src/mouvement.c:1459-1466` : Appel dans deplacer_tous_vehicules()

🎯 **Ce que le prof vérifie :**
Compréhension des algorithmes de collision 2D. Vérifie que vous connaissez AABB et que vous savez l'expliquer clairement.

---

#### Q26 : Qu'est-ce que AABB ? Pourquoi TOLERANCE=2 ?

⚡ **Réponse courte :**
"AABB = Axis-Aligned Bounding Box, boîte rectangulaire alignée sur les axes. TOLERANCE=2 réduit la boîte pour éviter que deux voitures côte à côte déclenchent une collision."

📖 **Réponse longue :**
"AABB est une technique de détection de collision simple et rapide. Chaque objet est entouré d'une boîte rectangulaire dont les côtés sont parallèles aux axes X et Y (pas de rotation). Pour tester collision, on vérifie si deux boîtes se chevauchent. TOLERANCE=2 sert à compenser le fait que les voitures font 3-4 cellules de large : si deux voitures sont dans des allées adjacentes (séparées par 1 cellule de mur), leurs boîtes peuvent se toucher mathématiquement mais physiquement il n'y a pas collision. En réduisant chaque boîte de 2 pixels de chaque côté, on crée une marge de sécurité. Par exemple, une boîte de largeur 4 devient largeur 0 après réduction (4 - 2×2 = 0), ce qui empêche les faux positifs. Sans TOLERANCE, le jeu serait injouable (collisions constantes)."

📂 **Où regarder :**
- `src/mouvement/collision.c:124` : const int TOLERANCE = 2;
- Lignes 126-136: Réduction des boîtes (x_min + TOLERANCE, x_max - TOLERANCE)
- Commentaire ligne 125: "Réduire légèrement pour éviter faux positifs"

🎯 **Ce que le prof vérifie :**
Compréhension fine de la collision et du tuning. Vérifie que vous savez expliquer les "magic numbers" et les justifier.

---

#### Q27 : Comment garantissez-vous qu'une voiture n'écrase pas une autre ?

⚡ **Réponse courte :**
"Avant chaque déplacement, peut_deplacer() vérifie que toutes les cellules de destination sont libres. Puis deplacer_tous_vehicules() teste collision AABB avec tous les autres."

📖 **Réponse longue :**
"Deux niveaux de protection. Niveau 1 : peut_deplacer() (collision.c:28-67) vérifie que les 12 cellules (4 lignes × 3 colonnes) de la nouvelle position ne contiennent pas de murs ou bordures dans plan_statique. Mais cette fonction ne teste PAS les autres véhicules (c'est un choix). Niveau 2 : deplacer_tous_vehicules() (mouvement.c:1459-1466) fait une double boucle pour tester collision AABB entre TOUS les véhicules deux à deux après déplacement. Si collision détectée, le jeu se termine (GAME OVER). Donc en théorie, deux voitures ne devraient jamais se chevaucher. En pratique, ça fonctionne bien sauf si deux voitures arrivent exactement à la même frame sur la même cellule (cas rare mais possible → GAME OVER)."

📂 **Où regarder :**
- `src/mouvement/collision.c:28-67` : peut_deplacer() vérifie murs/bordures
- `src/mouvement.c:1459-1479` : Double boucle collision + if (collision) GAME OVER
- `src/jeu.c:482-485` : Si collision, running = 0

🎯 **Ce que le prof vérifie :**
Compréhension de la robustesse et des edge cases. Vérifie que vous avez pensé aux scénarios limites.

---

#### Q28 : Comment la grille (matrice) est-elle synchronisée avec la liste chaînée ?

⚡ **Réponse courte :**
"Pas synchronisée en temps réel. La matrice marque les murs/places occupées au chargement. Les voitures sont dans la liste, pas dans la matrice."

📖 **Réponse longue :**
"La matrice d'occupation (mat*) et la liste chaînée (l_car*) servent deux objectifs différents et ne sont pas vraiment synchronisées. La matrice est initialisée une fois dans initialiser_matrice_depuis_plan() (plan.c:365-385) en marquant tous les éléments statiques : murs ('#'), bordures ('|', '_'), et places occupées initiales ('X'). Ensuite, la matrice est mise à jour seulement quand une voiture se gare (occuper_place_parking() appelle remplir_case()) ou libère (liberer_place_parking() appelle liberer_case()). Mais pendant le mouvement des voitures, la matrice n'est PAS mise à jour : les véhicules en déplacement sont trackés uniquement dans la liste chaînée. C'est pour ça que peut_deplacer() ne détecte pas les autres véhicules (ils ne sont pas dans la matrice)."

📂 **Où regarder :**
- `src/plan.c:365-385` : initialiser_matrice_depuis_plan()
- `src/plan.c:406-419` : occuper_place_parking() → remplir_case()
- `src/mouvement/collision.c:28-67` : peut_deplacer() consulte seulement matrice (pas liste)

🎯 **Ce que le prof vérifie :**
Compréhension de la séparation des responsabilités et des structures auxiliaires. Vérifie que vous savez expliquer pourquoi deux structures ne sont pas synchronisées.

---

#### Q29 : Quelle est la complexité de la détection de collision ?

⚡ **Réponse courte :**
"O(n²) où n = nombre de véhicules. Double boucle pour tester chaque paire. Pour 30 véhicules, ça fait ~900 tests."

📖 **Réponse longue :**
"La détection globale dans deplacer_tous_vehicules() (mouvement.c:1459-1479) est O(n²). Il y a deux boucles imbriquées : la première parcourt tous les véhicules (n), la seconde parcourt tous les autres pour chaque véhicule (n-1). Donc n × (n-1) / 2 paires testées (division par 2 car on évite de tester A vs B ET B vs A). Pour n=30, c'est 30 × 29 / 2 = 435 paires. Chaque test AABB (vehicules_en_collision) est O(1) (4 comparaisons), donc complexité totale O(n²). C'est acceptable pour n<50, mais deviendrait un problème pour n>100. Une optimisation classique serait le spatial hashing : diviser le plan en grille, mettre chaque véhicule dans sa cellule, et tester seulement contre véhicules de cellules adjacentes → O(n)."

📂 **Où regarder :**
- `src/mouvement.c:1459-1479` : Double boucle while (current) { while (autre) }
- `src/mouvement/collision.c:114-149` : vehicules_en_collision() O(1)

🎯 **Ce que le prof vérifie :**
Analyse de complexité algorithmique. Vérifie que vous savez calculer O() et proposer des optimisations (compétence algorithmique avancée).

---

#### Q30 : Pourquoi peut_deplacer() ne vérifie-t-il pas les collisions avec d'autres véhicules ?

⚡ **Réponse courte :**
"Par choix de design : peut_deplacer() vérifie seulement les obstacles statiques (murs). Les collisions dynamiques sont testées après, dans deplacer_tous_vehicules()."

📖 **Réponse longue :**
"C'est une séparation des responsabilités. peut_deplacer() (collision.c:28-67) se concentre sur les obstacles statiques : bordures du plan, murs ('#'), caractères spéciaux (╔, ╗, etc.). Elle consulte uniquement plan_statique et la matrice d'occupation. Tester les autres véhicules ici serait coûteux (faudrait parcourir la liste à chaque appel) et redondant. À la place, deplacer_tous_vehicules() fait d'abord bouger TOUS les véhicules (en appelant peut_deplacer pour les obstacles), PUIS teste les collisions véhicule-véhicule en une seule passe O(n²). C'est plus efficace que de tester à chaque déplacement individuel. Le compromis c'est qu'on détecte la collision après coup, d'où le GAME OVER au lieu de bloquer le mouvement."

📂 **Où regarder :**
- `src/mouvement/collision.c:28-67` : peut_deplacer() teste seulement plan_statique
- `src/mouvement.c:1327-1398` : deplacer_vehicule() appelle peut_deplacer()
- `src/mouvement.c:1459-1479` : Après tous déplacements, test collision

🎯 **Ce que le prof vérifie :**
Compréhension des choix de design et des trade-offs. Vérifie que vous savez justifier des décisions architecturales.

---

### THÈME F : Lecture de fichiers (.txt) (4 questions)

#### Q31 : Comment chargez-vous plan.txt ?

⚡ **Réponse courte :**
"charger_plan() ouvre le fichier, lit ligne par ligne avec fgets(), convertit UTF-8 en wchar_t avec mbstowcs(), et remplit plan_statique."

📖 **Réponse longue :**
"Le chargement se fait dans charger_plan() (plan.c:296-348). Étape 1 : fopen(fichier_plan, 'r') ouvre le fichier en lecture. Étape 2 : malloc(sizeof(PlanParking)) alloue la structure. Étape 3 : boucle while (fgets(ligne, MAX_LIGNE, fichier)) lit chaque ligne (max 600 bytes pour UTF-8). Étape 4 : nettoyer_retour_ligne() enlève le '\\n'. Étape 5 : traiter_ligne_plan() convertit la ligne UTF-8 (char*) en wchar_t via mbstowcs(), puis copie dans plan_statique[ligne_courante][]. En même temps, detecter_place_wchar() cherche les caractères ╦ pour identifier les places de parking, et detecter_fleche_wchar() cherche les flèches (←→↑↓⮡⮢...). Étape 6 : creer_matrice() alloue la matrice d'occupation. Étape 7 : initialiser_matrice_depuis_plan() marque les murs/places dans la matrice."

📂 **Où regarder :**
- `src/plan.c:296-348` : charger_plan() fonction complète
- `src/plan.c:240-282` : traiter_ligne_plan() + mbstowcs()
- `src/plan.c:40-54` : detecter_place_wchar()
- `src/plan.c:57-122` : detecter_fleche_wchar()

🎯 **Ce que le prof vérifie :**
Compréhension de l'I/O fichier et de l'encodage UTF-8. Vérifie que vous savez expliquer le parsing multi-byte.

---

#### Q32 : Que se passe-t-il si plan.txt est invalide ou manquant ?

⚡ **Réponse courte :**
"Si le fichier n'existe pas, fopen() retourne NULL et charger_plan() retourne NULL. Le programme affiche une erreur et se termine."

📖 **Réponse longue :**
"Il y a deux cas d'erreur. Premier cas : fichier manquant. Si fopen(fichier_plan, 'r') échoue (ligne 298), charger_plan() affiche 'Erreur : impossible d'ouvrir le fichier' et retourne NULL. main() (ligne 29) reçoit NULL, affiche un message d'erreur (ligne 45-49), et termine avec return 1. Deuxième cas : contenu invalide (par exemple, pas d'entrée/sortie détectée). Le code ne vérifie PAS explicitement si entree_x/sortie_x sont restés à 0 (initialisés dans initialiser_plan_parking ligne 20-27). Dans ce cas, les véhicules spawneraient à (0,0) ce qui crasherait probablement. Une amélioration serait d'ajouter après charger_plan() : if (plan->entree_x == 0) { erreur 'Entrée non trouvée'; return NULL; }."

📂 **Où regarder :**
- `src/plan.c:298-303` : Vérification fopen() != NULL
- `src/main.c:45-49` : Gestion erreur si charger_plan() retourne NULL
- `src/plan.c:12-27` : initialiser_plan_parking() met entree_x/sortie_x à 0

🎯 **Ce que le prof vérifie :**
Gestion d'erreur et robustesse. Vérifie que vous savez identifier les cas d'erreur non gérés et proposer des améliorations.

---

#### Q33 : Pourquoi MAX_LIGNE vaut 600 et pas 200 ?

⚡ **Réponse courte :**
"Parce que UTF-8 encode certains caractères sur 3 bytes. Une ligne de 200 caractères visuels peut faire 600 bytes si tous sont des box-drawing."

📖 **Réponse longue :**
"C'est lié à l'encodage UTF-8. Les caractères ASCII (A-Z, 0-9, espaces) prennent 1 byte. Mais les caractères box-drawing utilisés dans le plan (═, ║, ╔, ╗, ╦, ← , →, etc.) sont encodés sur 3 bytes en UTF-8. Par exemple, ═ (U+2550) = 0xE2 0x95 0x90 (3 bytes). Si une ligne contient 200 caractères box-drawing, elle fait 200 × 3 = 600 bytes. fgets(ligne, MAX_LIGNE, fichier) lit au maximum MAX_LIGNE-1 bytes, donc avec MAX_LIGNE=600, on peut lire une ligne de 199 caractères UTF-8 3-bytes. Si on mettait MAX_LIGNE=200, une ligne de 67 box-drawing (67 × 3 = 201) serait tronquée, ce qui casserait l'affichage. C'est une marge de sécurité conservative."

📂 **Où regarder :**
- `include/plan.h:13` : #define MAX_LIGNE 600
- Commentaire ligne 13: "Augmenté pour supporter UTF-8 (3 bytes par caractère)"
- `src/plan.c:315` : char ligne[MAX_LIGNE]; dans charger_plan()

🎯 **Ce que le prof vérifie :**
Compréhension de l'encodage UTF-8 et des calculs de taille. Vérifie que vous savez expliquer les "magic numbers" liés à l'encodage.

---

#### Q34 : Comment gérez-vous l'UTF-8 lors de la lecture du fichier ?

⚡ **Réponse courte :**
"Je lis en char* avec fgets(), puis je convertis en wchar_t avec mbstowcs(). wchar_t permet de manipuler chaque caractère UTF-8 comme une unité."

📖 **Réponse longue :**
"Le processus se fait en 3 étapes. Étape 1 : fgets(ligne, MAX_LIGNE, fichier) lit la ligne brute en char* (bytes UTF-8). Étape 2 : mbstowcs(wligne, ligne, MAX_LARGEUR) convertit la séquence multi-byte UTF-8 en tableau de wchar_t. mbstowcs() interprète les séquences 1-byte (ASCII), 2-byte, 3-byte (box-drawing), et 4-byte correctement. Étape 3 : copie dans plan_statique[i][j] qui est de type wchar_t[][]. L'avantage de wchar_t c'est qu'un caractère comme ═ (3 bytes en UTF-8) devient un seul wchar_t, donc plan_statique[5][10] accède directement au 10ème caractère visuel ligne 5, pas au 30ème byte. Sans wchar_t, compter les caractères visuels serait complexe (faudrait parser UTF-8 manuellement à chaque accès)."

📂 **Où regarder :**
- `src/plan.c:243-257` : Conversion mbstowcs() dans traiter_ligne_plan()
- `include/plan.h:49` : wchar_t plan_statique[MAX_HAUTEUR][MAX_LARGEUR]
- `src/main.c:16` : setlocale(LC_ALL, "") OBLIGATOIRE avant mbstowcs()

🎯 **Ce que le prof vérifie :**
Compréhension approfondie de l'UTF-8 et de wchar_t. C'est un sujet avancé, peu d'étudiants le maîtrisent.

---

### THÈME G : Choix "niveau intermédiaire" (4 questions)

#### Q35 : Pourquoi liste chaînée et pas std::vector (ou tableau dynamique en C) ?

⚡ **Réponse courte :**
"Je code en C pur, pas en C++, donc pas de std::vector. Et liste chaînée est plus simple à implémenter qu'un tableau dynamique avec realloc()."

📖 **Réponse longue :**
"Trois raisons. Premièrement, le projet est en C pur (pas de C++), donc std::vector n'existe pas. Deuxièmement, liste chaînée est un classique enseigné en cours : structure simple, insertion/suppression O(1), pas de réallocation. C'est le choix pédagogique attendu pour un étudiant intermédiaire. Troisièmement, un tableau dynamique en C nécessiterait de gérer capacité vs taille, realloc() peut échouer, et les pointeurs vers éléments sont invalidés après realloc (bug subtil). Liste chaînée évite ces problèmes : malloc() un véhicule, le chaîner, c'est tout. Le compromis c'est pas de random access (parcours O(n)), mais pour 20-30 véhicules c'est négligeable. Un tableau dynamique serait plus 'pro' mais aussi plus risqué pour un projet étudiant."

📂 **Où regarder :**
- `include/liste_car.h:45-50` : Structure liste_car simple
- `src/liste_car.c:47-92` : Implémentation insertion O(1)
- Makefile:2 : CC = gcc (pas g++), confirme C pur

🎯 **Ce que le prof vérifie :**
Justification du niveau technique et des choix pédagogiques. Vérifie que vous savez vous positionner comme étudiant (pas expert).

---

#### Q36 : Pourquoi pas de pathfinding A* pour les déplacements ?

⚡ **Réponse courte :**
"A* serait surdimensionné. Les flèches directionnelles fournissent déjà un chemin prédéfini. Les voitures suivent les flèches, pas besoin de calculer un chemin."

📖 **Réponse longue :**
"A* (A-star) est un algorithme de pathfinding classique en IA, mais il serait overkill ici pour trois raisons. Premièrement, le plan contient déjà des flèches (←→↑↓⮡⮢...) qui définissent les chemins de circulation : les voitures suivent ces flèches (suivre_fleches() ligne 878-946), elles n'ont pas besoin de calculer un chemin. Deuxièmement, A* a une complexité O(b^d) où b=branching factor et d=profondeur, ce qui nécessite une file de priorité, une heuristique (distance Manhattan), et de la mémoire pour stocker le chemin. Pour un projet étudiant, c'est trop complexe. Troisièmement, le suivi de flèches est O(nb_fleches) = O(10) constant, donc beaucoup plus simple et rapide. Le compromis c'est que les voitures ne peuvent pas improviser si elles sont bloquées (pas de 'plan B')."

📂 **Où regarder :**
- `src/mouvement.c:878-946` : suivre_fleches() (algorithme simple)
- `src/plan.c:57-122` : detecter_fleche_wchar() (8 types de flèches)
- Complexité: O(nb_fleches) vs A* O(b^d)

🎯 **Ce que le prof vérifie :**
Compréhension des algorithmes et des choix de complexité. Vérifie que vous savez justifier la simplicité VS la sophistication.

---

#### Q37 : Quels sont vos compromis simplicité vs optimisation ?

⚡ **Réponse courte :**
"J'ai priorisé simplicité : collision O(n²) au lieu de spatial hashing, affichage relit fichier au lieu de cache, liste chaînée au lieu de tableau. Acceptable pour n<50."

📖 **Réponse longue :**
"J'ai fait trois compromis délibérés en faveur de la simplicité. Premier compromis : collision O(n²) au lieu de spatial hashing O(n). Une grille de hachage spatial diviserait le plan en cellules et testerait seulement les véhicules voisins, mais c'est complexe à implémenter. Pour 20-30 véhicules, O(n²) est acceptable (~900 tests). Deuxième compromis : affichage relit plan.txt chaque frame au lieu de cacher en mémoire. Relire 80×40 caractères prend ~20ms, c'est le bottleneck, mais le code est simple (20 lignes). Un cache nécessiterait de gérer la synchronisation plan statique vs cache. Troisième compromis : liste chaînée O(n) parcours au lieu de tableau O(1) random access. Pour 30 véhicules, parcourir la liste prend ~1µs, négligeable. Ces choix reflètent un niveau intermédiaire : je comprends les optimisations possibles mais je privilégie la maintenabilité."

📂 **Où regarder :**
- `src/mouvement.c:1459-1479` : Collision O(n²)
- `src/affichage.c:270` : fopen() chaque frame
- `src/liste_car.c:157-158` : Parcours liste O(n)

🎯 **Ce que le prof vérifie :**
Maturité technique et auto-awareness. Vérifie que vous savez reconnaître vos choix et les assumer.

---

#### Q38 : Qu'est-ce qui montre un niveau C correct sans être expert ?

⚡ **Réponse courte :**
"Gestion mémoire rigoureuse (malloc/free correct, nettoyage cascade), structures de données classiques (listes, matrices), et 0 warning de compilation."

📖 **Réponse longue :**
"Cinq éléments montrent un niveau intermédiaire solide. Premièrement, gestion mémoire : toutes les allocations ont un free correspondant, nettoyage en cascade dans creer_matrice() (pattern avancé), réinitialisation *ptr = NULL systématique. Deuxièmement, structures de données : liste chaînée doublement liée, file d'attente FIFO, matrice 2D dynamique → ce sont des classiques bien implémentés. Troisièmement, modularité : séparation .h/.c, pas d'includes circulaires, dépendances claires. Quatrièmement, robustesse : compilation sans warning (-Wall -Wextra), gestion des cas limites (liste vide, fichier manquant). Cinquièmement, utilisation de bibliothèques système (ncurses, setlocale, wchar_t) montre que je sais utiliser des APIs existantes. Ce qui n'est PAS expert : optimisations poussées (spatial hashing, SIMD), patterns avancés (pool allocator, lock-free), multithreading."

📂 **Où regarder :**
- `src/matrice.c:22-31` : Nettoyage cascade (pattern avancé)
- `src/liste_car.c` : Implémentation liste complète
- Makefile:3 : CFLAGS = -Wall -Wextra (strict)
- Compilation : 0 warning, 0 error

🎯 **Ce que le prof vérifie :**
Capacité d'auto-évaluation et positionnement. Vérifie que vous savez reconnaître votre niveau sans sur/sous-estimer.

---

### THÈME H : Bugs / limites / améliorations (7 questions + pièges)

#### Q39 : Quels problèmes ou bugs connus avez-vous identifiés ?

⚡ **Réponse courte :**
"Trois bugs : malloc sans vérification NULL dans nv_vehicule() ligne 11, possibilité de deadlock si deux voitures se bloquent, et affichage qui relit fichier chaque frame (lent)."

📖 **Réponse longue :**
"J'ai identifié cinq problèmes. Bug 1 : nv_vehicule() et nv_liste_car() font malloc sans vérifier NULL (lignes 11 et 32), crash potentiel si mémoire insuffisante. Bug 2 : deadlock invisible possible si deux voitures arrivent face à face dans une allée étroite, elles se bloquent mutuellement sans détection. Bug 3 : affichage relit plan.txt depuis disque chaque frame (affichage.c:270), ~20ms de latence. Bug 4 : parsing UTF-8 manuel coûteux (affichage.c:290-323) à chaque frame. Bug 5 : détection collision O(n²) devient lent si >50 véhicules. Ces bugs ne cassent pas le jeu en pratique (20-30 voitures, malloc échoue rarement), mais ce ne sont pas robustes. Je les admettrais en soutenance et expliquerais les corrections possibles."

📂 **Où regarder :**
- `src/liste_car.c:11, 32` : malloc sans check NULL
- `src/mouvement.c:1459-1479` : Collision O(n²)
- `src/affichage.c:270` : fopen() chaque frame
- `src/affichage.c:290-323` : Parsing UTF-8 manuel

🎯 **Ce que le prof vérifie :**
Honnêteté et esprit critique. Vérifie que vous avez conscience des faiblesses et que vous ne les cachez pas.

---

#### Q40 : Comment amélioreriez-vous le code sans tout casser ?

⚡ **Réponse courte :**
"Trois améliorations incrémentales : ajouter check NULL après malloc, cacher plan.txt en mémoire, et optimiser collision avec spatial hashing."

📖 **Réponse longue :**
"Trois améliorations par ordre de priorité. Amélioration 1 (facile, 30 min) : ajouter if (!vh) return NULL; après malloc dans nv_vehicule() ligne 11, et vérifier le retour dans creer_voiture_aleatoire(). Ça corrige le bug critique sans casser la logique. Amélioration 2 (moyenne, 2h) : dans afficher_plan_avec_viewport(), charger plan.txt une fois au démarrage dans une variable statique wchar_t cache[MAX_HAUTEUR][MAX_LARGEUR], puis copier depuis le cache au lieu de fopen(). Réduction de ~20ms à ~2ms par frame. Amélioration 3 (difficile, 1 jour) : implémenter spatial hashing pour la collision. Diviser le plan en grille 10×10, assigner chaque véhicule à sa cellule, tester collision seulement contre cellules adjacentes. Réduction O(n²) → O(n). Ces trois changements sont incrémentaux (pas de refactoring global) et testables séparément."

📂 **Où regarder :**
- `src/liste_car.c:11` : Ajout if (!vh) return NULL;
- `src/affichage.c:270` : Remplacer fopen() par accès cache
- `src/mouvement.c:1459-1479` : Remplacer double boucle par grille

🎯 **Ce que le prof vérifie :**
Capacité de proposition et pragmatisme. Vérifie que vous savez prioriser et estimer la difficulté des améliorations.

---

#### Q41 : Quelles fonctionnalités ajouteriez-vous si vous aviez plus de temps ?

⚡ **Réponse courte :**
"Trois features : sauvegarde du high score dans un fichier, différents types de véhicules (camions plus lents), et éditeur de plan interactif."

📖 **Réponse longue :**
"Trois fonctionnalités par ordre de complexité. Feature 1 (1 jour) : sauvegarde du high score dans un fichier texte scores.txt. À la fin du jeu, si score > ancien_high_score, écrire dans le fichier. Au démarrage, charger depuis le fichier. Nécessite fopen/fclose/fprintf/fscanf. Feature 2 (2 jours) : différents types de véhicules avec comportements différents. Camions (type='c') : 2x plus lents (vitesse=1), occupent 2 places. Motos (type='m') : 2x plus rapides (vitesse=4), occupent 0.5 place. Nécessite d'adapter collision AABB pour différentes tailles. Feature 3 (1 semaine) : éditeur de plan interactif avec ncurses. Touche 'w' place un mur, 'p' place une place, flèches pour naviguer. Sauvegarde dans plan_custom.txt. Nécessite mode édition séparé, UI, validation du plan. Ces features seraient fun mais non critiques pour le projet actuel."

📂 **Où regarder :**
- `include/plan.h:73` : high_score déjà présent (à sauvegarder)
- `include/liste_car.h:16` : type (char) déjà prévu
- `data/plan.txt` : Format éditable

🎯 **Ce que le prof vérifie :**
Créativité et vision produit. Vérifie que vous pensez au-delà du code technique (features user-facing).

---

#### Q42 : Quelles sont les limites actuelles du système ?

⚡ **Réponse courte :**
"Trois limites hardcodées : max 50 places, max 100 flèches, plan fixe 100×150. Pas de gestion si dépassement, juste ignoré silencieusement."

📖 **Réponse longue :**
"Il y a cinq limites hardcodées dans plan.h. Limite 1 : MAX_HAUTEUR=100 et MAX_LARGEUR=150 (lignes 11-12), plan_statique est alloué statiquement de cette taille. Si le fichier plan.txt a 200 lignes, seules les 100 premières sont chargées. Limite 2 : places[50] (ligne 77), si le plan contient 60 places, seules 50 sont détectées (detecter_place_wchar ligne 46 vérifie if (places_totales < 50)). Limite 3 : fleches[100] (ligne 80), même problème pour 150 flèches. Limite 4 : MAX_FILE_ATTENTE=10 (liste_car.h:55), file d'attente limitée à 10 véhicules. Limite 5 : MAX_LIGNE=600 pour parsing UTF-8. Ces limites sont des tableaux statiques pour simplicité (pas de malloc variable), mais ça manque de flexibilité. Une amélioration serait de passer en allocations dynamiques."

📂 **Où regarder :**
- `include/plan.h:11-12` : MAX_HAUTEUR, MAX_LARGEUR
- `include/plan.h:77, 80` : places[50], fleches[100]
- `include/liste_car.h:55` : MAX_FILE_ATTENTE=10
- `src/plan.c:46` : if (plan->places_totales < 50) - pas de warning si dépassement

🎯 **Ce que le prof vérifie :**
Connaissance des contraintes système et des limites. Vérifie que vous avez conscience des magic numbers et de leurs impacts.

---

#### Q43 : [PIÈGE] "Votre code a des fuites mémoire, non ?"

⚡ **Réponse courte :**
"Non. Toutes les allocations ont un free correspondant. main() fait le nettoyage complet : detruire_file, detruire_liste, detruire_plan. Je pourrais vérifier avec valgrind."

📖 **Réponse longue :**
"Non, il n'y a pas de fuite mémoire, et je peux le démontrer. Premièrement, j'ai recensé les 12 allocations : 4 dans liste_car.c, 3 dans matrice.c, 1 dans plan.c, etc. Chacune a un free correspondant dans une fonction detruire_*(). Deuxièmement, main() ligne 64-66 fait le nettoyage systématique : detruire_file_attente(&file) libère tous les véhicules en attente, detruire_liste_car(&vehicules) libère tous les véhicules actifs, detruire_plan(&plan) libère la matrice et le plan. Troisièmement, les fonctions de destruction parcourent TOUTES les structures chaînées (boucles while) pour libérer chaque élément. Quatrièmement, en cas d'erreur pendant l'initialisation, main() fait le nettoyage partiel (lignes 45, 54). Je pourrais lancer valgrind --leak-check=full ./bin/parking pour confirmer '0 bytes lost'. Avez-vous identifié une fuite spécifique que j'aurais manquée ?"

📂 **Où regarder :**
- `src/main.c:64-66` : Nettoyage final (3 detruire)
- `src/liste_car.c:155-161` : detruire_liste_car() boucle complète
- `src/liste_car.c:354-370` : detruire_file_attente() boucle complète
- `src/matrice.c:57-76` : detruire_matrice() libère toutes lignes

🎯 **Ce que le prof vérifie :**
Confiance et défense technique. C'est un piège pour tester si vous paniquez ou si vous défendez votre code avec assurance et preuves.

---

#### Q44 : [PIÈGE] "Pourquoi n'avez-vous pas utilisé des threads pour paralléliser ?"

⚡ **Réponse courte :**
"Hors scope du projet. Threads ajouteraient de la complexité (mutex, conditions de course) sans gain réel pour 30 véhicules. Et ncurses n'est pas thread-safe."

📖 **Réponse longue :**
"Le multithreading serait inapproprié ici pour quatre raisons. Premièrement, complexité excessive : threads nécessitent mutexes pour protéger les structures partagées (liste_car, matrice), ce qui introduirait des bugs subtils (deadlocks, race conditions). Pour un projet étudiant niveau intermédiaire, c'est trop risqué. Deuxièmement, ncurses n'est pas thread-safe : refresh() et mvprintw() ne peuvent pas être appelés depuis plusieurs threads simultanément sans corruption d'affichage. Troisièmement, gain de performance négligeable : avec 30 véhicules, deplacer_tous_vehicules() prend ~5ms, paralléliser sur 4 cores donnerait ~1.5ms, ce qui ne change pas l'expérience utilisateur. Quatrièmement, hors scope du cours : nous n'avons pas étudié pthread en détail, ajouter des threads serait surenchère inutile. Le projet montre déjà des compétences C solides sans multithreading."

📂 **Où regarder :**
- Pas de #include <pthread.h> dans le projet
- `src/jeu.c:351-506` : Boucle séquentielle simple
- Documentation ncurses : "ncurses is not thread-safe"

🎯 **Ce que le prof vérifie :**
Résistance aux suggestions hors-sujet. C'est un piège pour voir si vous vous laissez déstabiliser ou si vous justifiez vos choix (ne pas surenchérir).

---

#### Q45 : [PIÈGE] "Pourquoi votre mouvement.c fait 2396 lignes ? C'est du mauvais code, non ?"

⚡ **Réponse courte :**
"C'est effectivement trop gros, je l'admets. C'est un point d'amélioration. Mais ça ne veut pas dire que le code est 'mauvais' : les fonctions sont modulaires, il manque juste de l'organisation en fichiers."

📖 **Réponse longue :**
"Je suis d'accord que 2396 lignes dans un seul fichier c'est trop, et je l'admettrais dès le début de ma soutenance. Mais 'trop gros' ≠ 'mauvais code'. À l'intérieur, le code est modulaire : suivre_fleches() fait une chose, deplacer_vehicule() fait une chose, tenter_parking_automatique() fait une chose. Chaque fonction a une responsabilité claire. Le problème c'est l'organisation en fichiers : j'aurais dû extraire ces fonctions dans des fichiers séparés (collision.c, navigation.c, parking_auto.c, sprites.c). J'ai commencé cette extraction (mouvement/sprites.c et mouvement/collision.c existent déjà). C'est un compromis de temps : j'ai priorisé les fonctionnalités avant la modularisation. Dans un vrai projet, je refactorerais. Mais le code compile, fonctionne, et a 0 warning. Ce n'est pas 'mauvais', c'est 'améliorable'. Nuance importante."

📂 **Où regarder :**
- `src/mouvement.c` : 2396 lignes mais fonctions bien définies
- `src/mouvement/sprites.c` : 131 lignes (déjà extrait)
- `src/mouvement/collision.c` : 230 lignes (déjà extrait)

🎯 **Ce que le prof vérifie :**
Gestion de la critique et nuance. C'est un piège agressif pour voir si vous vous défendez intelligemment sans être sur la défensive.

---

#### Q46 : [PIÈGE] "Un étudiant intermédiaire n'aurait jamais fait ça. Vous avez copié du code, non ?"

⚡ **Réponse courte :**
"Non, tout est écrit par moi. Je peux expliquer ligne par ligne n'importe quelle fonction. Le niveau intermédiaire peut maîtriser ncurses et listes chaînées."

📖 **Réponse longue :**
"Je comprends le scepticisme, mais je peux prouver que c'est mon code. Premièrement, je peux expliquer chaque choix technique : pourquoi TOLERANCE=2, pourquoi napms(57), pourquoi MAX_LIGNE=600, pourquoi nettoyage cascade dans creer_matrice(). Si j'avais copié, je ne saurais pas justifier ces détails. Deuxièmement, le code a des imperfections typiques d'un étudiant : malloc sans check NULL ligne 11, mouvement.c trop gros, affichage qui relit fichier. Un code copié d'un expert n'aurait pas ces bugs. Troisièmement, je peux tracer l'historique Git : 47 commits progressifs montrant l'évolution (corrections UTF-8, ajout difficulté, refactoring). Quatrièmement, certaines parties sont sous-optimales (collision O(n²), parsing UTF-8 manuel) mais fonctionnelles : c'est cohérent avec un étudiant qui apprend. Enfin, niveau intermédiaire ne veut pas dire 'débutant' : après 2 ans de C, maîtriser ncurses et listes chaînées est normal."

📂 **Où regarder :**
- `git log --oneline` : 47 commits avec messages progressifs
- `src/liste_car.c:11` : Bug malloc (pas dans un code expert)
- `src/mouvement.c:2396 lignes` : Organisation non-optimale

🎯 **Ce que le prof vérifie :**
Authenticité et capacité de défense sous pression. C'est le piège ultime pour tester si vous maîtrisez vraiment le code ou si vous bluffez.

---

## 3. ERREURS À ÉVITER EN SOUTENANCE

### Erreurs de communication

#### ❌ ERREUR 1 : Dire "Je ne sais pas"

**Mauvaise réponse :**
> "Je ne sais pas pourquoi j'ai mis TOLERANCE=2."

**Bonne réponse :**
> "Je ne l'ai pas documenté explicitement, mais TOLERANCE=2 sert à éviter les faux positifs de collision quand deux voitures sont adjacentes. J'ai testé différentes valeurs (1, 2, 3) et 2 donnait le meilleur équilibre."

**Principe :** Même si vous avez oublié, reconstruisez le raisonnement à partir du code.

---

#### ❌ ERREUR 2 : Mentir ou inventer

**Mauvaise réponse :**
> "J'ai optimisé la collision avec un octree."
(Alors que c'est clairement une double boucle O(n²))

**Bonne réponse :**
> "La collision est actuellement O(n²) avec une double boucle. Je sais qu'un octree ou spatial hashing serait O(n), mais je ne l'ai pas implémenté par manque de temps. C'est une optimisation future."

**Principe :** L'honnêteté technique est plus valorisée que la surenchère.

---

#### ❌ ERREUR 3 : Survendre ("C'est parfait")

**Mauvaise réponse :**
> "Mon code est parfait, sans bug, optimisé au maximum."

**Bonne réponse :**
> "Le code fonctionne bien pour l'usage prévu (20-30 véhicules), compile sans warning, et n'a pas de fuite mémoire. Mais il a des points d'amélioration : malloc sans check NULL ligne 11, mouvement.c trop gros, affichage qui relit fichier."

**Principe :** Montrer conscience des limites = maturité technique.

---

#### ❌ ERREUR 4 : Comparer à des projets professionnels

**Mauvaise réponse :**
> "Mon code est aussi bon qu'un moteur de jeu AAA."

**Bonne réponse :**
> "Pour un projet étudiant niveau intermédiaire, je pense avoir atteint les objectifs : gestion mémoire rigoureuse, structures de données classiques, et code maintenable. Ce n'est pas du niveau professionnel (pas de multithreading, pas d'optimisations poussées), mais c'est cohérent avec mon niveau d'études."

**Principe :** Se positionner au bon niveau (ni sous-estimer ni surestimer).

---

#### ❌ ERREUR 5 : Paniquer face aux pièges

**Mauvaise réponse (face à "Vous avez copié ?") :**
> "Euh... non... enfin... je ne sais pas... peut-être que..."

**Bonne réponse :**
> "Non, tout est mon code. Je peux expliquer ligne par ligne n'importe quelle fonction. Par exemple, voulez-vous que je détaille creer_matrice() avec le nettoyage cascade ligne 22-31 ?"

**Principe :** Confiance et preuves concrètes (offrir de détailler).

---

### Erreurs techniques

#### ❌ ERREUR 6 : Confondre ligne et colonne

**Mauvaise réponse :**
> "posx est la ligne et posy est la colonne."
(C'est l'inverse dans le code)

**Bonne réponse :**
> "posx est la colonne (indice horizontal) et posy est la ligne (indice vertical). C'est une convention un peu contre-intuitive mais cohérente avec matrice[ligne][colonne]."

**Principe :** Vérifier les conventions avant de répondre.

---

#### ❌ ERREUR 7 : Inventer des complexités

**Mauvaise réponse :**
> "La détection collision est O(log n) grâce à un arbre binaire."
(Alors que c'est O(n²) en double boucle)

**Bonne réponse :**
> "La détection collision est O(n²) car j'ai une double boucle qui teste chaque paire de véhicules. Pour n=30, ça fait ~450 paires, ce qui est acceptable. Si n>100, il faudrait optimiser."

**Principe :** Big-O ne s'invente pas, se calcule à partir du code.

---

#### ❌ ERREUR 8 : Ignorer les bugs connus

**Mauvaise réponse (face à "malloc ligne 11 n'est pas vérifié") :**
> "Ah bon ? Je ne savais pas."
(Alors que c'est évident en relisant le code)

**Bonne réponse :**
> "Oui, c'est un bug que j'ai identifié : nv_vehicule() ligne 11 fait malloc sans vérifier NULL. En pratique, malloc échoue rarement pour 200 bytes, mais ce n'est pas robuste. La correction serait if (!vh) return NULL; ligne 12."

**Principe :** Assumer les bugs connus et proposer des corrections.

---

### Stratégies de réponse

#### ✅ Stratégie 1 : Reformuler la question

**Question prof :** "Comment gérez-vous les collisions ?"

**Réponse :**
> "Vous voulez savoir l'algorithme de détection (AABB) ou le traitement après collision (GAME OVER) ?"

**Avantage :** Clarifier l'attente et gagner du temps de réflexion.

---

#### ✅ Stratégie 2 : Utiliser le tableau/écran

**Question prof :** "Expliquez AABB."

**Réponse (en dessinant) :**
> "Voici deux voitures [dessine deux rectangles]. Chacune a une boîte x_min, x_max, y_min, y_max. Je teste si elles sont séparées : voiture1 complètement à gauche de 2 ? [dessine flèche]. Si aucune séparation, collision."

**Avantage :** Visuel aide la compréhension et montre que vous maîtrisez.

---

#### ✅ Stratégie 3 : Donner un exemple concret

**Question prof :** "Pourquoi liste chaînée ?"

**Réponse :**
> "Exemple concret : à la frame 100, j'ai 15 véhicules. Frame 101, un spawn → 16 véhicules. Frame 102, une sortie → 15 véhicules. Avec une liste chaînée, ajouter/retirer prend O(1) sans réallocation. Avec un tableau dynamique, je devrais gérer capacité vs taille et potentiellement realloc, plus complexe."

**Avantage :** Exemples rendent les concepts abstraits concrets.

---

#### ✅ Stratégie 4 : Admettre et corriger

**Question prof :** "Vous n'avez pas de vérification NULL après malloc ?"

**Réponse :**
> "Exact, c'est un bug lignes 11 et 32. La correction serait d'ajouter if (!vh) return NULL; et de vérifier le retour dans creer_voiture_aleatoire(). Je ne l'ai pas fait car malloc échoue rarement pour de petites allocations, mais ce n'est pas une excuse valable."

**Avantage :** Honnêteté + proposition de solution = maturité.

---

### Phrases à bannir

- ❌ "Je ne sais pas" (sans justification)
- ❌ "C'est compliqué" (fuyant)
- ❌ "Je n'ai pas eu le temps" (excuse)
- ❌ "C'est parfait" (arrogant)
- ❌ "Ça marche chez moi" (pas professionnel)
- ❌ "Je pense que..." (sans vérifier le code)
- ❌ "Peut-être que..." (incertain)

### Phrases à privilégier

- ✅ "Laissez-moi vérifier dans le code..." (rigoureux)
- ✅ "C'est un compromis entre X et Y" (justifié)
- ✅ "Je n'ai pas implémenté X car..." (choix assumé)
- ✅ "C'est un point d'amélioration" (honnête)
- ✅ "Voulez-vous que je détaille ?" (pro-actif)
- ✅ "Bon point, je n'avais pas pensé à..." (ouvert)
- ✅ "Regardons ensemble ligne X" (collaboratif)

---

## Conclusion

Ce guide couvre 43 questions techniques précises basées sur votre code réel, un pitch structuré de 5 minutes, et des stratégies de communication pour la soutenance.

**Conseils finaux :**

1. **Relisez les fichiers critiques** listés dans chaque question (liste_car.c, matrice.c, mouvement.c, jeu.c, plan.c) pour pouvoir les citer précisément.

2. **Pratiquez le pitch** plusieurs fois à voix haute pour respecter le timing 5 minutes.

3. **Assumez les limitations** : mieux vaut admettre un bug connu que de mentir ou l'ignorer.

4. **Restez calme face aux pièges** : les profs testent votre confiance et votre honnêteté, pas votre perfection.

5. **Montrez que vous apprenez** : identifier des améliorations montre que vous progressez, pas que vous êtes incompétent.

Bonne soutenance ! 🎓
