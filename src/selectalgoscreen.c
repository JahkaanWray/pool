#include "selectalgoscreen.h"
#include "algoscreen.h"
#include <dlfcn.h>
#include <stdio.h>
#include <dirent.h>
#include <raylib.h>

Screen *update_select_algo_screen(Screen *screen)
{
    if (IsKeyPressed(KEY_ENTER))
    {
        const char *library_path = "./player_modules/libplayerplant.so";
        void *handle = dlopen(library_path, RTLD_LAZY);
        if (handle == NULL)
        {
            fprintf(stderr, "Error loading library: %s\n", dlerror());
            exit(1);
        }
        char *name = *(char **)dlsym(handle, "name");
        if (name == NULL)
        {
            fprintf(stderr, "Error loading name: %s\n", dlerror());
            exit(1);
        }
        char *description = *(char **)dlsym(handle, "description");
        if (description == NULL)
        {
            fprintf(stderr, "Error loading description: %s\n", dlerror());
            exit(1);
        }
        void *pot_ball = dlsym(handle, "pot_ball");
        if (pot_ball == NULL)
        {
            fprintf(stderr, "Error loading pot_ball: %s\n", dlerror());
            exit(1);
        }
        PlayerModule player_module = {
            .name = name,
            .description = description,
            .pot_ball = pot_ball,
            .handle = handle,
            .library_path = library_path};
        Player player = {.module = player_module, .type = AI};
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
    for (int i = 0; i < select_algo_screen->num_player_modules; i++)
    {
        PlayerModule player_module = select_algo_screen->player_modules[i];
        DrawText(player_module.name, 20, 80 + i * 40, 20, DARKGRAY);
        DrawText(player_module.description, 20, 100 + i * 40, 20, DARKGRAY);
    }
}

Screen *create_select_algo_screen()
{
    SelectAlgoScreen *select_algo_screen = malloc(sizeof(SelectAlgoScreen));
    select_algo_screen->base.update = update_select_algo_screen;
    select_algo_screen->base.render = render_select_algo_screen;

    return (Screen *)select_algo_screen;
}