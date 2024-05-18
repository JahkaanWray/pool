#include "dl.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>

char **load_library_paths(const char *directory_path, int *num_paths)
{
    DIR *dir;
    struct dirent *ent;
    const char *current_dir = ".";
    const char *parent_dir = "..";
    int count = 0;
    if ((dir = opendir(directory_path)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            char *name = ent->d_name;
            if (strcmp(name, current_dir) == 0 || strcmp(name, parent_dir) == 0)
            {
                continue;
            }
            printf("%s\n", name);
            count++;
        }
    }

    *num_paths = count;
    closedir(dir);

    char **paths = malloc(count * sizeof(char *));

    if ((dir = opendir(directory_path)) != NULL)
    {
        int i = 0;
        while ((ent = readdir(dir)) != NULL)
        {
            char *name = ent->d_name;
            if (strcmp(name, current_dir) == 0 || strcmp(name, parent_dir) == 0)
            {
                continue;
            }
            paths[i] = malloc(strlen(name) + strlen(directory_path) + 2);
            strcpy(paths[i], directory_path);
            strcat(paths[i], "/");
            strcat(paths[i], name);
            i++;
        }
    }

    return paths;
}

PlayerModule *load_player_modules(char **paths, int num_paths, int *num_player_modules)
{
    PlayerModule *player_modules = malloc(num_paths * sizeof(PlayerModule));
    int count = 0;
    for (int i = 0; i < num_paths; i++)
    {
        void *handle = dlopen(paths[i], RTLD_LAZY);
        if (handle == NULL)
        {
            fprintf(stderr, "Error loading library: %s\n", dlerror());
            continue;
        }
        char *name = *(char **)dlsym(handle, "name");
        if (name == NULL)
        {
            fprintf(stderr, "Error loading name: %s\n", dlerror());
            continue;
        }
        char *description = *(char **)dlsym(handle, "description");
        if (description == NULL)
        {
            fprintf(stderr, "Error loading description: %s\n", dlerror());
            continue;
        }
        void *pot_ball = dlsym(handle, "pot_ball");
        if (pot_ball == NULL)
        {
            fprintf(stderr, "Error loading pot_ball: %s\n", dlerror());
            continue;
        }
        PlayerModule player_module = {
            .name = name,
            .description = description,
            .pot_ball = pot_ball,
            .handle = handle,
            .library_path = paths[i]};
        player_modules[count++] = player_module;
    }

    *num_player_modules = count;
    return player_modules;
}
