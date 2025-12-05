# ============================================================================
# PROJET SIMULATEUR DE PARKING - ESIEA
# ============================================================================
# Makefile organisé avec structure de répertoires propre
# ============================================================================

# Compilateur et flags
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude -IUnity/src
LDFLAGS =

# Répertoires
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin
DATA_DIR = data
TEST_DIR = tests

# Fichiers sources du projet principal
SRC_MAIN = $(SRC_DIR)/main.c \
           $(SRC_DIR)/liste_car.c \
           $(SRC_DIR)/matrice.c \
           $(SRC_DIR)/plan.c \
           $(SRC_DIR)/affichage.c

# Fichiers objets (générés automatiquement depuis SRC_MAIN)
OBJ_MAIN = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_MAIN))

# Fichiers de test
SRC_TEST_LISTE = $(TEST_DIR)/test_liste.c $(SRC_DIR)/liste_car.c Unity/src/unity.c
SRC_TEST_ENCODAGE = $(TEST_DIR)/test_encodage.c
SRC_TEST_FINAL = $(TEST_DIR)/test_final.c

# Exécutables
EXEC_MAIN = $(BIN_DIR)/parking
EXEC_TEST_LISTE = $(BIN_DIR)/test_liste
EXEC_TEST_ENCODAGE = $(BIN_DIR)/test_encodage
EXEC_TEST_FINAL = $(BIN_DIR)/test_final

# ============================================================================
# RÈGLES PRINCIPALES
# ============================================================================

# Cible par défaut : compile le programme principal
all: $(EXEC_MAIN)

# Compiler et exécuter le programme principal
run: $(EXEC_MAIN)
	@echo "=========================================="
	@echo "   Lancement du simulateur de parking"
	@echo "=========================================="
	@cd $(DATA_DIR) && ../$(EXEC_MAIN)

# Compiler tous les tests
tests: $(EXEC_TEST_LISTE) $(EXEC_TEST_ENCODAGE) $(EXEC_TEST_FINAL)
	@echo "✓ Tous les tests ont été compilés"

# Exécuter les tests unitaires
test: $(EXEC_TEST_LISTE)
	@echo "=========================================="
	@echo "   Exécution des tests unitaires"
	@echo "=========================================="
	./$(EXEC_TEST_LISTE)

# Test d'encodage UTF-8
test-encodage: $(EXEC_TEST_ENCODAGE)
	@echo "=========================================="
	@echo "   Test d'encodage UTF-8"
	@echo "=========================================="
	@cd $(DATA_DIR) && ../$(EXEC_TEST_ENCODAGE)

# Test final
test-final: $(EXEC_TEST_FINAL)
	@echo "=========================================="
	@echo "   Test final"
	@echo "=========================================="
	@cd $(DATA_DIR) && ../$(EXEC_TEST_FINAL)

# ============================================================================
# RÈGLES DE COMPILATION
# ============================================================================

# Programme principal
$(EXEC_MAIN): $(OBJ_MAIN) | $(BIN_DIR)
	@echo "[LINK] $@"
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Test liste chaînée
$(EXEC_TEST_LISTE): $(SRC_TEST_LISTE) | $(BIN_DIR)
	@echo "[BUILD] $@"
	@$(CC) $(CFLAGS) -o $@ $^

# Test encodage
$(EXEC_TEST_ENCODAGE): $(SRC_TEST_ENCODAGE) | $(BIN_DIR)
	@echo "[BUILD] $@"
	@$(CC) $(CFLAGS) -o $@ $^

# Test final
$(EXEC_TEST_FINAL): $(SRC_TEST_FINAL) | $(BIN_DIR)
	@echo "[BUILD] $@"
	@$(CC) $(CFLAGS) -o $@ $^

# Règle générique pour compiler les fichiers .c en .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# ============================================================================
# CRÉATION DES RÉPERTOIRES
# ============================================================================

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# ============================================================================
# NETTOYAGE
# ============================================================================

# Nettoyer les fichiers compilés
clean:
	@echo "Nettoyage des fichiers compilés..."
	@rm -rf $(OBJ_DIR)/*.o
	@rm -f $(EXEC_MAIN) $(EXEC_TEST_LISTE) $(EXEC_TEST_ENCODAGE) $(EXEC_TEST_FINAL)
	@echo "✓ Nettoyage terminé"

# Nettoyer complètement (y compris les répertoires)
distclean: clean
	@echo "Nettoyage complet..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "✓ Nettoyage complet terminé"

# ============================================================================
# UTILITAIRES
# ============================================================================

# Afficher la structure du projet
tree:
	@echo "Structure du projet:"
	@tree -L 2 -I 'Unity' --dirsfirst || ls -R

# Afficher les informations de compilation
info:
	@echo "=========================================="
	@echo "   INFORMATIONS DE COMPILATION"
	@echo "=========================================="
	@echo "Compilateur: $(CC)"
	@echo "Flags: $(CFLAGS)"
	@echo "Sources: $(SRC_MAIN)"
	@echo "Objets: $(OBJ_MAIN)"
	@echo "Exécutable: $(EXEC_MAIN)"
	@echo "=========================================="

# Afficher l'aide
help:
	@echo "=========================================="
	@echo "   MAKEFILE - SIMULATEUR DE PARKING"
	@echo "=========================================="
	@echo "Cibles disponibles:"
	@echo ""
	@echo "  make           - Compile le projet"
	@echo "  make run       - Compile et exécute le programme"
	@echo "  make tests     - Compile tous les tests"
	@echo "  make test      - Exécute les tests unitaires"
	@echo "  make clean     - Nettoie les fichiers compilés"
	@echo "  make distclean - Nettoie complètement"
	@echo "  make info      - Affiche les infos de compilation"
	@echo "  make help      - Affiche cette aide"
	@echo ""
	@echo "Tests disponibles:"
	@echo "  make test-encodage  - Test d'encodage UTF-8"
	@echo "  make test-final     - Test final"
	@echo "=========================================="

# ============================================================================
# DÉPENDANCES
# ============================================================================

# Les fichiers objets dépendent des headers
$(OBJ_DIR)/main.o: $(INC_DIR)/liste_car.h $(INC_DIR)/matrice.h $(INC_DIR)/plan.h
$(OBJ_DIR)/affichage.o: $(INC_DIR)/affichage.h $(INC_DIR)/plan.h $(INC_DIR)/liste_car.h
$(OBJ_DIR)/liste_car.o: $(INC_DIR)/liste_car.h
$(OBJ_DIR)/matrice.o: $(INC_DIR)/matrice.h
$(OBJ_DIR)/plan.o: $(INC_DIR)/plan.h $(INC_DIR)/matrice.h $(INC_DIR)/liste_car.h

# ============================================================================
# PHONY TARGETS (cibles qui ne sont pas des fichiers)
# ============================================================================

.PHONY: all run tests test test-encodage test-final clean distclean tree info help
