#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <complex.h>
#include "vector3.h"
#include "polynomial.h"

typedef struct
{
    Vector3 p1;
    Vector3 p2;
} Cushion;

typedef struct
{
    Vector3 position;
    double radius;
} Pocket;

typedef struct
{
    Vector3 initial_position;
    Vector3 initial_velocity;
    Vector3 acceleration;
    Vector3 initial_angular_velocity;
    Vector3 angular_acceleration;
    bool rolling;
    double start_time;
    double end_time;
} PathSegment;

typedef struct
{
    Cushion *cushions;
    int num_cushions;
    int cushion_capacity;
    Pocket *pockets;
    int num_pockets;
    int pocket_capacity;
} Table;

typedef struct
{
    PathSegment *segments;
    int num_segments;
    int capacity;
} Path;

typedef struct
{
    int id;
    Vector3 initial_position;
    long colour;
    double radius;
    double mass;
    Path path;
} Ball;

typedef struct
{
    Ball *balls;
    int num_balls;
    int ball_capacity;
} BallSet;

typedef struct
{
    double mu_slide;
    double mu_roll;
    double g;
    double e_ball_ball;
    double e_ball_cushion;
    double e_ball_table;
    double mu_ball_cushion;
    double mu_ball_ball;
} Coefficients;
typedef struct
{
    Table table;
    BallSet ball_set;
    Coefficients coefficients;
} Scene;

typedef enum
{
    HUMAN,
    AI
} PlayerType;

typedef struct
{
    PlayerType type;
    Game *game;
} Player;

typedef enum
{
    NONE,
    BALL_BALL_COLLISION,
    BALL_CUSHION_COLLISION,
    BALL_POCKETED,
    BALL_ROLL,
    BALL_STOP
} ShotEventType;
typedef struct
{
    ShotEventType type;
    Ball *ball1;
    Ball *ball2;
    Cushion *cushion;
    Pocket *pocket;
} ShotEvent;

typedef struct
{
    Path *ball_paths;
    ShotEvent *events;
} Shot;

typedef struct
{
    Scene scene;
    Player *players;
    int num_players;

    Shot *shot_history;
} Game;

Vector3 get_position(PathSegment segment, double time)
{
    if (time < segment.start_time)
    {
        return segment.initial_position;
    }
    if (time > segment.end_time)
    {
        return Vector3_add(segment.initial_position, Vector3_add(Vector3_scalar_multiply(segment.initial_velocity, segment.end_time - segment.start_time), Vector3_scalar_multiply(segment.acceleration, 0.5 * (segment.end_time - segment.start_time) * (segment.end_time - segment.start_time))));
    }
    Vector3 p = Vector3_add(segment.initial_position, Vector3_add(Vector3_scalar_multiply(segment.initial_velocity, time - segment.start_time), Vector3_scalar_multiply(segment.acceleration, 0.5 * (time - segment.start_time) * (time - segment.start_time))));
    return p;
}

Vector3 get_velocity(PathSegment segment, double time)
{
    if (time < segment.start_time)
    {
        return segment.initial_velocity;
    }
    if (time > segment.end_time)
    {
        return Vector3_add(segment.initial_velocity, Vector3_scalar_multiply(segment.acceleration, segment.end_time - segment.start_time));
    }
    Vector3 v = Vector3_add(segment.initial_velocity, Vector3_scalar_multiply(segment.acceleration, time - segment.start_time));
    return v;
}

Vector3 get_angular_velocity(PathSegment segment, double time)
{
    if (time < segment.start_time)
    {
        return segment.initial_angular_velocity;
    }
    if (time > segment.end_time)
    {
        return Vector3_add(segment.initial_angular_velocity, Vector3_scalar_multiply(segment.angular_acceleration, segment.end_time - segment.start_time));
    }
    return Vector3_add(segment.initial_angular_velocity, Vector3_scalar_multiply(segment.angular_acceleration, time - segment.start_time));
}
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
        Vector3 acceleration = segment.acceleration;
        printf("Acceleration = (%f, %f, %f)\n", acceleration.x, acceleration.y, acceleration.z);
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
        if (path->capacity == 0)
        {
            path->capacity = 1;
        }
        else
        {
            path->capacity *= 2;
        }
        path->segments = realloc(path->segments, path->capacity * sizeof(PathSegment));
    }
    path->segments[path->num_segments] = segment;
    path->num_segments++;
}

