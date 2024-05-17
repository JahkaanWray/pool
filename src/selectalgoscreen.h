#include "screen.h"
#include "player.h"

typedef struct SelectAlgoScreen
{
    Screen base;
    int num_libraries;
    char *library_paths[10];
    PlayerModule player_modules[10];
    int num_player_modules;
} SelectAlgoScreen;

Screen *create_select_algo_screen();
Screen *update_select_algo_screen(Screen *screen);
void render_select_algo_screen(Screen *screen);