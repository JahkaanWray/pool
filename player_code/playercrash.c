#include "player.h"

char *name = "Crashing Player";
char *description = "This player dereferences a null pointer, causing a crash.";

void pot_ball(Game *game, Ball *ball)
{
    *(int *)0 = 0;
    return;
}