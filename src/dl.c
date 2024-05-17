#include "dl.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>

void load_library_paths(char **paths, int *num_paths)
{
    DIR *dir;
    struct dirent *ent;
    const char *current_dir = ".";
    const char *parent_dir = "..";
    int count = 0;
    if ((dir = opendir("./player_modules")) != NULL)
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

    paths = malloc(count * sizeof(char *));

    if ((dir = opendir("./player_modules")) != NULL)
    {
        int i = 0;
        while ((ent = readdir(dir)) != NULL)
        {
            char *name = ent->d_name;
            if (strcmp(name, current_dir) == 0 || strcmp(name, parent_dir) == 0)
            {
                continue;
            }
            paths[i++] = name;
        }
    }
}

void load_player_modules(char **paths, int num_paths, PlayerModule *player_modules, int *num_player_modules)
{
    for (int i = 0; i < num_paths; i++)
    {
        char *full_path = malloc(256);
        sprintf(full_path, "./player_modules/%s", paths[i]);
        void *library = dlopen(full_path, RTLD_LAZY);
        if (library == NULL)
        {
            fprintf(stderr, "dlopen failed: %s\n", dlerror());
            continue;
        }
        void *pot_ball = dlsym(library, "pot_ball");
        if (pot_ball == NULL)
        {
            fprintf(stderr, "dlsym failed: %s\n", dlerror());
            continue;
        }
        char *name = *(char **)dlsym(library, "name");
        if (name == NULL)
        {
            fprintf(stderr, "dlsym failed: %s\n", dlerror());
            continue;
        }
        char *description = *(char **)dlsym(library, "description");
        if (description == NULL)
        {
            fprintf(stderr, "dlsym failed: %s\n", dlerror());
            continue;
        }

        PlayerModule player_module = {
            .name = name,
            .description = description,
            .pot_ball = pot_ball,
            .handle = library,
            .library_path = full_path};

        player_modules[*num_player_modules] = player_module;
        (*num_player_modules)++;
    }
}