void add_sliding_segment(Ball *ball, Vector3 initial_position, Vector3 initial_velocity, Vector3 initial_angular_velocity, double start_time, Coefficients coefficients)
{
    double mu_slide = coefficients.mu_slide;
    double mu_roll = coefficients.mu_roll;
    double g = coefficients.g;
    double R = ball->radius;
    Vector3 contact_point_v = Vector3_subtract(initial_velocity, Vector3_cross(initial_angular_velocity, (Vector3){0, 0, R}));
    Vector3 acceleration = Vector3_scalar_multiply(Vector3_normalize(contact_point_v), -mu_slide * g);
    Vector3 angular_acceleration = Vector3_scalar_multiply(Vector3_cross(acceleration, (Vector3){0, 0, -1}), -2.5 / R);
    double end_time = start_time + 2 * Vector3_mag(contact_point_v) / (7 * mu_slide * g);
    PathSegment segment = {initial_position, initial_velocity, acceleration, initial_angular_velocity, angular_acceleration, false, start_time, end_time};
    ball->path.segments[ball->path.num_segments - 1].end_time = start_time;
    add_segment(&(ball->path), segment);
}

void add_rolling_segment(Ball *ball, Vector3 initial_position, Vector3 initial_velocity, double start_time, Coefficients coefficients)
{
    double mu_roll = coefficients.mu_roll;
    double g = coefficients.g;
    double R = ball->radius;
    Vector3 acceleration = Vector3_scalar_multiply(Vector3_normalize(initial_velocity), -mu_roll * g);
    Vector3 initial_angular_velocity = Vector3_cross(initial_velocity, (Vector3){0, 0, -1 / R});
    Vector3 angular_acceleration = Vector3_scalar_multiply(Vector3_cross(acceleration, (Vector3){0, 0, -1}), 1 / R);
    double end_time = start_time + Vector3_mag(initial_velocity) / (mu_roll * g);
    PathSegment segment = {initial_position, initial_velocity, acceleration, initial_angular_velocity, angular_acceleration, true, start_time, end_time};
    add_segment(&(ball->path), segment);
}

void free_path(Path *path)
{
    free(path->segments);
    path->num_segments = 0;
    path->capacity = 0;
}

