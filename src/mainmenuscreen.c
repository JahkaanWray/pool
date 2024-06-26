#include "mainmenuscreen.h"
#include "selectscreen.h"
#include "selectalgoscreen.h"
#include "algotestscreen.h"
#include "player.h"
#include <raylib.h>
#include <stdlib.h>

Screen *create_main_menu_screen()
{
    MainMenuScreen *screen = (MainMenuScreen *)malloc(sizeof(MainMenuScreen));
    screen->base.update = update_main_menu_screen;
    screen->base.render = render_main_menu_screen;
    return (Screen *)screen;
}

Screen *update_main_menu_screen(Screen *screen)
{
    if (IsKeyPressed(KEY_ESCAPE))
    {
        free(screen);
        return NULL;
    }
    if (IsKeyPressed(KEY_ENTER))
    {
        SelectScreen *select_screen = (SelectScreen *)create_select_screen();
        free(screen);
        return (Screen *)select_screen;
    }
    if (IsKeyPressed(KEY_A))
    {
        SelectAlgoScreen *select_algo_screen = (SelectAlgoScreen *)create_select_algo_screen();
        free(screen);
        return (Screen *)select_algo_screen;
    }
    if (IsKeyPressed(KEY_B))
    {
        Player player = {.type = HUMAN};
        Game *game = create_game(&player, 1);
        AlgorithmTestScreen *algo_test_screen = (AlgorithmTestScreen *)create_algorithm_test_screen(game);
        free(screen);
        return (Screen *)algo_test_screen;
    }
    return screen;
}

void render_main_menu_screen(Screen *screen)
{
    (void)screen;
    ClearBackground(RAYWHITE);
    DrawText("Main Menu", 190, 200, 20, DARKGRAY);
    DrawText("Press Enter to start", 180, 220, 20, DARKGRAY);
}
