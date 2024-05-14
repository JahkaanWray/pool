#include "player.h"
#include <stdlib.h>

char *name = "Random Player 2";
char *description = "Second ramdom player";

void pot_ball(Game *game, Ball *ball)
{
    game->v = (Vector3){rand() % 200 - 100, rand() % 200 - 100, 0};
    game->w = (Vector3){0, 0, 0};
    (void)ball;
}