all: main

vector: src/vector3.c
	gcc -c src/vector3.c -lm -g

main: src/main.c vector
	gcc -o main src/main.c vector3.o -lm -lSDL2 -g

clean:
	rm main