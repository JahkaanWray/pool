#include "screen.h"

typedef struct
{
    Screen base;
} MainMenuScreen;

Screen *create_main_menu_screen();

Screen *update_main_menu_screen(Screen *screen);

void render_main_menu_screen(Screen *screen);