bool detect_ball_cushion_collision(Ball ball, Cushion line_segment, double *t)
{
    double collision_time;
    PathSegment *segment = &(ball.path.segments[ball.path.num_segments - 1]);
    Vector3 p1 = segment->initial_position;
    Vector3 v1 = segment->initial_velocity;
    Vector3 w1 = segment->initial_angular_velocity;
    Vector3 line_normal = Vector3_normalize(Vector3_cross(Vector3_subtract(line_segment.p2, line_segment.p1), (Vector3){0, 0, 1}));
    double sn = Vector3_dot(Vector3_subtract(line_segment.p1, p1), line_normal);
    double vn = Vector3_dot(v1, line_normal);
    Vector3 a = segment->acceleration;
    double an = Vector3_dot(a, line_normal);
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
    double tolerance = 1e-3;
    if (collision_time1 <= segment->start_time + tolerance || collision_time1 > segment->end_time)
    {
        collision_time1 = -1;
    }
    if (collision_time2 <= segment->start_time + tolerance || collision_time2 > segment->end_time)
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

bool detect_ball_ball_collision(Ball ball1, Ball ball2, double *t)
{
    PathSegment *segment1 = &(ball1.path.segments[ball1.path.num_segments - 1]);
    PathSegment *segment2 = &(ball2.path.segments[ball2.path.num_segments - 1]);
    Vector3 p1 = segment1->initial_position;
    Vector3 p2 = segment2->initial_position;
    Vector3 v1 = segment1->initial_velocity;
    Vector3 v2 = segment2->initial_velocity;
    Vector3 a1 = segment1->acceleration;
    Vector3 a2 = segment2->acceleration;
    double t1 = segment1->start_time;
    double t2 = segment2->start_time;
    double r1 = ball1.radius;
    double r2 = ball2.radius;

    double A1 = 0.5 * a1.x;
    double B1 = v1.x - a1.x * t1;
    double C1 = p1.x - v1.x * t1 + 0.5 * a1.x * t1 * t1;

    double A2 = 0.5 * a2.x;
    double B2 = v2.x - a2.x * t2;
    double C2 = p2.x - v2.x * t2 + 0.5 * a2.x * t2 * t2;

    double A3 = 0.5 * a1.y;
    double B3 = v1.y - a1.y * t1;
    double C3 = p1.y - v1.y * t1 + 0.5 * a1.y * t1 * t1;

    double A4 = 0.5 * a2.y;
    double B4 = v2.y - a2.y * t2;
    double C4 = p2.y - v2.y * t2 + 0.5 * a2.y * t2 * t2;

    double a = (A1 - A2) * (A1 - A2) + (A3 - A4) * (A3 - A4);
    double b = 2 * ((A1 - A2) * (B1 - B2) + (A3 - A4) * (B3 - B4));
    double c = 2 * ((A1 - A2) * (C1 - C2) + (A3 - A4) * (C3 - C4)) + (B1 - B2) * (B1 - B2) + (B3 - B4) * (B3 - B4);
    double d = 2 * ((B1 - B2) * (C1 - C2) + (B3 - B4) * (C3 - C4));
    double e = (C1 - C2) * (C1 - C2) + (C3 - C4) * (C3 - C4) - (r1 + r2) * (r1 + r2);

    double x1, x2, x3, x4;
    solve_quartic(a, b, c, d, e, &x1, &x2, &x3, &x4);

    double collision_time = INFINITY;
    double tolerance = 1e-3;
    if (x1 > segment1->start_time + tolerance && x1 < segment1->end_time && x1 > segment2->start_time + tolerance && x1 < segment2->end_time && x1 < collision_time)
    {
        collision_time = x1;
    }
    if (x2 > segment1->start_time + tolerance && x2 < segment1->end_time && x2 > segment2->start_time + tolerance && x2 < segment2->end_time && x2 < collision_time)
    {
        collision_time = x2;
    }
    if (x3 > segment1->start_time + tolerance && x3 < segment1->end_time && x3 > segment2->start_time + tolerance && x3 < segment2->end_time && x3 < collision_time)
    {
        collision_time = x3;
    }
    if (x4 > segment1->start_time + tolerance && x4 < segment1->end_time && x4 > segment2->start_time + tolerance && x4 < segment2->end_time && x4 < collision_time)
    {
        collision_time = x4;
    }
    if (collision_time == INFINITY)
    {
        return false;
    }
    *t = collision_time;
    return true;
}

typedef enum
{
    NONE,
    BALL_BALL,
    BALL_CUSHION,
    BALL_TABLE,
    BALL_ROLL,
    BALL_STOP
} UpdateType;

void resolve_ball_ball_collision(Ball *ball1, Ball *ball2, double time, Coefficients coefficients)
{
    PathSegment *segment1 = &(ball1->path.segments[ball1->path.num_segments - 1]);
    PathSegment *segment2 = &(ball2->path.segments[ball2->path.num_segments - 1]);
    Vector3 p1 = get_position(*segment1, time);
    Vector3 p2 = get_position(*segment2, time);
    Vector3 v1 = get_velocity(*segment1, time);
    Vector3 v2 = get_velocity(*segment2, time);
    Vector3 w1 = get_angular_velocity(*segment1, time);
    Vector3 w2 = get_angular_velocity(*segment2, time);
    Vector3 normal = Vector3_normalize(Vector3_subtract(p2, p1));
    Vector3 tangent = Vector3_cross(normal, (Vector3){0, 0, 1});
    double e = coefficients.e_ball_ball;
    double m1 = ball1->mass;
    double m2 = ball2->mass;
    double v1n = Vector3_dot(v1, normal);
    double v2n = Vector3_dot(v2, normal);
    double v1t = Vector3_dot(v1, tangent);
    double v2t = Vector3_dot(v2, tangent);
    double v1n_final = (v1n * (m1 - e * m2) + 2 * e * m2 * v2n) / (m1 + m2);
    double v2n_final = (v2n * (m2 - e * m1) + 2 * e * m1 * v1n) / (m1 + m2);
    double v1t_final = v1t;
    double v2t_final = v2t;

    Vector3 v1_final = Vector3_add(Vector3_scalar_multiply(normal, v1n_final), Vector3_scalar_multiply(tangent, v1t_final));
    Vector3 v2_final = Vector3_add(Vector3_scalar_multiply(normal, v2n_final), Vector3_scalar_multiply(tangent, v2t_final));
    add_sliding_segment(ball1, p1, v1_final, w1, time, coefficients);
    add_sliding_segment(ball2, p2, v2_final, w2, time, coefficients);
}

void resolve_ball_cushion_collision(Ball *ball, Cushion cushion, double time, Coefficients coefficients)
{
    PathSegment *segment = &(ball->path.segments[ball->path.num_segments - 1]);
    Vector3 p = get_position(*segment, time);
    Vector3 v = get_velocity(*segment, time);
    Vector3 w = get_angular_velocity(*segment, time);
    Vector3 normal = Vector3_normalize(Vector3_cross(Vector3_subtract(cushion.p2, cushion.p1), (Vector3){0, 0, 1}));
    Vector3 tangent = Vector3_cross(normal, (Vector3){0, 0, 1});
    double e = coefficients.e_ball_cushion;
    double v_n = Vector3_dot(v, normal);
    double v_t = Vector3_dot(v, tangent);
    double v_n_final = -e * v_n;
    double v_t_final = v_t;
    Vector3 v_final = Vector3_add(Vector3_scalar_multiply(normal, v_n_final), Vector3_scalar_multiply(tangent, v_t_final));
    add_sliding_segment(ball, p, v_final, w, time, coefficients);
}

void resolve_roll(Ball *ball, double time, Coefficients coefficients)
{
    PathSegment *segment = &(ball->path.segments[ball->path.num_segments - 1]);
    Vector3 p = get_position(*segment, segment->end_time);
    Vector3 v = get_velocity(*segment, segment->end_time);
    add_rolling_segment(ball, p, v, time, coefficients);
}

void resolve_stop(Ball *ball, double time)
{
    PathSegment *segment = &(ball->path.segments[ball->path.num_segments - 1]);
    Vector3 p = get_position(*segment, segment->end_time);
    Vector3 v = get_velocity(*segment, segment->end_time);
    PathSegment stop_segment = {p, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, false, time, INFINITY};
    add_segment(&(ball->path), stop_segment);
}

bool update_path(Scene *scene)
{
    UpdateType update_type = NONE;
    double first_time = INFINITY;
    double time = INFINITY;
    Cushion cushion;
    Ball *ball1;
    Ball *ball2;
    for (int i = 0; i < scene->ball_set.num_balls; i++)
    {
        Ball *current_ball = &(scene->ball_set.balls[i]);
        for (int j = i + 1; j < scene->ball_set.num_balls; j++)
        {
            Ball *other_ball = &(scene->ball_set.balls[j]);
            if (detect_ball_ball_collision(*current_ball, *other_ball, &time))
            {
                if (time < first_time)
                {
                    first_time = time;
                    update_type = BALL_BALL;
                    ball1 = current_ball;
                    ball2 = other_ball;
                }
            }
        }

        for (int j = 0; j < scene->table.num_cushions; j++)
        {
            Cushion current_cushion = scene->table.cushions[j];
            if (detect_ball_cushion_collision(*current_ball, current_cushion, &time))
            {
                if (time < first_time)
                {
                    first_time = time;
                    update_type = BALL_CUSHION;
                    ball1 = current_ball;
                    cushion = current_cushion;
                }
            }
        }
    }
    double mu_slide = scene->coefficients.mu_slide;
    double mu_roll = scene->coefficients.mu_roll;
    double g = scene->coefficients.g;

    for (int i = 0; i < scene->ball_set.num_balls; i++)
    {
        Ball *current_ball = &(scene->ball_set.balls[i]);
        PathSegment *last_segment = &(current_ball->path.segments[current_ball->path.num_segments - 1]);
        time = last_segment->end_time;
        if (time < first_time)
        {
            first_time = time;
            ball1 = current_ball;
            if (last_segment->rolling)
            {
                update_type = BALL_STOP;
            }
            else
            {
                update_type = BALL_ROLL;
            }
        }
    }
    if (update_type == NONE)
    {
        return false;
    }
    if (update_type == BALL_BALL)
    {
        resolve_ball_ball_collision(ball1, ball2, first_time, scene->coefficients);
        printf("Ball %d hit ball %d\n", ball1->id, ball2->id);
    }
    else if (update_type == BALL_CUSHION)
    {
        resolve_ball_cushion_collision(ball1, cushion, first_time, scene->coefficients);
        printf("Ball %d hit cushion\n", ball1->id);
    }
    else if (update_type == BALL_ROLL)
    {
        resolve_roll(ball1, first_time, scene->coefficients);
        printf("Ball %d started rolling\n", ball1->id);
    }
    else if (update_type == BALL_STOP)
    {
        resolve_stop(ball1, first_time);
        printf("Ball %d stopped moving\n", ball1->id);
    }
    return true;
}

void generate_paths(Scene *scene, Ball *ball, Vector3 initial_position, Vector3 initial_velocity, Vector3 initial_angular_velocity, double start_time)
{
    for (int i = 0; i < scene->ball_set.num_balls; i++)
    {
        Ball *current_ball = &(scene->ball_set.balls[i]);
        if (current_ball->id == ball->id)
        {
            continue;
        }
        PathSegment segment = {current_ball->initial_position, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, false, 0, INFINITY};
        add_segment(&(current_ball->path), segment);
    }
    double mu_slide = scene->coefficients.mu_slide;
    double mu_roll = scene->coefficients.mu_roll;
    double g = scene->coefficients.g;
    double R = ball->radius;
    double end_time;

    Vector3 contact_point_v = Vector3_subtract(initial_velocity, Vector3_cross(initial_angular_velocity, (Vector3){0, 0, R}));
    Vector3 acceleration = Vector3_scalar_multiply(Vector3_normalize(contact_point_v), -mu_slide * g);
    Vector3 angular_acceleration = Vector3_scalar_multiply(Vector3_cross(acceleration, (Vector3){0, 0, -1}), -2.5 / R);

    end_time = start_time + 2 * Vector3_mag(contact_point_v) / (7 * mu_slide * g);
    PathSegment segment = {initial_position, initial_velocity, acceleration, initial_angular_velocity, angular_acceleration, false, start_time, end_time};
    add_segment(&(ball->path), segment);
    while (update_path(scene))
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

void solve_direct_shot(Scene *scene, Vector3 initial_position, Vector3 target_position, Vector3 v_roll, Vector3 *v, Vector3 *w)
{
    double R = scene->ball_set.balls[0].radius;
    double mu_slide = scene->coefficients.mu_slide;
    double mu_roll = scene->coefficients.mu_roll;
    double g = scene->coefficients.g;

    Vector3 roll_position = Vector3_subtract(target_position, Vector3_scalar_multiply(v_roll, Vector3_mag(v_roll) / (2 * mu_roll * g)));
    Vector3 w_roll = Vector3_cross(v_roll, (Vector3){0, 0, -1 / R});
    printf("Ball must start rolling at ");
    Vector3_print(roll_position);

    double a = 0.25 * mu_slide * mu_slide * g * g;
    double b = 0;
    double c = -Vector3_mag(v_roll) * Vector3_mag(v_roll);
    double d = 2 * v_roll.x * (roll_position.x - initial_position.x) + 2 * v_roll.y * (roll_position.y - initial_position.y);
    double e = -(initial_position.x - roll_position.x) * (initial_position.x - roll_position.x) - (initial_position.y - roll_position.y) * (initial_position.y - roll_position.y);

    double x1, x2, x3, x4;
    solve_quartic(a, b, c, d, e, &x1, &x2, &x3, &x4);
    printf("%f %f %f %f\n", x1, x2, x3, x4);
    // Find smallest root greater than zero
    double t = INFINITY;
    if (x1 > 0 && x1 < t)
    {
        t = x1;
    }
    if (x2 > 0 && x2 < t)
    {
        t = x2;
    }
    if (x3 > 0 && x3 < t)
    {
        t = x3;
    }
    if (x4 > 0 && x4 < t)
    {
        t = x4;
    }
    printf("%f\n", t);
    printf("Not accounting for acceleration the ball would be at ");
    Vector3_print(Vector3_subtract(roll_position, Vector3_scalar_multiply(v_roll, t)));
    printf("Acceleration must cause ball to travel ");
    Vector3_print(Vector3_subtract(Vector3_subtract(roll_position, Vector3_scalar_multiply(v_roll, t)), initial_position));
    Vector3 required_acceleration = Vector3_add(Vector3_subtract(initial_position, roll_position), Vector3_scalar_multiply(v_roll, t));
    required_acceleration = Vector3_scalar_multiply(Vector3_normalize(required_acceleration), mu_slide * g);
    printf("Acceleration must be ");
    Vector3_print(Vector3_normalize(required_acceleration));

    printf("Initial velocity must be ");
    Vector3 required_velocity = Vector3_subtract(v_roll, Vector3_scalar_multiply(required_acceleration, t));
    Vector3_print(required_velocity);
    *v = required_velocity;

    Vector3 required_angular_acceleration = Vector3_cross(required_acceleration, (Vector3){0, 0, -1});
    printf("Required angular  acceleration : ");
    Vector3_print(required_angular_acceleration);
    required_angular_acceleration = Vector3_scalar_multiply(required_angular_acceleration, -2.5 / R);
    printf("Angular velocity when ball starts rolling is ");
    Vector3_print(w_roll);
    Vector3 required_angular_velocity = Vector3_subtract(w_roll, Vector3_scalar_multiply(required_angular_acceleration, t));

    *w = required_angular_velocity;
}

void solve_one_cushion_shot(Scene *scene, Vector3 initial_position, Vector3 target_position, Vector3 v_roll, Cushion cushion, Vector3 *v, Vector3 *w)
{
    double mu_slide = scene->coefficients.mu_slide;
    double mu_roll = scene->coefficients.mu_roll;
    double g = scene->coefficients.g;
    double R = scene->ball_set.balls[0].radius;

    Vector3 cushion_tangent = Vector3_normalize(Vector3_subtract(cushion.p2, cushion.p1));
    double cushion_coord = 0;
    double cushion_length = Vector3_mag(Vector3_subtract(cushion.p2, cushion.p1));
    Vector3 cushion_contact_point;
    while (cushion_coord < cushion_length)
    {
        cushion_contact_point = Vector3_add(cushion.p1, Vector3_scalar_multiply(cushion_tangent, cushion_coord));
        Vector3 v_after_collision;
        Vector3 w_after_collision;
        solve_direct_shot(scene, cushion_contact_point, target_position, v_roll, &v_after_collision, &w_after_collision);
        Vector3 v_normal = Vector3_subtract(v_after_collision, Vector3_scalar_multiply(cushion_tangent, Vector3_dot(v_after_collision, cushion_tangent)));
        Vector3 v_before_collision = Vector3_subtract(v_after_collision, Vector3_scalar_multiply(v_normal, 2));
        Vector3 v_contact_point = Vector3_subtract(v_before_collision, Vector3_cross(w_after_collision, (Vector3){0, 0, R}));
        Vector3 acceleration = Vector3_scalar_multiply(Vector3_normalize(v_contact_point), -mu_slide * g);
        double x1, x2, x3, x4;
        solve_quadratic(0.5 * acceleration.x, -v_contact_point.x, cushion_contact_point.x - initial_position.x, &x1, &x2);
        solve_quadratic(0.5 * acceleration.y, -v_contact_point.y, cushion_contact_point.y - initial_position.y, &x3, &x4);
        printf("%f %f %f %f\n", x1, x2, x3, x4);
        cushion_coord += 10;
    }
}

Table new_table()
{
    Table table;
    table.cushions = malloc(4 * sizeof(Cushion));
    table.num_cushions = 4;
    table.cushion_capacity = 4;
    table.cushions[0] = (Cushion){{100, 100, 0}, {800, 100, 0}};
    table.cushions[1] = (Cushion){{800, 100, 0}, {800, 800, 0}};
    table.cushions[2] = (Cushion){{800, 800, 0}, {100, 800, 0}};
    table.cushions[3] = (Cushion){{100, 800, 0}, {100, 100, 0}};
    return table;
}

void render_table(SDL_Renderer *renderer, Table table)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int i = 0; i < table.num_cushions; i++)
    {
        Cushion cushion = table.cushions[i];
        SDL_RenderDrawLine(renderer, cushion.p1.x, cushion.p1.y, cushion.p2.x, cushion.p2.y);
    }
}

