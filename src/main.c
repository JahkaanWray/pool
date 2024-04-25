#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <complex.h>
#include "vector3.h"

double mu_slide = 3.5;
double mu_roll = 0.83;
double g = 9.8;
double R = 1.0;

typedef struct
{
    Vector3 p1;
    Vector3 p2;
} LineSegment;

typedef struct
{
    Vector3 initial_position;
    Vector3 initial_velocity;
    Vector3 initial_angular_velocity;
    bool rolling;
    double start_time;
    double end_time;
} PathSegment;

Vector3 get_position(PathSegment segment, double time)
{
    if (time < segment.start_time)
    {
        return segment.initial_position;
    }
    Vector3 acceleration;
    if (!segment.rolling)
    {

        Vector3 contact_point_v = Vector3_subtract(segment.initial_velocity, Vector3_cross(segment.initial_angular_velocity, (Vector3){0, 0, R}));
        acceleration = Vector3_scalar_multiply(Vector3_normalize(contact_point_v), -mu_slide * g);
    }
    else
    {
        acceleration = Vector3_scalar_multiply(Vector3_normalize(segment.initial_velocity), -mu_roll * g);
    }
    if (time > segment.end_time)
    {
        return Vector3_add(segment.initial_position, Vector3_add(Vector3_scalar_multiply(segment.initial_velocity, segment.end_time - segment.start_time), Vector3_scalar_multiply(acceleration, 0.5 * (segment.end_time - segment.start_time) * (segment.end_time - segment.start_time))));
    }
    Vector3 p = Vector3_add(segment.initial_position, Vector3_add(Vector3_scalar_multiply(segment.initial_velocity, time - segment.start_time), Vector3_scalar_multiply(acceleration, 0.5 * (time - segment.start_time) * (time - segment.start_time))));
    return p;
}

Vector3 get_velocity(PathSegment segment, double time)
{
    if (time < segment.start_time)
    {
        return segment.initial_velocity;
    }
    Vector3 acceleration;
    if (!segment.rolling)
    {
        Vector3 contact_point_v = Vector3_subtract(segment.initial_velocity, Vector3_cross(segment.initial_angular_velocity, (Vector3){0, 0, R}));
        acceleration = Vector3_scalar_multiply(Vector3_normalize(contact_point_v), -mu_slide * g);
    }
    else
    {
        acceleration = Vector3_scalar_multiply(Vector3_normalize(segment.initial_velocity), -mu_roll * g);
    }
    if (time > segment.end_time)
    {
        return Vector3_add(segment.initial_velocity, Vector3_scalar_multiply(acceleration, segment.end_time - segment.start_time));
    }
    Vector3 v = Vector3_add(segment.initial_velocity, Vector3_scalar_multiply(acceleration, time - segment.start_time));
    return v;
}

Vector3 get_angular_velocity(PathSegment segment, double time)
{
    if (time < segment.start_time)
    {
        return segment.initial_angular_velocity;
    }
    if (!segment.rolling)
    {
        Vector3 contact_point_v = Vector3_subtract(segment.initial_velocity, Vector3_cross(segment.initial_angular_velocity, (Vector3){0, 0, R}));
        Vector3 contact_point_v_normalized = Vector3_normalize(contact_point_v);
        Vector3 angular_acceleration = Vector3_scalar_multiply(Vector3_cross(contact_point_v_normalized, (Vector3){0, 0, -1}), (2.5 * mu_slide * g) / R);
        return Vector3_add(segment.initial_angular_velocity, Vector3_scalar_multiply(angular_acceleration, time - segment.start_time));
    }
    else
    {
        return Vector3_scalar_multiply(Vector3_cross(get_velocity(segment, time), (Vector3){0, 0, -1}), 1 / R);
    }
}
typedef struct
{
    PathSegment *segments;
    int num_segments;
    int capacity;
} Path;

