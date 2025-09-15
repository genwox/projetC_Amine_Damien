CC = gcc
CFLAGS = -Wall -Wextra -g -IUnity/src -I.

# Projet principal
SRC_MAIN = main.c liste_car.c matrice.c
OUT_MAIN = prog

# Tests
SRC_TEST = test/test_liste.c liste_car.c Unity/src/unity.c
OUT_TEST = test_liste

all: $(OUT_MAIN)

$(OUT_MAIN): $(SRC_MAIN)
	$(CC) $(CFLAGS) -o $(OUT_MAIN) $(SRC_MAIN)

run: $(OUT_MAIN)
	./$(OUT_MAIN)

test: $(OUT_TEST)
	./$(OUT_TEST)

$(OUT_TEST): $(SRC_TEST)
	$(CC) $(CFLAGS) -o $(OUT_TEST) $(SRC_TEST)

clean:
	rm -f $(OUT_MAIN) $(OUT_TEST)