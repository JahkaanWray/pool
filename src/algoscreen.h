#include "screen.h"
#include "game.h"

typedef struct AlgorithmScreen
{
    Screen base;
    Game *game;
} AlgorithmScreen;

Screen *create_algorithm_screen(Game *game);
Screen *update_algorithm_screen(Screen *screen);
void render_algorithm_screen(Screen *screen);