#include "selectscreen.h"
#include "mainmenuscreen.h"
#include "gameplayscreen.h"
#include <raylib.h>
#include <stdio.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

void load_player_libraries(SelectScreen *select_screen)
{
    for (int i = 0; i < select_screen->num_libraries; i++)
    {
        const char *library_folder = "./player_modules/";
        char *library_path = select_screen->library_paths[i];
        char *full_path = malloc(strlen(library_folder) + strlen(library_path) + 1);
        strcpy(full_path, library_folder);
        strcat(full_path, library_path);
        printf("Loading library: %s\n", full_path);
        void *library = dlopen(full_path, RTLD_LAZY);
        if (!library)
        {
            fprintf(stderr, "dlopen failed: %s\n", dlerror());
            free(full_path);
            continue;
        }
        void *pot_ball = dlsym(library, "pot_ball");
        printf("Loading function\n");
        if (!pot_ball)
        {
            fprintf(stderr, "dlsym failed: %s\n", dlerror());
            free(full_path);
            continue;
        }
        char *name = *(char **)dlsym(library, "name");
        printf("Loading name\n");
        if (name == NULL)
        {
            fprintf(stderr, "dlsym failed: %s\n", dlerror());
            free(full_path);
            continue;
        }
        char *description = *(char **)dlsym(library, "description");
        printf("Loading description\n");
        if (description == NULL)
        {
            fprintf(stderr, "dlsym failed: %s\n", dlerror());
            free(full_path);
            continue;
        }
        printf("%s\n", description);

        PlayerModule player_module = {
            .name = name,
            .description = description,
            .pot_ball = pot_ball,
            .handle = library,
            .library_path = full_path};

        select_screen->player_modules[select_screen->num_player_modules++] = player_module;
    }
}

void load_library_paths(SelectScreen *select_screen)
{
    DIR *dir;
    struct dirent *ent;
    int i = 0;
    const char *current_dir = ".";
    const char *parent_dir = "..";
    if ((dir = opendir("./player_modules")) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            char *name = ent->d_name;
            if (strcmp(name, current_dir) == 0 || strcmp(name, parent_dir) == 0)
            {
                continue;
            }
            printf("%s\n", name);
            select_screen->library_paths[i++] = name;
            select_screen->num_libraries++;
        }
        closedir(dir);
    }
    else
    {
        perror("");
        return;
    }
}

Screen *create_select_screen()
{
    SelectScreen *screen = malloc(sizeof(SelectScreen));
    screen->base.update = update_select_screen;
    screen->base.render = render_select_screen;

    screen->num_libraries = 0;
    screen->num_players = 2;
    screen->players = malloc(screen->num_players * sizeof(Player));
    screen->current_player = 0;
    screen->mode = SELECT_PLAYER;
    screen->player_modules = malloc(10 * sizeof(PlayerModule));
    screen->num_player_modules = 0;
    screen->selected_module = 0;
    load_library_paths(screen);
    load_player_libraries(screen);
    for (int i = 0; i < screen->num_players; i++)
    {
        screen->players[i].type = HUMAN;
        screen->players[i].module = screen->player_modules[0];
    }

    return (Screen *)screen;
}

Screen *update_select_screen(Screen *screen)
{
    SelectScreen *select_screen = (SelectScreen *)screen;
    if (IsKeyPressed(KEY_ENTER))
    {
        if (select_screen->mode == SELECT_PLAYER)
        {
            select_screen->mode = SELECT_PLAYER_TYPE;
        }
        else if (select_screen->mode == SELECT_PLAYER_TYPE)
        {
            if (select_screen->players[select_screen->current_player].type == AI)
            {

                select_screen->mode = SELECT_PLAYER_MODULE;
            }
            else
            {
                select_screen->mode = SELECT_PLAYER;
            }
        }
        else if (select_screen->mode == SELECT_PLAYER_MODULE)
        {
            select_screen->mode = SELECT_PLAYER;
        }
    }
    if (IsKeyPressed(KEY_UP))
    {
        SelectMode mode = select_screen->mode;
        if (mode == SELECT_PLAYER)
        {
            select_screen->current_player = (select_screen->current_player + 1) % select_screen->num_players;
        }
        else if (mode == SELECT_PLAYER_TYPE)
        {
            if (select_screen->players[select_screen->current_player].type == HUMAN)
            {
                select_screen->players[select_screen->current_player].type = AI;
            }
            else
            {
                select_screen->players[select_screen->current_player].type = HUMAN;
            }
        }
        else if (mode == SELECT_PLAYER_MODULE)
        {
            select_screen->selected_module = (select_screen->selected_module + 1) % select_screen->num_player_modules;
            select_screen->players[select_screen->current_player].module = select_screen->player_modules[select_screen->selected_module];
        }
    }
    if (IsKeyPressed(KEY_SPACE))
    {
        printf("Space pressed\n");
        GameplayScreen *gameplay_screen = (GameplayScreen *)create_gameplay_screen(select_screen->players, select_screen->num_players);
        return (Screen *)gameplay_screen;
    }
    return screen;
}

void render_select_screen(Screen *screen)
{
    SelectScreen *select_screen = (SelectScreen *)screen;
    ClearBackground(RAYWHITE);
    DrawText("2 Player Select", 190, 200, 20, DARKGRAY);
    Color colour;
    for (int i = 0; i < select_screen->num_players; i++)
    {
        Player player = select_screen->players[i];
        colour = i == select_screen->current_player ? RED : DARKGRAY;
        DrawText(TextFormat("Player %d: %s", i + 1, player.type == HUMAN ? "HUMAN" : "AI"), 190, 220 + i * 60, 20, colour);
        if (player.type == AI && player.module.name != NULL)
        {
            DrawText(TextFormat("Module: %s", player.module.name), 190, 240 + i * 60, 20, DARKGRAY);
        }
    }
}