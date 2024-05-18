#include "selectalgoscreen.h"
#include "algoscreen.h"
#include "dl.h"
#include <dlfcn.h>
#include <stdio.h>
#include <dirent.h>
#include <raylib.h>
#include <stdlib.h>

Screen *update_select_algo_screen(Screen *screen)
{
    SelectAlgoScreen *select_algo_screen = (SelectAlgoScreen *)screen;
    if (IsKeyPressed(KEY_DOWN))
    {
        select_algo_screen->selected_module = (select_algo_screen->selected_module + 1) % select_algo_screen->num_player_modules;
    }
    if (IsKeyPressed(KEY_UP))
    {
        select_algo_screen->selected_module = (select_algo_screen->selected_module - 1 + select_algo_screen->num_player_modules) % select_algo_screen->num_player_modules;
    }
    if (IsKeyPressed(KEY_ENTER))
    {
        PlayerModule module = select_algo_screen->player_modules[select_algo_screen->selected_module];
        Player player = {.module = module, .type = AI};
        Game *game = create_game(&player, 1);
        free(screen);
        return (Screen *)create_algorithm_screen(game);
    }
    return screen;
}

void render_select_algo_screen(Screen *screen)
{
    ClearBackground(RAYWHITE);
    DrawText("Select Algorithm", 20, 20, 40, DARKGRAY);

    SelectAlgoScreen *select_algo_screen = (SelectAlgoScreen *)screen;
    Color colour;
    for (int i = 0; i < select_algo_screen->num_player_modules; i++)
    {
        colour = i == select_algo_screen->selected_module ? RED : DARKGRAY;
        PlayerModule player_module = select_algo_screen->player_modules[i];
        DrawText(player_module.name, 20, 80 + i * 40, 20, colour);
        DrawText(player_module.description, 20, 100 + i * 40, 20, colour);
    }
}

Screen *create_select_algo_screen()
{
    SelectAlgoScreen *select_algo_screen = malloc(sizeof(SelectAlgoScreen));
    select_algo_screen->base.update = update_select_algo_screen;
    select_algo_screen->base.render = render_select_algo_screen;

    char **paths = load_library_paths("./player_modules", &select_algo_screen->num_libraries);
    PlayerModule *modules = load_player_modules(paths, select_algo_screen->num_libraries, &select_algo_screen->num_player_modules);
    select_algo_screen->player_modules = modules;
    select_algo_screen->selected_module = 0;

    return (Screen *)select_algo_screen;
}