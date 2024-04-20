main: src/main.c
	gcc -o main src/main.c -lm -lSDL2 -g

clean:
	rm main