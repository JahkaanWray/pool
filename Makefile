CC = gcc
CFLAGS = -g -Wall -Wextra -Wpedantic 
LDFLAGS = -lm -lraylib -ldl
SHARED = -shared -fPIC

PLAYER_MODULES = ./player_modules
PLAYER_CODE_DIR = ./player_code

PLAYER_SRC = $(wildcard $(PLAYER_CODE_DIR)/*.c)
PLAYER_OBJS = $(patsubst $(PLAYER_CODE_DIR)/%.c, $(PLAYER_MODULES)/lib%.so, $(PLAYER_SRC))

all: main $(PLAYER_OBJS)

serialise.o: src/serialise.c
	$(CC) -c src/serialise.c -lm $(CFLAGS)

compare: src/compare.c game.o polynomial.o serialise.o
	$(CC) -o compare src/compare.c game.o serialise.o polynomial.o -lm -lraylib $(CFLAGS)

vector3.o: src/vector3.c
	gcc -c src/vector3.c -lm -lraylib $(CFLAGS)

game.o: src/game.c polynomial.o
	gcc -c src/game.c -lraylib -lm $(CFLAGS)

mainmenuscreen.o: src/mainmenuscreen.c
	gcc -c src/mainmenuscreen.c -lraylib -lm $(CFLAGS)

selectscreen.o: src/selectscreen.c
	gcc -c src/selectscreen.c -lraylib -lm $(CFLAGS)

selectalgoscreen.o: src/selectalgoscreen.c
	gcc -c src/selectalgoscreen.c -lraylib -lm $(CFLAGS)

algoscreen.o: src/algoscreen.c
	gcc -c src/algoscreen.c -lraylib -lm $(CFLAGS)

gameplayscreen.o: src/gameplayscreen.c
	gcc -c src/gameplayscreen.c -lraylib -lm $(CFLAGS)

pausescreen.o: src/pausescreen.c
	gcc -c src/pausescreen.c -lraylib -lm $(CFLAGS)

main: src/main.c vector3.o polynomial.o $(PLAYER_OBJS) mainmenuscreen.o selectscreen.o selectalgoscreen.o algoscreen.o gameplayscreen.o pausescreen.o game.o serialise.o
	gcc -o main src/main.c vector3.o polynomial.o mainmenuscreen.o selectscreen.o selectalgoscreen.o algoscreen.o gameplayscreen.o pausescreen.o serialise.o game.o -lm -lraylib -lSDL2 -ldl $(CFLAGS)

main2: src/main2.c
	gcc -o main2 src/main2.c -lraylib -lm $(CFLAGS)

$(PLAYER_MODULES)/lib%.so: $(PLAYER_CODE_DIR)/%.c
	$(CC) $(CFLAGS) $(SHARED) -o $@ $< $(LDFLAGS)

polynomial.o: src/polynomial.c
	gcc -c src/polynomial.c -lm $(CFLAGS)

polytest: src/polynomialtest.c polynomial.o
	gcc -o polytest src/polynomialtest.c polynomial.o -lm $(CFLAGS)

clean:
	rm -f main
	rm -f main2
	rm -f *.o *.so
	rm -f $(PLAYER_MODULES)/*.so