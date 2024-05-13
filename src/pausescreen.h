#include "screen.h"

typedef struct
{
    Screen base;
    Screen *prev_screen;
} PauseScreen;

Screen *create_pause_screen(Screen *prev_screen);
Screen *update_pause_screen(Screen *screen);
void render_pause_screen(Screen *screen);