void print_path(Path path)
{
    for (int i = 0; i < path.num_segments; i++)
    {
        PathSegment segment = path.segments[i];
        printf("Segment %d\n", i);
        printf("Initial position = (%f, %f, %f)\n", segment.initial_position.x, segment.initial_position.y, segment.initial_position.z);
        printf("Initial velocity = (%f, %f, %f)\n", segment.initial_velocity.x, segment.initial_velocity.y, segment.initial_velocity.z);
        printf("Initial angular velocity = (%f, %f, %f)\n", segment.initial_angular_velocity.x, segment.initial_angular_velocity.y, segment.initial_angular_velocity.z);
        printf("Start time = %f\n", segment.start_time);
        printf("End time = %f\n", segment.end_time);
        printf("Rolling = %d\n", segment.rolling);
        if (segment.rolling)
        {
            Vector3 acceleration = Vector3_scalar_multiply(Vector3_normalize(segment.initial_velocity), -mu_roll * g);
            printf("Acceleration = (%f, %f, %f)\n", acceleration.x, acceleration.y, acceleration.z);
        }
        else
        {
            Vector3 contact_point_v = Vector3_subtract(segment.initial_velocity, Vector3_cross(segment.initial_angular_velocity, (Vector3){0, 0, R}));
            Vector3 acceleration = Vector3_scalar_multiply(Vector3_normalize(contact_point_v), -mu_slide * g);
            printf("Acceleration = (%f, %f, %f)\n", acceleration.x, acceleration.y, acceleration.z);
        }
        printf("\n\n\n\n");
    }
}

Path new_path()
{
    Path path;
    path.segments = malloc(10 * sizeof(PathSegment));
    path.num_segments = 0;
    path.capacity = 10;
    return path;
}

void add_segment(Path *path, PathSegment segment)
{
    if (path->num_segments == path->capacity)
    {
        path->capacity *= 2;
        path->segments = realloc(path->segments, path->capacity * sizeof(PathSegment));
    }
    path->segments[path->num_segments] = segment;
    path->num_segments++;
}

void free_path(Path *path)
{
    free(path->segments);
}

bool detect_collision_with_line_segment(PathSegment *segment, LineSegment line_segment, double *t)
{
    double collision_time;
    Vector3 p1 = segment->initial_position;
    Vector3 v1 = segment->initial_velocity;
    Vector3 w1 = segment->initial_angular_velocity;
    Vector3 line_normal = Vector3_normalize(Vector3_cross(Vector3_subtract(line_segment.p2, line_segment.p1), (Vector3){0, 0, 1}));
    double sn = Vector3_dot(Vector3_subtract(line_segment.p1, p1), line_normal);
    double vn = Vector3_dot(v1, line_normal);
    Vector3 acceleration;
    if (segment->rolling)
    {
        acceleration = Vector3_scalar_multiply(Vector3_normalize(v1), -mu_roll * g);
    }
    else
    {
        Vector3 contact_point_v = Vector3_subtract(v1, Vector3_cross(w1, (Vector3){0, 0, R}));
        acceleration = Vector3_scalar_multiply(Vector3_normalize(contact_point_v), -mu_slide * g);
    }
    double an = Vector3_dot(acceleration, line_normal);
    if (an == 0)
    {
        collision_time = segment->start_time + (sn / vn);
        if (collision_time <= segment->start_time || collision_time > segment->end_time)
        {
            return false;
        }
        *t = collision_time;
        return true;
    }
    double discriminant = vn * vn + 2 * an * sn;
    if (discriminant < 0)
    {
        return false;
    }
    double collision_time1 = segment->start_time + (-vn + sqrt(discriminant)) / an;
    double collision_time2 = segment->start_time + (-vn - sqrt(discriminant)) / an;
    if (collision_time1 <= segment->start_time || collision_time1 > segment->end_time)
    {
        collision_time1 = -1;
    }
    if (collision_time2 <= segment->start_time || collision_time2 > segment->end_time)
    {
        collision_time2 = -1;
    }
    if (collision_time1 == -1 && collision_time2 == -1)
    {
        return false;
    }
    if (collision_time1 == -1)
    {
        collision_time = collision_time2;
    }
    else if (collision_time2 == -1)
    {
        collision_time = collision_time1;
    }
    else
    {
        collision_time = fmin(collision_time1, collision_time2);
    }
    *t = collision_time;
    return true;
}
bool detect_collision(PathSegment *segment, LineSegment line_segments[], double *t, LineSegment *cushion)
{
    double first_collision_time = INFINITY;
    for (int i = 0; i < 4; i++)
    {
        double collision_time;

        if (detect_collision_with_line_segment(segment, line_segments[i], &collision_time))
        {
            printf("Possible collision detected with cushion %d at t = %f\n", i, collision_time);
            if (collision_time < first_collision_time || i == 0)
            {
                first_collision_time = collision_time;
                *cushion = line_segments[i];
            }
        }
    }

    if (first_collision_time < segment->end_time && first_collision_time > segment->start_time)
    {
        *t = first_collision_time;
        printf("Collision detected with cushion\n");
        return true;
    }
    return false;
}

