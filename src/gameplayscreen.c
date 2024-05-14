#include "gameplayscreen.h"
#include <raylib.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include "mainmenuscreen.h"
#include "pausescreen.h"
#include "game.h"

void reload_player_modules(Game *game)
{
    printf("Reloading player modules\n");
    for (int i = 0; i < game->num_players; i++)
    {
        printf("Reloading player %d\n", i);
        Player *player = &game->players[i];
        if (player->module.handle != NULL)
        {
            printf("Closing handle\n");
            dlclose(player->module.handle);
        }
        void *library = dlopen(player->module.library_path, RTLD_LAZY);
        printf("Loading library: %s\n", player->module.library_path);
        if (library == NULL)
        {
            fprintf(stderr, "dlopen failed: %s\n", dlerror());
            continue;
        }
        player->module.handle = library;
        void *pot_ball = dlsym(player->module.handle, "pot_ball");
        player->module.pot_ball = (PlayerPotBallFunction)pot_ball;

        printf("Loading function\n");
        printf("%p\n", pot_ball);
        if (pot_ball == NULL)
        {
            fprintf(stderr, "dlsym failed: %s\n", dlerror());
            continue;
        }
    }
}

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
    if (IsKeyPressed(KEY_P))
    {
        return create_pause_screen(screen);
    }
    if (IsKeyPressed(KEY_R))
    {
        reload_player_modules(&gameplay_screen->game);
    }
    return screen;
}

void render_gameplay_screen(Screen *screen)
{
    GameplayScreen *gameplay_screen = (GameplayScreen *)screen;
    render_game(&gameplay_screen->game);
}