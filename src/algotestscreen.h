#include "screen.h"
#include "game.h"

typedef struct AlgorithmTestScreen
{
    Screen base;
    Game *game;
    Vector3 possible_paths[100];
} AlgorithmTestScreen;

Screen *create_algorithm_test_screen(Game *game);
Screen *update_algorithm_test_screen(Screen *screen);
void render_algorithm_test_screen(Screen *screen);
