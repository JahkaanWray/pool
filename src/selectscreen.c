#include "selectscreen.h"
#include "mainmenuscreen.h"
#include "gameplayscreen.h"
#include "dl.h"
#include <raylib.h>
#include <stdio.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

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
    char **paths = load_library_paths("./player_modules", &screen->num_libraries);
    PlayerModule *modules = load_player_modules(paths, screen->num_libraries, &screen->num_player_modules);
    screen->player_modules = modules;
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