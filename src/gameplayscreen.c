#include "gameplayscreen.h"
#include <raylib.h>
#include <stdlib.h>
#include "mainmenuscreen.h"
#include "game.h"

Screen *create_gameplay_screen(Player *players, int num_players)
{
    GameplayScreen *gameplay_screen = malloc(sizeof(GameplayScreen));
    gameplay_screen->base.update = update_gameplay_screen;
    gameplay_screen->base.render = render_gameplay_screen;

    gameplay_screen->game = *create_game(players, num_players);

    return (Screen *)gameplay_screen;
}

Screen *update_gameplay_screen(Screen *screen)
{
    GameplayScreen *gameplay_screen = (GameplayScreen *)screen;
    update_game(&gameplay_screen->game);
    if (IsKeyPressed(KEY_ESCAPE))
    {
        return (Screen *)create_main_menu_screen();
    }
    return screen;
}

void render_gameplay_screen(Screen *screen)
{
    GameplayScreen *gameplay_screen = (GameplayScreen *)screen;
    render_game(&gameplay_screen->game);
}