bool update_path(Path *path, LineSegment line_segments[])
{
    printf("Updating path\n");
    if (path->num_segments == 0)
    {
        return false;
    }
    PathSegment *last_segment = &(path->segments[path->num_segments - 1]);
    double collision_time;
    LineSegment cushion;
    printf("Checking for collision\n");
    if (detect_collision(last_segment, line_segments, &collision_time, &cushion))
    {

        Vector3 line_normal = Vector3_normalize(Vector3_cross(Vector3_subtract(cushion.p2, cushion.p1), (Vector3){0, 0, 1}));
        Vector3 collision_position = get_position(*last_segment, collision_time);
        Vector3 collision_velocity = get_velocity(*last_segment, collision_time);
        Vector3 collision_angular_velocity = get_angular_velocity(*last_segment, collision_time);
        Vector3 new_velocity = Vector3_subtract(collision_velocity, Vector3_scalar_multiply(line_normal, 2 * Vector3_dot(collision_velocity, line_normal)));
        last_segment->end_time = collision_time;
        bool rolling = false;
        double start_time = collision_time;
        Vector3 contact_point_v = Vector3_subtract(new_velocity, Vector3_cross(collision_angular_velocity, (Vector3){0, 0, R}));
        double end_time = start_time + 2 * Vector3_mag(contact_point_v) / (7 * mu_slide * g);
        PathSegment next_segment = {collision_position, new_velocity, collision_angular_velocity, rolling, start_time, end_time};
        add_segment(path, next_segment);
        printf("Collision detected\n");
        printf("Collition time = %f\n", collision_time);
        printf("Collision position = (%f, %f, %f)\n", collision_position.x, collision_position.y, collision_position.z);
        printf("Collision velocity = (%f, %f, %f)\n", collision_velocity.x, collision_velocity.y, collision_velocity.z);
        printf("Collision angular velocity = (%f, %f, %f)\n", collision_angular_velocity.x, collision_angular_velocity.y, collision_angular_velocity.z);
        printf("New velocity = (%f, %f, %f)\n", new_velocity.x, new_velocity.y, new_velocity.z);

        print_path(*path);

        return true;
    }
    printf("No collision detected\n");
    if (last_segment->rolling)
    {
        printf("Last segment is rolling\nPath finished\n");
        return false;
    }
    printf("Last segment is sliding\nAdding rolling segment\n");
    Vector3 last_position = get_position(*last_segment, last_segment->end_time);
    Vector3 last_velocity = get_velocity(*last_segment, last_segment->end_time);
    Vector3 last_angular_velocity = get_angular_velocity(*last_segment, last_segment->end_time);
    bool rolling = true;
    double start_time = last_segment->end_time;
    double end_time = start_time + Vector3_mag(last_velocity) / (mu_roll * g);
    PathSegment next_segment = {last_position, last_velocity, last_angular_velocity, rolling, start_time, end_time};
    add_segment(path, next_segment);
    print_path(*path);
    return true;
}

