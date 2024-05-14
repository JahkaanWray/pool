CC = gcc
CFLAGS = -g -Wall -Wextra 
LDFLAGS = -lm -lraylib -ldl
SHARED = -shared -fPIC

PLAYER_MODULES = ./player_modules
PLAYER_CODE_DIR = ./player_code

PLAYER_SRC = $(wildcard $(PLAYER_CODE_DIR)/*.c)
PLAYER_OBJS = $(patsubst $(PLAYER_CODE_DIR)/%.c, $(PLAYER_MODULES)/lib%.so, $(PLAYER_SRC))

all: main $(PLAYER_OBJS)

compare: src/compare.c
	$(CC) -o compare src/compare.c game.o polynomial.o -lm -lraylib -g

vector: src/vector3.c
	gcc -c src/vector3.c -lm -lraylib -g

game: src/game.c poly
	gcc -c src/game.c -lraylib -lm -g

mainmenuscreen: src/mainmenuscreen.c
	gcc -c src/mainmenuscreen.c -lraylib -lm -g

selectscreen: src/selectscreen.c
	gcc -c src/selectscreen.c -lraylib -lm -g

gameplayscreen: src/gameplayscreen.c
	gcc -c src/gameplayscreen.c -lraylib -lm -g

pausescreen: src/pausescreen.c
	gcc -c src/pausescreen.c -lraylib -lm -g

main: src/main.c vector poly $(PLAYER_OBJS) mainmenuscreen selectscreen gameplayscreen pausescreen game
	gcc -o main src/main.c vector3.o polynomial.o mainmenuscreen.o selectscreen.o gameplayscreen.o pausescreen.o game.o -lm -lraylib -lSDL2 -ldl -g

main2: src/main2.c
	gcc -o main2 src/main2.c -lraylib -lm -g

$(PLAYER_MODULES)/lib%.so: $(PLAYER_CODE_DIR)/%.c
	$(CC) $(CFLAGS) $(SHARED) -o $@ $< $(LDFLAGS)

playerbankkick: src/playerbankkick.c
	gcc -shared -o ./player_modules/libplayerbankkick.so src/playerbankkick.c -lm -lraylib -g

poly: src/polynomial.c
	gcc -c src/polynomial.c -lm -g

polytest: src/polynomialtest.c poly
	gcc -o polytest src/polynomialtest.c polynomial.o -lm -g

clean:
	rm main