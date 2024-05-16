#include "player.h"
#include "game.h"
#include <stdio.h>
#include <dlfcn.h>
#include <time.h>
#include "serialise.h"

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        printf("Usage: %s <player1.so> <player2.so> \n", argv[0]);
        return 1;
    }

    Player players[2];
    players[0].type = AI;
    players[1].type = AI;

    char *player1_path = argv[1];
    char *player2_path = argv[2];

    players[0].module.library_path = player1_path;
    players[1].module.library_path = player2_path;

    printf("Loading player 1: %s\n", player1_path);
    void *library1 = dlopen(player1_path, RTLD_LAZY);
    if (!library1)
    {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return 1;
    }
    players[0].module.handle = library1;
    char *name = *(char **)dlsym(library1, "name");
    if (name == NULL)
    {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
        return 1;
    }
    players[0].module.name = name;
    char *description = *(char **)dlsym(library1, "description");
    if (description == NULL)
    {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
        return 1;
    }
    printf("Player 1 description: %s\n", description);
    players[0].module.description = description;
    void *pot_ball = dlsym(library1, "pot_ball");
    if (!pot_ball)
    {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
        return 1;
    }
    players[0].module.pot_ball = pot_ball;
    printf("Player 1 name: %s\n", name);
    printf("Loading player 2: %s\n", player2_path);
    void *library2 = dlopen(player2_path, RTLD_LAZY);
    if (!library2)
    {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return 1;
    }
    players[1].module.handle = library2;
    name = *(char **)dlsym(library2, "name");
    if (name == NULL)
    {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
        return 1;
    }
    players[1].module.name = name;
    printf("Player 2 name: %s\n", name);
    description = *(char **)dlsym(library2, "description");
    if (description == NULL)
    {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
        return 1;
    }
    players[1].module.description = description;
    printf("Player 2 description: %s\n", description);
    pot_ball = dlsym(library2, "pot_ball");
    if (!pot_ball)
    {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
        return 1;
    }
    players[1].module.pot_ball = pot_ball;

    SetRandomSeed(time(NULL));

    Game *game = create_game(players, 2);
    game->playback_speed = 1000;
    game->default_playback_speed = 1000;

    while (game->num_frames < 10)
    {
        printf("Frame %d\n", game->num_frames + 1);
        update_game(game);
    }

    Frame *frames = game->frames;
    int p1 = 0;
    int p2 = 0;
    int shots = 0;
    int longest = 0;
    int longest_frame = 0;
    int shortest = 1000;
    int shortest_frame = 0;
    int freqs[20] = {0};
    for (int i = 0; i < game->num_frames; i++)
    {
        printf("Frame winner: %p\n", (void *)frames[i].winner);
        if (frames[i].winner == &(game->players[0]))
        {
            p1++;
        }
        else
        {
            p2++;
        }

        shots += frames[i].num_shots;
        if (frames[i].num_shots > longest)
        {
            longest = frames[i].num_shots;
            longest_frame = i;
        }
        if (frames[i].num_shots < shortest && frames[i].num_shots > 0)
        {
            shortest = frames[i].num_shots;
            shortest_frame = i;
        }
        if (frames[i].num_shots < 20)
        {
            freqs[frames[i].num_shots]++;
        }
        else
        {
            freqs[19]++;
        }
    }

    printf("%s: %d\n", players[0].module.name, p1);
    printf("%s: %d\n", players[1].module.name, p2);
    printf("Average shots per frame: %f\n", (float)shots / game->num_frames);
    printf("Longest frame: frame %d, %d shots\n", longest_frame, longest);
    printf("Shortest frame: frame %d, %d shots\n", shortest_frame, shortest);
    printf("Frequency of frames with n shots:\n");
    for (int i = 0; i < 20; i++)
    {
        printf("%d: %d\n", i, freqs[i]);
    }

    for (int i = 0; i < game->num_frames; i++)
    {
        printf("Frame %d\n", i);
        for (int j = 0; j < frames[i].num_shots; j++)
        {
            printf("Shot %d\n", j);
            printf("Players array %p\n", (void *)game->players);
            printf("%p %p\n", (void *)&(game->players[0]), (void *)&(game->players[1]));
            printf("Player: %p\n", (void *)(frames[i].shot_history[j].player));
            for (int k = 0; k < game->scene.ball_set.num_balls; k++)
            {
                printf("Ball %d\n", k);
                for (int l = 0; l < frames[i].shot_history[j].ball_paths[k].num_segments; l++)
                {
                    printf("Segment %d\n", l);
                    printf("Initial position: (%f, %f, %f)\n", frames[i].shot_history[j].ball_paths[k].segments[l].initial_position.x, frames[i].shot_history[j].ball_paths[k].segments[l].initial_position.y, frames[i].shot_history[j].ball_paths[k].segments[l].initial_position.z);
                    printf("Initial velocity: (%f, %f, %f)\n", frames[i].shot_history[j].ball_paths[k].segments[l].initial_velocity.x, frames[i].shot_history[j].ball_paths[k].segments[l].initial_velocity.y, frames[i].shot_history[j].ball_paths[k].segments[l].initial_velocity.z);
                    printf("Acceleration: (%f, %f, %f)\n", frames[i].shot_history[j].ball_paths[k].segments[l].acceleration.x, frames[i].shot_history[j].ball_paths[k].segments[l].acceleration.y, frames[i].shot_history[j].ball_paths[k].segments[l].acceleration.z);
                    printf("Start time: %f\n", frames[i].shot_history[j].ball_paths[k].segments[l].start_time);
                    printf("End time: %f\n", frames[i].shot_history[j].ball_paths[k].segments[l].end_time);
                }
            }
        }
    }

    serialise_game(game);
    deserialise_game("frames.bin");

    return 0;
}