all: main polytest

vector: src/vector3.c
	gcc -c src/vector3.c -lm -lraylib -g

main: src/main.c vector poly player
	gcc -o main src/main.c vector3.o  polynomial.o -lm -lraylib -lSDL2 -ldl -g

player: src/player.c
	gcc -shared -o libplayer.so src/player.c -lm -lraylib -g

main2: src/main2.c vector poly
	gcc -o main2 src/main2.c vector3.o  polynomial.o -lm -lSDL2 -g

poly: src/polynomial.c
	gcc -c src/polynomial.c -lm -g

polytest: src/polynomialtest.c poly
	gcc -o polytest src/polynomialtest.c polynomial.o -lm -g

clean:
	rm main