#include "mainmenuscreen.h"
#include "algoscreen.h"
#include "dlfcn.h"
#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>

Screen *create_algorithm_screen(Game *game)
{
    AlgorithmScreen *screen = malloc(sizeof(AlgorithmScreen));
    screen->base.update = update_algorithm_screen;
    screen->base.render = render_algorithm_screen;
    screen->game = game;
    screen->current_ball = 0;
    return (Screen *)screen;
}

Screen *update_algorithm_screen(Screen *screen)
{

    AlgorithmScreen *algo_screen = (AlgorithmScreen *)screen;
    if (IsKeyPressed(KEY_ESCAPE))
    {
        clear_paths(&algo_screen->game->scene);
        for (int i = 0; i < algo_screen->game->num_players; i++)
        {
            if (algo_screen->game->players[i].module.handle != NULL)
            {
                dlclose(algo_screen->game->players[i].module.handle);
            }
        }
        free(screen);
        return (Screen *)create_main_menu_screen();
    }
    if (IsKeyPressed(KEY_SPACE))
    {
        algo_screen->current_ball = (algo_screen->current_ball + 1) % algo_screen->game->scene.ball_set.num_balls;
    }
    Game *game = algo_screen->game;
    Ball *ball = &game->scene.ball_set.balls[algo_screen->current_ball];
    Vector2 mouse_position = GetMousePosition();
    ball->initial_position = (Vector3){mouse_position.x, mouse_position.y, 0};
    ball->initial_position = Vector3Scale(ball->initial_position, 1.0 / 200);
    Ball *target_ball = &game->scene.ball_set.balls[1];
    algo_screen->game->players[0].module.pot_ball(algo_screen->game, target_ball);
    clear_paths(&game->scene);
    generate_shot(game, game->v, game->w);
    return screen;
}

void render_algorithm_screen(Screen *screen)
{
    AlgorithmScreen *algo_screen = (AlgorithmScreen *)screen;
    ClearBackground(GREEN);
    render_game(algo_screen->game);
}