BallSet empty_ball_set()
{
    BallSet ball_set;
    ball_set.balls = malloc(10 * sizeof(Ball));
    ball_set.num_balls = 0;
    ball_set.ball_capacity = 10;
    return ball_set;
}

BallSet standard_ball_set()
{
    BallSet ball_set;
    ball_set.balls = malloc(10 * sizeof(Ball));
    ball_set.num_balls = 10;
    ball_set.ball_capacity = 10;
    for (int i = 0; i < 10; i++)
    {
        Ball ball;
        ball.id = i;
        ball.initial_position = (Vector3){500, 200 + 50 * i, 0};
        ball.colour = 0x010000 * (int)(255 * i / 10.0f) + 0x000101 * (int)(255 * (10 - i) / 10.0f);
        ball.radius = 10;
        ball.mass = 1;
        ball.path = new_path();
        ball_set.balls[i] = ball;
    }
    return ball_set;
}

BallSet test_ball_set()
{
    BallSet ball_set;
    ball_set.balls = malloc(2 * sizeof(Ball));
    ball_set.num_balls = 2;
    ball_set.ball_capacity = 2;
    Ball ball1;
    ball1.id = 0;
    ball1.initial_position = (Vector3){500, 200, 0};
    ball1.colour = 0xFF0000;
    ball1.radius = 5;
    ball1.mass = 1;
    ball1.path = new_path();
    ball_set.balls[0] = ball1;
    Ball ball2;
    ball2.id = 1;
    ball2.initial_position = (Vector3){500, 650, 0};
    ball2.colour = 0x0000FF;
    ball2.radius = 5;
    ball2.mass = 1;
    ball2.path = new_path();
    ball_set.balls[1] = ball2;
    return ball_set;
}

