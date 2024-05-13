all: main polytest

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

main: src/main.c vector poly player1 player2 player3 player4 mainmenuscreen selectscreen gameplayscreen pausescreen game
	gcc -o main src/main.c vector3.o polynomial.o mainmenuscreen.o selectscreen.o gameplayscreen.o pausescreen.o game.o -lm -lraylib -lSDL2 -ldl -g

main2: src/main2.c
	gcc -o main2 src/main2.c -lraylib -lm -g

player1: src/player1.c
	gcc -shared -o ./player_modules/libplayer1.so src/player1.c -lm -lraylib -g

player2: src/player2.c
	gcc -shared -o ./player_modules/libplayer2.so src/player2.c -lm -lraylib -g

player3: src/player3.c
	gcc -shared -o ./player_modules/libplayer3.so src/player3.c -lm -lraylib -g

player4: src/player4.c
	gcc -shared -o ./player_modules/libplayer4.so src/player4.c -lm -lraylib -g

poly: src/polynomial.c
	gcc -c src/polynomial.c -lm -g

polytest: src/polynomialtest.c poly
	gcc -o polytest src/polynomialtest.c polynomial.o -lm -g

clean:
	rm main