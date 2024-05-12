all: main polytest

vector: src/vector3.c
	gcc -c src/vector3.c -lm -lraylib -g

main: src/main.c vector poly player1 player2
	gcc -o main src/main.c vector3.o  polynomial.o -lm -lraylib -lSDL2 -ldl -g

player1: src/player1.c
	gcc -shared -o ./player_modules/libplayer1.so src/player1.c -lm -lraylib -g

player2: src/player2.c
	gcc -shared -o ./player_modules/libplayer2.so src/player2.c -lm -lraylib -g

main2: src/main2.c vector poly
	gcc -o main2 src/main2.c vector3.o  polynomial.o -lm -lSDL2 -g

poly: src/polynomial.c
	gcc -c src/polynomial.c -lm -g

polytest: src/polynomialtest.c poly
	gcc -o polytest src/polynomialtest.c polynomial.o -lm -g

clean:
	rm main