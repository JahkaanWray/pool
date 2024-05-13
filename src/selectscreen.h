#include "player.h"
#include "screen.h"

typedef enum
{
    SELECT_PLAYER,
    SELECT_PLAYER_TYPE,
    SELECT_PLAYER_MODULE
} SelectMode;

typedef struct
{
    Screen base;
    char *library_paths[10];
    int num_libraries;

    Player *players;
    int num_players;
    int current_player;

    PlayerModule *player_modules;
    int num_player_modules;
    int selected_module;

    SelectMode mode;

} SelectScreen;

Screen *create_select_screen();

Screen *update_select_screen(Screen *screen);

void render_select_screen(Screen *screen);