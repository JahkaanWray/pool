CC = gcc
CFLAGS = -g -Wall -Wextra -Wpedantic 
LDFLAGS = -lm -lraylib -ldl
SHARED = -shared -fPIC

PLAYER_MODULES = ./player_modules
PLAYER_CODE_DIR = ./player_code

PLAYER_SRC = $(wildcard $(PLAYER_CODE_DIR)/*.c)
PLAYER_OBJS = $(patsubst $(PLAYER_CODE_DIR)/%.c, $(PLAYER_MODULES)/lib%.so, $(PLAYER_SRC))

all: main $(PLAYER_OBJS)

compare: src/compare.c game poly
	$(CC) -o compare src/compare.c game.o polynomial.o -lm -lraylib $(CFLAGS)

vector: src/vector3.c
	gcc -c src/vector3.c -lm -lraylib $(CFLAGS)

game: src/game.c poly
	gcc -c src/game.c -lraylib -lm $(CFLAGS)

mainmenuscreen: src/mainmenuscreen.c
	gcc -c src/mainmenuscreen.c -lraylib -lm $(CFLAGS)

selectscreen: src/selectscreen.c
	gcc -c src/selectscreen.c -lraylib -lm $(CFLAGS)

gameplayscreen: src/gameplayscreen.c
	gcc -c src/gameplayscreen.c -lraylib -lm $(CFLAGS)

pausescreen: src/pausescreen.c
	gcc -c src/pausescreen.c -lraylib -lm $(CFLAGS)

main: src/main.c vector poly $(PLAYER_OBJS) mainmenuscreen selectscreen gameplayscreen pausescreen game
	gcc -o main src/main.c vector3.o polynomial.o mainmenuscreen.o selectscreen.o gameplayscreen.o pausescreen.o game.o -lm -lraylib -lSDL2 -ldl $(CFLAGS)

main2: src/main2.c
	gcc -o main2 src/main2.c -lraylib -lm $(CFLAGS)

$(PLAYER_MODULES)/lib%.so: $(PLAYER_CODE_DIR)/%.c
	$(CC) $(CFLAGS) $(SHARED) -o $@ $< $(LDFLAGS)

playerbankkick: src/playerbankkick.c
	gcc -shared -o ./player_modules/libplayerbankkick.so src/playerbankkick.c -lm -lraylib $(CFLAGS)

poly: src/polynomial.c
	gcc -c src/polynomial.c -lm $(CFLAGS)

polytest: src/polynomialtest.c poly
	gcc -o polytest src/polynomialtest.c polynomial.o -lm $(CFLAGS)

clean:
	rm main