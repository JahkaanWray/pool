#include <stdlib.h>
#include "player.h"

char *name = "Random Player";
char *description = "This player hits the cue ball in a random direction.";

void pot_ball(Game *game, Ball *ball)
{
    game->v = (Vector3){rand() % 200 - 100, rand() % 200 - 100, 0};
    game->w = (Vector3){0, 0, 0};
    (void)ball;
}