void free_ball_set(BallSet *ball_set)
{
    for (int i = 0; i < ball_set->num_balls; i++)
    {
        free_path(&(ball_set->balls[i].path));
    }
    free(ball_set->balls);
    ball_set->num_balls = 0;
    ball_set->ball_capacity = 0;
}

Vector3 get_ball_position(Ball ball, double time)
{
    for (int i = 0; i < ball.path.num_segments; i++)
    {
        PathSegment segment = ball.path.segments[i];
        if (time >= segment.start_time && time < segment.end_time)
        {
            return get_position(segment, time);
        }
    }
    return ball.initial_position;
}

void render_ball(SDL_Renderer *renderer, Ball ball, double time)
{
    SDL_SetRenderDrawColor(renderer, (ball.colour >> 16) & 0xFF, (ball.colour >> 8) & 0xFF, ball.colour & 0xFF, 255);
    Vector3 position = get_ball_position(ball, time);
    SDL_RenderFillRect(renderer, &(SDL_Rect){position.x - ball.radius, position.y - ball.radius, 2 * ball.radius, 2 * ball.radius});
    render_path(renderer, ball.path);
}

void render_ball_set(SDL_Renderer *renderer, BallSet ball_set, double time)
{
    for (int i = 0; i < ball_set.num_balls; i++)
    {
        render_ball(renderer, ball_set.balls[i], time);
    }
}

