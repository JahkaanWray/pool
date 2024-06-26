#include "screen.h"
#include "game.h"

typedef struct GameplayScreen
{
    Screen base;

    Game game;
} GameplayScreen;

Screen *create_gameplay_screen(Player *players, int num_players);

Screen *update_gameplay_screen(Screen *screen);

void render_gameplay_screen(Screen *screen);