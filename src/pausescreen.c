#include "pausescreen.h"
#include <raylib.h>
#include <stdlib.h>

Screen *create_pause_screen(Screen *prev_screen)
{
    PauseScreen *pause_screen = malloc(sizeof(PauseScreen));
    pause_screen->base.update = update_pause_screen;
    pause_screen->base.render = render_pause_screen;
    pause_screen->prev_screen = prev_screen;
    return (Screen *)pause_screen;
}

Screen *update_pause_screen(Screen *screen)
{
    PauseScreen *pause_screen = (PauseScreen *)screen;
    if (IsKeyPressed(KEY_P))
    {
        free(screen);
        return pause_screen->prev_screen;
    }
    return screen;
}

void render_pause_screen(Screen *screen)
{
    PauseScreen *pause_screen = (PauseScreen *)screen;
    pause_screen->prev_screen->render(pause_screen->prev_screen);
    ClearBackground(ColorAlpha(RAYWHITE, 0x80));
    DrawText("PAUSED", 10, 10, 20, RED);
}