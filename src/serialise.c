#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"

void serialise_game(Game *game)
{
    FILE *file = fopen("frames.bin", "wb");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }
    int num_frames = game->num_frames;
    Frame *frames = game->frames;
    for (int i = 0; i < game->num_players; i++)
    {
        char *name = game->players[i].module.name;
        int name_length = strlen(name);
        char *description = game->players[i].module.description;
        int description_length = strlen(description);
        fwrite(&name_length, sizeof(int), 1, file);
        fwrite(name, sizeof(char), strlen(name), file);
        fwrite(&description_length, sizeof(int), 1, file);
        fwrite(description, sizeof(char), strlen(description), file);
    }
    fwrite(&num_frames, sizeof(int), 1, file);
    for (int i = 0; i < num_frames; i++)
    {
        fwrite(&(frames[i].num_shots), sizeof(int), 1, file);
        for (int j = 0; j < frames[i].num_shots; j++)
        {
            fwrite(&(frames[i].shot_history[j].num_events), sizeof(int), 1, file);
            for (int k = 0; k < frames[i].shot_history[j].num_events; k++)
            {
                fwrite(&(frames[i].shot_history[j].events[k].type), sizeof(ShotEventType), 1, file);
            }
            for (int k = 0; k < game->scene.ball_set.num_balls; k++)
            {
                fwrite(&(frames[i].shot_history[j].ball_paths[k].num_segments), sizeof(int), 1, file);
                for (int l = 0; l < frames[i].shot_history[j].ball_paths[k].num_segments; l++)
                {
                    fwrite(&(frames[i].shot_history[j].ball_paths[k].segments[l].initial_position), sizeof(Vector3), 1, file);
                    fwrite(&(frames[i].shot_history[j].ball_paths[k].segments[l].initial_velocity), sizeof(Vector3), 1, file);
                    fwrite(&(frames[i].shot_history[j].ball_paths[k].segments[l].acceleration), sizeof(Vector3), 1, file);
                    fwrite(&(frames[i].shot_history[j].ball_paths[k].segments[l].start_time), sizeof(double), 1, file);
                    fwrite(&(frames[i].shot_history[j].ball_paths[k].segments[l].end_time), sizeof(double), 1, file);
                }
            }
        }
    }
    fclose(file);
}

void deserialise_game(char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }
    for (int i = 0; i < 2; i++)
    {
        int name_length;
        fread(&name_length, sizeof(int), 1, file);
        char name[name_length + 1];
        fread(name, sizeof(char), name_length, file);
        name[name_length] = '\0';
        printf("Name: %s\n", name);
        int description_length;
        fread(&description_length, sizeof(int), 1, file);
        char description[description_length + 1];
        fread(description, sizeof(char), description_length, file);
        description[description_length] = '\0';
        printf("Description: %s\n", description);
    }

    int num_frames;
    fread(&num_frames, sizeof(int), 1, file);
    printf("Num frames: %d\n", num_frames);
    for (int i = 0; i < num_frames; i++)
    {
        printf("Frame %d\n", i + 1);
        int num_shots;
        fread(&num_shots, sizeof(int), 1, file);
        printf("Num shots: %d\n", num_shots);

        for (int j = 0; j < num_shots; j++)
        {
            printf("Shot %d\n", j + 1);
            int num_events;
            fread(&num_events, sizeof(int), 1, file);
            printf("Num events: %d\n", num_events);
            for (int k = 0; k < num_events; k++)
            {
                ShotEventType type;
                fread(&type, sizeof(ShotEventType), 1, file);
                printf("Event type: %s\n", type == BALL_BALL_COLLISION ? "Ball Ball Collision" : type == BALL_CUSHION_COLLISION ? "Ball Cushion Collision"
                                                                                             : type == BALL_POCKETED            ? "Ball Potted"
                                                                                             : type == BALL_ROLL                ? "Ball Roll"
                                                                                             : type == BALL_STOP                ? "Ball Stop"
                                                                                                                                : "None");
            }
            for (int k = 0; k < 10; k++)
            {
                printf("Ball %d\n", k + 1);
                int num_segments;
                fread(&num_segments, sizeof(int), 1, file);
                printf("Num segments: %d\n", num_segments);
                for (int l = 0; l < num_segments; l++)
                {
                    Vector3 initial_position;
                    fread(&initial_position, sizeof(Vector3), 1, file);
                    Vector3 initial_velocity;
                    fread(&initial_velocity, sizeof(Vector3), 1, file);
                    Vector3 acceleration;
                    fread(&acceleration, sizeof(Vector3), 1, file);
                    double start_time;
                    fread(&start_time, sizeof(double), 1, file);
                    double end_time;
                    fread(&end_time, sizeof(double), 1, file);
                    printf("Segment %d\n", l + 1);
                    printf("Initial position: (%f, %f, %f)\n", initial_position.x, initial_position.y, initial_position.z);
                    printf("Initial velocity: (%f, %f, %f)\n", initial_velocity.x, initial_velocity.y, initial_velocity.z);
                    printf("Acceleration: (%f, %f, %f)\n", acceleration.x, acceleration.y, acceleration.z);
                    printf("Start time: %f\n", start_time);
                    printf("End time: %f\n", end_time);
                }
            }
        }
    }
    fclose(file);
}
