# ============================================================================
# MAKEFILE - SIMULATEUR DE PARKING 
# ============================================================================

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
LDFLAGS = -lncursesw

# Fichiers sources (détection automatique)
SOURCES = src/main.c \
          src/liste_car.c \
          src/matrice.c \
          src/utils.c \
          src/plan.c \
          src/affichage.c \
          src/mouvement.c \
          src/mouvement/sprites.c \
          src/mouvement/collision.c \
          src/jeu.c

# Exécutable
EXEC = bin/parking



# Compiler le programme (cible par défaut)
all: $(EXEC)

# Lier l'exécutable
$(EXEC): $(SOURCES)
	@mkdir -p bin
	@echo "Compilation du simulateur de parking..."
	@$(CC) $(CFLAGS) -o $(EXEC) $(SOURCES) $(LDFLAGS)
	@echo "✓ Compilation réussie: $(EXEC)"

# Compiler et exécuter
run: $(EXEC)
	@echo "Lancement du simulateur..."
	@cd data && ../$(EXEC)

# Nettoyer
clean:
	@echo "Nettoyage..."
	@rm -rf bin obj
	@echo "✓ Nettoyage terminé"

.PHONY: all run clean
