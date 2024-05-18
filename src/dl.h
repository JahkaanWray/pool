#include "player.h"

char **load_library_paths(const char *directory_path, int *num_paths);

PlayerModule *load_player_modules(char **paths, int num_paths, int *num_player_modules);