void generate_path(Path *path, Vector3 initial_position, Vector3 initial_velocity, Vector3 initial_angular_velocity, bool rolling, double start_time, LineSegment line_segments[])
{
    double end_time;
    if (rolling)
    {
        end_time = start_time + Vector3_mag(initial_velocity) / (mu_roll * g);
    }
    else
    {
        Vector3 contact_point_v = Vector3_subtract(initial_velocity, Vector3_cross(initial_angular_velocity, (Vector3){0, 0, R}));

        end_time = start_time + 2 * Vector3_mag(contact_point_v) / (7 * mu_slide * g);
    }
    PathSegment segment = {initial_position, initial_velocity, initial_angular_velocity, rolling, start_time, end_time};
    add_segment(path, segment);
    while (update_path(path, line_segments))
        ;
}

void render_path_segment(SDL_Renderer *renderer, PathSegment segment)
{
    if (segment.rolling)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        Vector3 p1 = segment.initial_position;
        Vector3 p2 = get_position(segment, segment.end_time);
        SDL_RenderDrawLine(renderer, p1.x, p1.y, p2.x, p2.y);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (int i = 0; i < 100; i++)
        {
            double t1 = segment.start_time + i * (segment.end_time - segment.start_time) / 100;
            double t2 = segment.start_time + (i + 1) * (segment.end_time - segment.start_time) / 100;
            Vector3 p1 = get_position(segment, t1);
            Vector3 p2 = get_position(segment, t2);
            SDL_RenderDrawLine(renderer, p1.x, p1.y, p2.x, p2.y);
        }
    }
}

void render_path(SDL_Renderer *renderer, Path path)
{
    for (int i = 0; i < path.num_segments; i++)
    {
        PathSegment segment = path.segments[i];
        render_path_segment(renderer, segment);
    }
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("SDL2 Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1640, 900, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Vector3 v = {1, 0, 0};
    Vector3 w = {0, 1, 0};
    int v_mag = 1;
    int w_mag = 1;

    LineSegment cushions[4] = {{{100, 100, 0}, {800, 100, 0}}, {{800, 100, 0}, {800, 800, 0}}, {{800, 800, 0}, {100, 800, 0}}, {{100, 800, 0}, {100, 100, 0}}};

    bool quit = 0;
    SDL_Event event;
    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = 1;
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                int mx, my;
                Uint32 mouseState = SDL_GetMouseState(&mx, &my);
                if (mx > 1450 && mx < 1530 && my > 10 && my < 890)
                {
                    v_mag = 890 - my;
                }
                if (mx > 1550 && mx < 1630 && my > 10 && my < 890)
                {
                    w_mag = 890 - my;
                }
                if (mx > 0 && mx < 100 && my > 700 && my < 800)
                {
                    v = Vector3_normalize((Vector3){mx - 50, my - 750, 0});
                }
                if (mx > 0 && mx < 100 && my > 800 && my < 900)
                {
                    w = Vector3_normalize((Vector3){mx - 50, my - 850, 0});
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 180, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect){0, 700, 100, 100});
        SDL_RenderFillRect(renderer, &(SDL_Rect){0, 800, 100, 100});
        SDL_RenderFillRect(renderer, &(SDL_Rect){1540, 0, 100, 900});
        SDL_RenderFillRect(renderer, &(SDL_Rect){1440, 0, 100, 900});
        SDL_SetRenderDrawColor(renderer, 180, 0, 180, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect){1550, 890 - w_mag, 80, w_mag});
        SDL_RenderFillRect(renderer, &(SDL_Rect){1450, 890 - v_mag, 80, v_mag});
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, 50, 750, 50 + 50 * v.x, 750 + 50 * v.y);
        SDL_RenderDrawLine(renderer, 50, 850, 50 + 50 * w.x, 850 + 50 * w.y);

        Path path = new_path();
        generate_path(&path, (Vector3){500, 300, 0}, Vector3_scalar_multiply(v, v_mag), Vector3_scalar_multiply(w, w_mag), false, 0, cushions);
        render_path(renderer, path);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for (int i = 0; i < 4; i++)
        {
            SDL_RenderDrawLine(renderer, cushions[i].p1.x, cushions[i].p1.y, cushions[i].p2.x, cushions[i].p2.y);
        }
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}