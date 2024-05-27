#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <complex.h>
#include <dlfcn.h>
#include "polynomial.h"
#include "screen.h"
#include "mainmenuscreen.h"
#include "selectscreen.h"
#include "gameplayscreen.h"
#include "game.h"
#include "player.h"

typedef struct App
{
    Screen *current_screen;
} App;

Screen *update_app(App *app)
{
    return app->current_screen->update(app->current_screen);
}

void render_app(App *app)
{
    BeginDrawing();
    app->current_screen->render(app->current_screen);
    EndDrawing();
}

void init_app(App *app)
{
    app->current_screen = create_main_menu_screen();
}

int main()
{
    const int SCREEN_WIDTH = 1640;
    const int SCREEN_HEIGHT = 900;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pool");

    SetExitKey(KEY_NULL);

    SetTargetFPS(60);
    App app;
    init_app(&app);

    while (!WindowShouldClose())
    {
        app.current_screen = update_app(&app);
        if (app.current_screen == NULL)
        {
            break;
        }
        render_app(&app);
    }

    CloseWindow();
    return 0;
}