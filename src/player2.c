#include "player.h"

char *description = "This is a dummy player that just hits the cue ball in the same direction every time.";

void pot_ball(Game *game, Ball *ball)
{
    game->v = (Vector3){100, 0, 0};
    game->w = (Vector3){0, 0, 0};
}