void render_scene(SDL_Renderer *renderer, Scene scene, double time)
{
    render_table(renderer, scene.table);
    render_ball_set(renderer, scene.ball_set, time);
}

Scene new_scene()
{
    Scene scene;
    scene.table = new_table();
    scene.ball_set = test_ball_set();
    Coefficients coefficients;
    coefficients.mu_slide = 5.5;
    coefficients.mu_roll = 0.6;
    coefficients.mu_ball_cushion = 0.5;
    coefficients.mu_ball_ball = 0.5;
    coefficients.g = 9.8;
    coefficients.e_ball_ball = 1;
    coefficients.e_ball_cushion = 1;
    coefficients.e_ball_table = 0.9;

    scene.coefficients = coefficients;

    return scene;
}

void render_UI(SDL_Renderer *renderer, Vector3 v, Vector3 w)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &(SDL_Rect){0, 700, 100, 100});
    SDL_RenderFillRect(renderer, &(SDL_Rect){0, 800, 100, 100});
    SDL_RenderFillRect(renderer, &(SDL_Rect){1540, 0, 100, 900});
    SDL_RenderFillRect(renderer, &(SDL_Rect){1440, 0, 100, 900});
    SDL_SetRenderDrawColor(renderer, 180, 0, 180, 255);
    SDL_RenderFillRect(renderer, &(SDL_Rect){1550, 890 - (int)Vector3_mag(w), 80, (int)Vector3_mag(w)});
    SDL_RenderFillRect(renderer, &(SDL_Rect){1450, 890 - (int)Vector3_mag(v), 80, (int)Vector3_mag(v)});

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    Vector3 v_normalized = Vector3_normalize(v);
    Vector3 w_normalized = Vector3_normalize(w);
    SDL_RenderDrawLine(renderer, 50, 750, 50 + 50 * v_normalized.x, 750 + 50 * v_normalized.y);
    SDL_RenderDrawLine(renderer, 50, 850, 50 + 50 * w_normalized.x, 850 + 50 * w_normalized.y);
}

