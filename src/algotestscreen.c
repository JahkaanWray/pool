#include "algotestscreen.h"
#include "mainmenuscreen.h"
#include <stdlib.h>

Screen *create_algorithm_test_screen(Game *game)
{
    AlgorithmTestScreen *screen = malloc(sizeof(AlgorithmTestScreen));
    screen->game = game;
    screen->base.update = update_algorithm_test_screen;
    screen->base.render = render_algorithm_test_screen;
    return (Screen *)screen;
}

Screen *update_algorithm_test_screen(Screen *screen)
{
    AlgorithmTestScreen *algo_screen = (AlgorithmTestScreen *)screen;
    if (IsKeyPressed(KEY_ESCAPE))
    {
        free(screen);
        return (Screen *)create_main_menu_screen();
    }

    for (int i = 0; i < 100; i++)
    {
        Vector2 m = GetMousePosition();
        Vector3 v = Vector3Subtract((Vector3){m.x, m.y, 0}, Vector3Scale(algo_screen->game->scene.ball_set.balls[0].initial_position, 200));
        v = Vector3Normalize(v);
        v = Vector3Scale(v, i * 0.05);

        clear_paths(&algo_screen->game->scene);
        generate_shot(algo_screen->game, v, (Vector3){0, 0, 0});
        Path path = algo_screen->game->current_shot.ball_paths[0];
        algo_screen->possible_paths[i] = path.segments[path.num_segments - 1].initial_position;
    }
    update_game(algo_screen->game);
    return screen;
}

void render_algorithm_test_screen(Screen *screen)
{
    ClearBackground(GREEN);
    render_game(((AlgorithmTestScreen *)screen)->game);
    for (int i = 0; i < 99; i++)
    {
        Vector3 p1 = ((AlgorithmTestScreen *)screen)->possible_paths[i];
        DrawCircle(p1.x * 200, p1.y * 200, 5, RED);
    }
}