void take_shot(Scene *scene, Vector3 velocity, Vector3 angular_velocity)
{
    Ball *cue_ball = &(scene->ball_set.balls[0]);
    generate_paths(scene, cue_ball, cue_ball->initial_position, velocity, angular_velocity, 0);
    printf("Path generated\n");
    printf("Path has %d segments\n", cue_ball->path.num_segments);
}

void clear_paths(Scene *scene)
{
    for (int i = 0; i < scene->ball_set.num_balls; i++)
    {
        scene->ball_set.balls[i].path.num_segments = 0;
    }
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("SDL2 Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1640, 900, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Scene scene = new_scene();

    Vector3 v = {1, 0, 0};
    Vector3 w = {0, 1, 0};

    Vector3 target_position = {700, 500, 0};
    Vector3 v_roll = {0, 50, 0};

    double time = 0;
    double playback_speed = 0;

    int mode = 0;

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
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    quit = 1;
                }
                else if (event.key.keysym.sym == SDLK_SPACE)
                {
                    mode = (mode + 1) % 3;
                }
                else if (event.key.keysym.sym == SDLK_UP)
                {
                    playback_speed += 0.2;
                }
                else if (event.key.keysym.sym == SDLK_DOWN)
                {
                    playback_speed -= 0.2;
                }
                else if (event.key.keysym.sym == SDLK_RETURN)
                {
                    take_shot(&scene, v, w);
                    time = 0;
                    playback_speed = 0;
                }
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                int mx, my;
                Uint32 mouseState = SDL_GetMouseState(&mx, &my);
                if (mx > 1450 && mx < 1530 && my > 10 && my < 890)
                {
                    v = Vector3_scalar_multiply(Vector3_normalize(v), 890 - my);
                }
                if (mx > 1550 && mx < 1630 && my > 10 && my < 890)
                {
                    w = Vector3_scalar_multiply(Vector3_normalize(w), 890 - my);
                }
                if (mx > 0 && mx < 100 && my > 700 && my < 800)
                {
                    double v_mag = Vector3_mag(v);
                    v = Vector3_normalize((Vector3){mx - 50, my - 750, 0});
                    v = Vector3_scalar_multiply(v, v_mag);
                }
                if (mx > 0 && mx < 100 && my > 800 && my < 900)
                {
                    double w_mag = Vector3_mag(w);
                    w = Vector3_normalize((Vector3){mx - 50, my - 850, 0});
                    w = Vector3_scalar_multiply(w, w_mag);
                }
                if (mode == 0)
                {
                    target_position = (Vector3){mx, my, 0};
                }
                else if (mode == 1)
                {
                }
                else
                {
                    v_roll = Vector3_subtract(target_position, (Vector3){mx, my, 0});
                }
                Vector3 required_velocity;
                Vector3 required_angular_velocity;
                // solve_direct_shot(&scene, scene.ball_set.balls[0].initial_position, target_position, v_roll, &required_velocity, &required_angular_velocity);
                // v = (required_velocity);
                // w = (required_angular_velocity);
                v = Vector3_subtract((Vector3){mx, my, 0}, scene.ball_set.balls[0].initial_position);
                clear_paths(&scene);
                take_shot(&scene, v, w);
            }
        }

        time += playback_speed / 60;

        SDL_SetRenderDrawColor(renderer, 0, 180, 0, 255);
        SDL_RenderClear(renderer);
        render_scene(renderer, scene, time);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect){target_position.x - 5, target_position.y - 5, 10, 10});

        render_UI(renderer, v, w);

        SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}