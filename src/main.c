#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <complex.h>
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
    Color colour;
    double radius;
    double mass;
    Path path;
    bool pocketed;
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

struct Game;

typedef void (*PlayerPotBallFunction)(struct Game *game, Ball *ball);
typedef struct
{
    PlayerType type;
    struct Game *game;
    PlayerPotBallFunction pot_ball;
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
    double time;
} ShotEvent;

typedef struct
{
    Path *ball_paths;
    ShotEvent *events;
    int num_events;
    int event_capacity;
    double end_time;
} Shot;

typedef enum
{
    BEFORE_SHOT,
    DURING_SHOT,
    AFTER_SHOT
} GameState;

typedef struct
{
    Shot *shot_history;
    int num_shots;
    int shot_capacity;

    Player *winner;
} Frame;

typedef enum
{
    MAIN_MENU,
    GAMEPLAY,
    GAME_OVER
} GameScreen;

typedef struct Game
{
    GameScreen screen;

    Scene scene;
    Player *players;
    int num_players;
    int current_player;

    Shot current_shot;

    Frame current_frame;
    Frame *frames;
    int num_frames;
    int frame_capacity;

    GameState state;

    double time;
    double playback_speed;
    double default_playback_speed;

    Vector3 v;
    Vector3 w;
} Game;

void shot_add_event(Shot *shot, ShotEvent event)
{
    if (shot->num_events == shot->event_capacity)
    {
        if (shot->event_capacity == 0)
        {
            shot->event_capacity = 1;
        }
        else
        {
            shot->event_capacity *= 2;
        }
        shot->events = realloc(shot->events, shot->event_capacity * sizeof(ShotEvent));
    }
    shot->events[shot->num_events] = event;
    shot->num_events++;
}

Vector3 get_position(PathSegment segment, double time)
{
    if (time < segment.start_time)
    {
        return segment.initial_position;
    }
    if (time > segment.end_time)
    {
        return Vector3Add(segment.initial_position, Vector3Add(Vector3Scale(segment.initial_velocity, segment.end_time - segment.start_time), Vector3Scale(segment.acceleration, 0.5 * (segment.end_time - segment.start_time) * (segment.end_time - segment.start_time))));
    }
    Vector3 p = Vector3Add(segment.initial_position, Vector3Add(Vector3Scale(segment.initial_velocity, time - segment.start_time), Vector3Scale(segment.acceleration, 0.5 * (time - segment.start_time) * (time - segment.start_time))));
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
        return Vector3Add(segment.initial_velocity, Vector3Scale(segment.acceleration, segment.end_time - segment.start_time));
    }
    Vector3 v = Vector3Add(segment.initial_velocity, Vector3Scale(segment.acceleration, time - segment.start_time));
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
        return Vector3Add(segment.initial_angular_velocity, Vector3Scale(segment.angular_acceleration, segment.end_time - segment.start_time));
    }
    return Vector3Add(segment.initial_angular_velocity, Vector3Scale(segment.angular_acceleration, time - segment.start_time));
}
void print_path(Path path)
{
    for (int i = 0; i < path.num_segments; i++)
    {
        PathSegment segment = path.segments[i];
        Vector3 acceleration = segment.acceleration;
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
    Vector3 contact_point_v = Vector3Subtract(initial_velocity, Vector3CrossProduct(initial_angular_velocity, (Vector3){0, 0, R}));
    Vector3 acceleration = Vector3Scale(Vector3Normalize(contact_point_v), -mu_slide * g);
    Vector3 angular_acceleration = Vector3Scale(Vector3CrossProduct(acceleration, (Vector3){0, 0, -1}), -2.5 / R);
    double end_time = start_time + 2 * Vector3Length(contact_point_v) / (7 * mu_slide * g);
    PathSegment segment = {initial_position, initial_velocity, acceleration, initial_angular_velocity, angular_acceleration, false, start_time, end_time};
    ball->path.segments[ball->path.num_segments - 1].end_time = start_time;
    add_segment(&(ball->path), segment);
}

void add_rolling_segment(Ball *ball, Vector3 initial_position, Vector3 initial_velocity, double start_time, Coefficients coefficients)
{
    double mu_roll = coefficients.mu_roll;
    double g = coefficients.g;
    double R = ball->radius;
    Vector3 acceleration = Vector3Scale(Vector3Normalize(initial_velocity), -mu_roll * g);
    Vector3 initial_angular_velocity = Vector3CrossProduct(initial_velocity, (Vector3){0, 0, -1 / R});
    Vector3 angular_acceleration = Vector3Scale(Vector3CrossProduct(acceleration, (Vector3){0, 0, -1}), 1 / R);
    double end_time = start_time + Vector3Length(initial_velocity) / (mu_roll * g);
    PathSegment segment = {initial_position, initial_velocity, acceleration, initial_angular_velocity, angular_acceleration, true, start_time, end_time};
    add_segment(&(ball->path), segment);
}

void free_path(Path *path)
{
    free(path->segments);
    path->num_segments = 0;
    path->capacity = 0;
}

bool detect_ball_cushion_collision(Game *game, Ball ball, Cushion line_segment, double *t)
{
    double collision_time;
    PathSegment *segment = &(ball.path.segments[ball.path.num_segments - 1]);
    Vector3 p1 = segment->initial_position;
    Vector3 v1 = segment->initial_velocity;
    Vector3 w1 = segment->initial_angular_velocity;
    Vector3 line_normal = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(line_segment.p2, line_segment.p1), (Vector3){0, 0, 1}));
    double sn = Vector3DotProduct(Vector3Subtract(line_segment.p1, p1), line_normal);
    double vn = Vector3DotProduct(v1, line_normal);
    Vector3 a = segment->acceleration;
    double an = Vector3DotProduct(a, line_normal);
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
    bool repeat_collision = false;
    ShotEvent *events = game->current_shot.events;
    ShotEvent last_event = {NONE, NULL, NULL, NULL, NULL, 0};
    if (game->current_shot.num_events > 0)
    {
        last_event = events[game->current_shot.num_events - 1];
    }
    double min_time = last_event.time;

    if (last_event.type == BALL_CUSHION_COLLISION)
    {
        if (last_event.ball1->id == ball.id && last_event.cushion->p1.x == line_segment.p1.x && last_event.cushion->p1.y == line_segment.p1.y && last_event.cushion->p2.x == line_segment.p2.x && last_event.cushion->p2.y == line_segment.p2.y)
        {
            repeat_collision = true;
        }
    }
    double tolerance = repeat_collision ? 1e-3 : 0;
    if (collision_time1 <= segment->start_time + tolerance || collision_time1 > segment->end_time || collision_time1 < min_time)
    {
        collision_time1 = -1;
    }
    if (collision_time2 <= segment->start_time + tolerance || collision_time2 > segment->end_time || collision_time2 < min_time)
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

bool detect_ball_ball_collision(Game *game, Ball ball1, Ball ball2, double *t)
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
    bool repeat_collision = false;
    ShotEvent *events = game->current_shot.events;
    ShotEvent last_event = {NONE, NULL, NULL, NULL, NULL, 0};
    if (game->current_shot.num_events > 0)
    {
        last_event = events[game->current_shot.num_events - 1];
    }
    double min_time = last_event.time;

    if (last_event.type == BALL_BALL_COLLISION)
    {
        if ((last_event.ball1->id == ball1.id && last_event.ball2->id == ball2.id) || (last_event.ball1->id == ball2.id && last_event.ball2->id == ball1.id))
        {
            repeat_collision = true;
        }
    }
    double tolerance = repeat_collision ? 1e-3 : 0;
    if (x1 > segment1->start_time + tolerance && x1 < segment1->end_time && x1 > segment2->start_time + tolerance && x1 < segment2->end_time && x1 < collision_time && x1 > min_time)
    {
        collision_time = x1;
    }
    if (x2 > segment1->start_time + tolerance && x2 < segment1->end_time && x2 > segment2->start_time + tolerance && x2 < segment2->end_time && x2 < collision_time && x2 > min_time)
    {
        collision_time = x2;
    }
    if (x3 > segment1->start_time + tolerance && x3 < segment1->end_time && x3 > segment2->start_time + tolerance && x3 < segment2->end_time && x3 < collision_time && x3 > min_time)
    {
        collision_time = x3;
    }
    if (x4 > segment1->start_time + tolerance && x4 < segment1->end_time && x4 > segment2->start_time + tolerance && x4 < segment2->end_time && x4 < collision_time && x4 > min_time)
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

bool detect_ball_pocket_collision(Game *game, Ball ball, Pocket pocket, double *t)
{
    PathSegment *segment1 = &(ball.path.segments[ball.path.num_segments - 1]);
    Vector3 p1 = segment1->initial_position;
    Vector3 p2 = pocket.position;
    Vector3 v1 = segment1->initial_velocity;
    Vector3 v2 = {0, 0, 0};
    Vector3 a1 = segment1->acceleration;
    Vector3 a2 = {0, 0, 0};
    double t1 = segment1->start_time;
    double r1 = ball.radius;
    double r2 = pocket.radius;

    if (segment1->rolling)
    {
        Vector3 p3 = get_position(*segment1, segment1->end_time);
        printf("Initial position: %f %f\n", p1.x, p1.y);
        printf("Final position: %f %f\n", p3.x, p3.y);
        printf("Pocket position: %f %f\n", p2.x, p2.y);
        printf("Line vector: %f %f\n", p3.x - p1.x, p3.y - p1.y);
        double a = (p3.x - p1.x) * (p3.x - p1.x) + (p3.y - p1.y) * (p3.y - p1.y);
        double b = 2 * ((p3.x - p1.x) * (p1.x - p2.x) + (p3.y - p1.y) * (p1.y - p2.y));
        double c = (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y) - (r1 + r2) * (r1 + r2);
        double x1, x2;
        printf("a: %f\n", a);
        printf("b: %f\n", b);
        printf("c: %f\n", c);
        printf("b^2 %f\n", b * b);
        printf("4ac %f\n", 4 * a * c);
        printf("discriminant: %f\n", b * b - 4 * a * c);
        solve_quadratic(a, b, c, &x1, &x2);
        double x = INFINITY;
        printf("x1: %f, x2: %f\n", x1, x2);
        if (x1 > 0 && x1 < 1 && x1 < x)
        {
            x = x1;
        }
        if (x2 > 0 && x2 < 1 && x2 < x)
        {
            x = x2;
        }
        if (x == INFINITY)
        {
            return false;
        }
        printf("x: %f\n", x);
        double distance = x * Vector3Length(Vector3Subtract(p3, p1));
        printf("Distance: %f\n", distance);
        double v = Vector3Length(v1);
        printf("Speed: %f\n", v);
        a = -Vector3Length(a1);
        printf("Acceleration: %f\n", a);
        solve_quadratic(0.5 * a, v, -distance, &x1, &x2);
        double collision_time = INFINITY;
        printf("x1: %f, x2: %f\n", x1, x2);
        if (x1 < collision_time && x1 > 0)
        {
            collision_time = x1;
        }
        if (x2 < collision_time && x2 > 0)
        {
            collision_time = x2;
        }
        *t = collision_time + segment1->start_time;
        printf("Collision time: %f\n", *t);
        printf("\n\n\n\n");
        return true;
    }

    double A1 = 0.5 * a1.x;
    double B1 = v1.x - a1.x * t1;
    double C1 = p1.x - v1.x * t1 + 0.5 * a1.x * t1 * t1;

    double C2 = p2.x;

    double A3 = 0.5 * a1.y;
    double B3 = v1.y - a1.y * t1;
    double C3 = p1.y - v1.y * t1 + 0.5 * a1.y * t1 * t1;

    double C4 = p2.y;

    double a = (A1) * (A1) + (A3) * (A3);
    double b = 2 * ((A1) * (B1) + (A3) * (B3));
    double c = 2 * ((A1) * (C1 - C2) + (A3) * (C3 - C4)) + (B1) * (B1) + (B3) * (B3);
    double d = 2 * ((B1) * (C1 - C2) + (B3) * (C3 - C4));
    double e = (C1 - C2) * (C1 - C2) + (C3 - C4) * (C3 - C4) - (r1 + r2) * (r1 + r2);

    double x1, x2, x3, x4;
    solve_quartic(a, b, c, d, e, &x1, &x2, &x3, &x4);

    double collision_time = INFINITY;
    bool repeat_collision = false;
    ShotEvent *events = game->current_shot.events;
    ShotEvent last_event = {NONE, NULL, NULL, NULL, NULL, 0};
    if (game->current_shot.num_events > 0)
    {
        last_event = events[game->current_shot.num_events - 1];
    }

    double min_time = last_event.time;

    if (last_event.type == BALL_POCKETED)
    {
        if (last_event.ball1->id == ball.id)
        {
            repeat_collision = true;
        }
    }
    double tolerance = repeat_collision ? 1e-3 : 0;
    if (x1 > segment1->start_time + tolerance && x1 < segment1->end_time && x1 < collision_time && x1 > min_time)
    {
        collision_time = x1;
    }
    if (x2 > segment1->start_time + tolerance && x2 < segment1->end_time && x2 < collision_time && x2 > min_time)
    {
        collision_time = x2;
    }
    if (x3 > segment1->start_time + tolerance && x3 < segment1->end_time && x3 < collision_time && x3 > min_time)
    {
        collision_time = x3;
    }
    if (x4 > segment1->start_time + tolerance && x4 < segment1->end_time && x4 < collision_time && x4 > min_time)
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
    Vector3 normal = Vector3Normalize(Vector3Subtract(p2, p1));
    Vector3 tangent = Vector3CrossProduct(normal, (Vector3){0, 0, 1});
    double e = coefficients.e_ball_ball;
    double m1 = ball1->mass;
    double m2 = ball2->mass;
    double v1n = Vector3DotProduct(v1, normal);
    double v2n = Vector3DotProduct(v2, normal);
    double v1t = Vector3DotProduct(v1, tangent);
    double v2t = Vector3DotProduct(v2, tangent);
    double v1n_final = (v1n * (m1 - e * m2) + 2 * e * m2 * v2n) / (m1 + m2);
    double v2n_final = (v2n * (m2 - e * m1) + 2 * e * m1 * v1n) / (m1 + m2);
    double v1t_final = v1t;
    double v2t_final = v2t;

    Vector3 v1_final = Vector3Add(Vector3Scale(normal, v1n_final), Vector3Scale(tangent, v1t_final));
    Vector3 v2_final = Vector3Add(Vector3Scale(normal, v2n_final), Vector3Scale(tangent, v2t_final));
    add_sliding_segment(ball1, p1, v1_final, w1, time, coefficients);
    add_sliding_segment(ball2, p2, v2_final, w2, time, coefficients);
}

void resolve_ball_cushion_collision(Ball *ball, Cushion cushion, double time, Coefficients coefficients)
{
    PathSegment *segment = &(ball->path.segments[ball->path.num_segments - 1]);
    Vector3 p = get_position(*segment, time);
    Vector3 v = get_velocity(*segment, time);
    Vector3 w = get_angular_velocity(*segment, time);
    Vector3 normal = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(cushion.p2, cushion.p1), (Vector3){0, 0, 1}));
    Vector3 tangent = Vector3CrossProduct(normal, (Vector3){0, 0, 1});
    double e = coefficients.e_ball_cushion;
    double v_n = Vector3DotProduct(v, normal);
    double v_t = Vector3DotProduct(v, tangent);
    double v_n_final = -e * v_n;
    double v_t_final = v_t;
    Vector3 v_final = Vector3Add(Vector3Scale(normal, v_n_final), Vector3Scale(tangent, v_t_final));
    add_sliding_segment(ball, p, v_final, w, time, coefficients);
}

void resolve_ball_pocket_collision(Ball *ball, Pocket pocket, double time, Coefficients coefficients)
{
    PathSegment *segment = &(ball->path.segments[ball->path.num_segments - 1]);
    Vector3 p;
    if (ball->id == 0)
    {
        p = (Vector3){500, 300, 0};
    }
    else
    {
        p = Vector3Add((Vector3){1000, 200, 0}, Vector3Scale((Vector3){0, 50, 0}, ball->id));
    }
    Vector3 v = {0, 0, 0};
    Vector3 w = {0, 0, 0};
    add_sliding_segment(ball, p, v, w, time, coefficients);
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

bool update_path(Game *game)
{
    ShotEventType update_type = NONE;
    double first_time = INFINITY;
    double time = INFINITY;
    Cushion cushion;
    Ball *ball1;
    Ball *ball2;
    Pocket pocket;
    for (int i = 0; i < game->scene.ball_set.num_balls; i++)
    {
        Ball *current_ball = &(game->scene.ball_set.balls[i]);
        for (int j = i + 1; j < game->scene.ball_set.num_balls; j++)
        {
            Ball *other_ball = &(game->scene.ball_set.balls[j]);
            if (detect_ball_ball_collision(game, *current_ball, *other_ball, &time))
            {
                if (time < first_time)
                {
                    first_time = time;
                    update_type = BALL_BALL_COLLISION;
                    ball1 = current_ball;
                    ball2 = other_ball;
                }
            }
        }

        for (int j = 0; j < game->scene.table.num_cushions; j++)
        {
            Cushion current_cushion = game->scene.table.cushions[j];
            if (detect_ball_cushion_collision(game, *current_ball, current_cushion, &time))
            {
                if (time < first_time)
                {
                    first_time = time;
                    update_type = BALL_CUSHION_COLLISION;
                    ball1 = current_ball;
                    cushion = current_cushion;
                }
            }
        }

        for (int j = 0; j < game->scene.table.num_pockets; j++)
        {
            Pocket current_pocket = game->scene.table.pockets[j];
            if (detect_ball_pocket_collision(game, *current_ball, current_pocket, &time))
            {
                if (time < first_time)
                {
                    first_time = time;
                    update_type = BALL_POCKETED;
                    ball1 = current_ball;
                    pocket = current_pocket;
                }
            }
        }
    }
    double mu_slide = game->scene.coefficients.mu_slide;
    double mu_roll = game->scene.coefficients.mu_roll;
    double g = game->scene.coefficients.g;

    for (int i = 0; i < game->scene.ball_set.num_balls; i++)
    {
        Ball *current_ball = &(game->scene.ball_set.balls[i]);
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
    if (update_type == BALL_BALL_COLLISION)
    {
        resolve_ball_ball_collision(ball1, ball2, first_time, game->scene.coefficients);
    }
    else if (update_type == BALL_CUSHION_COLLISION)
    {
        resolve_ball_cushion_collision(ball1, cushion, first_time, game->scene.coefficients);
    }
    else if (update_type == BALL_POCKETED)
    {
        resolve_ball_pocket_collision(ball1, pocket, first_time, game->scene.coefficients);
    }
    else if (update_type == BALL_ROLL)
    {
        resolve_roll(ball1, first_time, game->scene.coefficients);
    }
    else if (update_type == BALL_STOP)
    {
        resolve_stop(ball1, first_time);
    }
    ShotEvent event = {update_type, ball1, ball2, &cushion, &pocket, first_time};
    shot_add_event(&(game->current_shot), event);
    printf("Detected event at time %f\n", first_time);
    return true;
}

void generate_paths(Game *game, Ball *ball, Vector3 initial_position, Vector3 initial_velocity, Vector3 initial_angular_velocity, double start_time)
{
    for (int i = 0; i < game->scene.ball_set.num_balls; i++)
    {
        Ball *current_ball = &(game->scene.ball_set.balls[i]);
        if (current_ball->id == ball->id)
        {
            continue;
        }
        PathSegment segment = {current_ball->initial_position, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, false, 0, INFINITY};
        add_segment(&(current_ball->path), segment);
    }
    double mu_slide = game->scene.coefficients.mu_slide;
    double mu_roll = game->scene.coefficients.mu_roll;
    double g = game->scene.coefficients.g;
    double R = ball->radius;
    double end_time;

    Vector3 contact_point_v = Vector3Subtract(initial_velocity, Vector3CrossProduct(initial_angular_velocity, (Vector3){0, 0, R}));
    Vector3 acceleration = Vector3Scale(Vector3Normalize(contact_point_v), -mu_slide * g);
    Vector3 angular_acceleration = Vector3Scale(Vector3CrossProduct(acceleration, (Vector3){0, 0, -1}), -2.5 / R);

    end_time = start_time + 2 * Vector3Length(contact_point_v) / (7 * mu_slide * g);
    PathSegment segment = {initial_position, initial_velocity, acceleration, initial_angular_velocity, angular_acceleration, false, start_time, end_time};
    add_segment(&(ball->path), segment);
    while (update_path(game))
        ;
}

void render_path_segment(PathSegment segment)
{
    if (segment.rolling)
    {
        Vector3 p1 = segment.initial_position;
        Vector3 p2 = get_position(segment, segment.end_time);
        DrawLine(p1.x, p1.y, p2.x, p2.y, BLUE);
    }
    else
    {
        for (int i = 0; i < 100; i++)
        {
            double t1 = segment.start_time + i * (segment.end_time - segment.start_time) / 100;
            double t2 = segment.start_time + (i + 1) * (segment.end_time - segment.start_time) / 100;
            Vector3 p1 = get_position(segment, t1);
            Vector3 p2 = get_position(segment, t2);
            DrawLine(p1.x, p1.y, p2.x, p2.y, RED);
        }
    }
}

void render_path(Path path)
{
    for (int i = 0; i < path.num_segments; i++)
    {
        PathSegment segment = path.segments[i];
        render_path_segment(segment);
    }
}

void pot_ball(Game *game, Ball *ball, int power)
{
    if (ball == NULL)
    {
        return;
    }
    Vector3 p2 = ball->initial_position;
    Vector3 p3 = game->scene.ball_set.balls[0].initial_position;
    Vector3 aim_point;
    bool shot_found = false;
    for (int i = 0; i < game->scene.table.num_pockets; i++)
    {
        Vector3 p1 = game->scene.table.pockets[i].position;
        Vector3 normal = Vector3Normalize(Vector3Subtract(p2, p1));
        aim_point = Vector3Add(p2, Vector3Scale(normal, 2 * ball->radius));
        bool object_ball_blocked = false;
        for (int j = 0; j < game->scene.ball_set.num_balls; j++)
        {
            Ball *current_ball = &(game->scene.ball_set.balls[j]);
            if (current_ball->id == ball->id)
            {
                continue;
            }
            Vector3 p4 = current_ball->initial_position;
            double distance;
            Vector3 aim_line = Vector3Normalize(Vector3Subtract(aim_point, p2));
            Vector3 aim_line_normal = Vector3CrossProduct(aim_line, (Vector3){0, 0, 1});
            distance = Vector3DotProduct(Vector3Subtract(p4, p1), aim_line_normal);
            if (fabs(distance) < 2 * ball->radius && Vector3DotProduct(Vector3Subtract(p4, p1), aim_line) > 0)
            {
                object_ball_blocked = true;
                break;
            }
        }
        bool cue_ball_blocked = false;
        for (int j = 0; j < game->scene.ball_set.num_balls; j++)
        {
            Ball *current_ball = &(game->scene.ball_set.balls[j]);
            if (current_ball->id != 0)
            {
                continue;
            }
            Vector3 p4 = current_ball->initial_position;
            double distance;
            Vector3 aim_line = Vector3Normalize(Vector3Subtract(aim_point, p2));
            Vector3 aim_line_normal = Vector3CrossProduct(aim_line, (Vector3){0, 0, 1});
            distance = Vector3DotProduct(Vector3Subtract(p4, p1), aim_line_normal);
            if (fabs(distance) < 2 * ball->radius)
            {
                cue_ball_blocked = true;
                break;
            }
        }
        bool cuttable = false;
        double dot_product = Vector3DotProduct(Vector3Normalize(Vector3Subtract(p1, p2)), Vector3Normalize(Vector3Subtract(aim_point, p3)));

        if (dot_product > 0.2)
        {
            cuttable = true;
        }

        if (!object_ball_blocked && !cue_ball_blocked && cuttable)
        {
            shot_found = true;
            break;
        }
    }
    if (!shot_found)
    {
        game->v = (Vector3){100, 0, 0};
        game->v = Vector3Scale(Vector3Normalize(Vector3Subtract(aim_point, p3)), power);
        game->w = (Vector3){0, 0, 0};

        return;
    }
    game->v = Vector3Scale(Vector3Normalize(Vector3Subtract(aim_point, p3)), power);
    game->w = (Vector3){0, 0, 0};
}

void solve_direct_shot(Scene *scene, Vector3 initial_position, Vector3 target_position, Vector3 v_roll, Vector3 *v, Vector3 *w)
{
    double R = scene->ball_set.balls[0].radius;
    double mu_slide = scene->coefficients.mu_slide;
    double mu_roll = scene->coefficients.mu_roll;
    double g = scene->coefficients.g;

    Vector3 roll_position = Vector3Subtract(target_position, Vector3Scale(v_roll, Vector3Length(v_roll) / (2 * mu_roll * g)));
    Vector3 w_roll = Vector3CrossProduct(v_roll, (Vector3){0, 0, -1 / R});

    double a = 0.25 * mu_slide * mu_slide * g * g;
    double b = 0;
    double c = -Vector3Length(v_roll) * Vector3Length(v_roll);
    double d = 2 * v_roll.x * (roll_position.x - initial_position.x) + 2 * v_roll.y * (roll_position.y - initial_position.y);
    double e = -(initial_position.x - roll_position.x) * (initial_position.x - roll_position.x) - (initial_position.y - roll_position.y) * (initial_position.y - roll_position.y);

    double x1, x2, x3, x4;
    solve_quartic(a, b, c, d, e, &x1, &x2, &x3, &x4);
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
    Vector3 required_acceleration = Vector3Add(Vector3Subtract(initial_position, roll_position), Vector3Scale(v_roll, t));
    required_acceleration = Vector3Scale(Vector3Normalize(required_acceleration), mu_slide * g);

    Vector3 required_velocity = Vector3Subtract(v_roll, Vector3Scale(required_acceleration, t));
    *v = required_velocity;

    Vector3 required_angular_acceleration = Vector3CrossProduct(required_acceleration, (Vector3){0, 0, -1});
    required_angular_acceleration = Vector3Scale(required_angular_acceleration, -2.5 / R);
    Vector3 required_angular_velocity = Vector3Subtract(w_roll, Vector3Scale(required_angular_acceleration, t));

    *w = required_angular_velocity;
}

void solve_one_cushion_shot(Scene *scene, Vector3 initial_position, Vector3 target_position, Vector3 v_roll, Cushion cushion, Vector3 *v, Vector3 *w)
{
    double mu_slide = scene->coefficients.mu_slide;
    double mu_roll = scene->coefficients.mu_roll;
    double g = scene->coefficients.g;
    double R = scene->ball_set.balls[0].radius;

    Vector3 cushion_tangent = Vector3Normalize(Vector3Subtract(cushion.p2, cushion.p1));
    double cushion_coord = 0;
    double cushion_length = Vector3Length(Vector3Subtract(cushion.p2, cushion.p1));
    Vector3 cushion_contact_point;
    while (cushion_coord < cushion_length)
    {
        cushion_contact_point = Vector3Add(cushion.p1, Vector3Scale(cushion_tangent, cushion_coord));
        Vector3 v_after_collision;
        Vector3 w_after_collision;
        solve_direct_shot(scene, cushion_contact_point, target_position, v_roll, &v_after_collision, &w_after_collision);
        Vector3 v_normal = Vector3Subtract(v_after_collision, Vector3Scale(cushion_tangent, Vector3DotProduct(v_after_collision, cushion_tangent)));
        Vector3 v_before_collision = Vector3Subtract(v_after_collision, Vector3Scale(v_normal, 2));
        Vector3 v_contact_point = Vector3Subtract(v_before_collision, Vector3CrossProduct(w_after_collision, (Vector3){0, 0, R}));
        Vector3 acceleration = Vector3Scale(Vector3Normalize(v_contact_point), -mu_slide * g);
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
    table.num_pockets = 4;
    table.pockets = malloc(table.num_pockets * sizeof(Pocket));
    table.pocket_capacity = table.num_pockets;
    table.pockets[0] = (Pocket){100, 100, 0, 20};
    table.pockets[1] = (Pocket){800, 100, 0, 20};
    table.pockets[2] = (Pocket){800, 800, 0, 20};
    table.pockets[3] = (Pocket){100, 800, 0, 20};
    return table;
}

void render_table(Table table)
{
    for (int i = 0; i < table.num_cushions; i++)
    {
        Cushion cushion = table.cushions[i];
        DrawLine(cushion.p1.x, cushion.p1.y, cushion.p2.x, cushion.p2.y, BLACK);
    }
    for (int i = 0; i < table.num_pockets; i++)
    {
        Pocket pocket = table.pockets[i];
        DrawCircle(pocket.position.x, pocket.position.y, pocket.radius, BLACK);
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
        ball.radius = 2;
        ball.mass = 1;
        ball.path = new_path();
        ball_set.balls[i] = ball;
        ball_set.balls[i].pocketed = false;
    }
    ball_set.balls[0].initial_position.x = 700;
    ball_set.balls[0].colour = WHITE;
    ball_set.balls[1].colour = YELLOW;
    ball_set.balls[2].colour = BLUE;
    ball_set.balls[3].colour = RED;
    ball_set.balls[4].colour = PURPLE;
    ball_set.balls[5].colour = ORANGE;
    ball_set.balls[6].colour = DARKGREEN;
    ball_set.balls[7].colour = MAROON;
    ball_set.balls[8].colour = BLACK;
    ball_set.balls[9].colour = GOLD;

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
    ball1.colour = RED;
    ball1.radius = 5;
    ball1.mass = 1;
    ball1.path = new_path();
    ball_set.balls[0] = ball1;
    Ball ball2;
    ball2.id = 1;
    ball2.initial_position = (Vector3){500, 650, 0};
    ball2.colour = BLUE;
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

void render_ball(Ball ball, double time)
{
    Vector3 position = get_ball_position(ball, time);
    DrawCircle(position.x, position.y, ball.radius, ball.colour);
    render_path(ball.path);
}

void render_ball_set(BallSet ball_set, double time)
{
    for (int i = 0; i < ball_set.num_balls; i++)
    {
        render_ball(ball_set.balls[i], time);
    }
}

void render_scene(Scene scene, double time)
{
    render_table(scene.table);
    render_ball_set(scene.ball_set, time);
}

Scene new_scene()
{
    Scene scene;
    scene.table = new_table();
    scene.ball_set = standard_ball_set();
    Coefficients coefficients;
    coefficients.mu_slide = 15.5;
    coefficients.mu_roll = 2.6;
    coefficients.mu_ball_cushion = 0.5;
    coefficients.mu_ball_ball = 0.5;
    coefficients.g = 9.8;
    coefficients.e_ball_ball = 1;
    coefficients.e_ball_cushion = 1;
    coefficients.e_ball_table = 0.9;

    scene.coefficients = coefficients;

    return scene;
}

void render_UI(Game *game, Vector3 v, Vector3 w)
{
    DrawRectangle(0, 700, 100, 100, BLACK);
    DrawRectangle(0, 800, 100, 100, BLACK);
    DrawRectangle(1540, 0, 100, 900, BLACK);
    DrawRectangle(1440, 0, 100, 900, BLACK);

    DrawRectangle(1550, 890 - (int)Vector3Length(w), 80, (int)Vector3Length(w), PINK);
    DrawRectangle(1450, 890 - (int)Vector3Length(v), 80, (int)Vector3Length(v), PINK);

    Vector3 v_normalized = Vector3Normalize(v);
    Vector3 w_normalized = Vector3Normalize(w);
    DrawLine(50, 750, 50 + 50 * v_normalized.x, 750 + 50 * v_normalized.y, WHITE);
    DrawLine(50, 850, 50 + 50 * w_normalized.x, 850 + 50 * w_normalized.y, WHITE);

    DrawFPS(1350, 10);

    if (game->state == BEFORE_SHOT)
    {
        DrawText("Before shot", 10, 10, 20, WHITE);
        DrawText("Press Enter to take shot", 10, 40, 20, WHITE);
    }
    else if (game->state == DURING_SHOT)
    {
        DrawText("During shot", 10, 10, 20, WHITE);
        DrawText("Press Up/Down to change playback speed", 10, 40, 20, WHITE);
    }
    else if (game->state == AFTER_SHOT)
    {
        DrawText("After shot", 10, 10, 20, WHITE);
        DrawText("Press Enter to end shot", 10, 40, 20, WHITE);
    }

    if (game->players[game->current_player].type == HUMAN)
    {
        DrawText("Human player", 10, 70, 20, WHITE);
    }
    else
    {
        DrawText("AI player", 10, 70, 20, WHITE);
    }
    char player_text[100];
    sprintf(player_text, "Player %d", game->current_player + 1);
    DrawText(player_text, 10, 100, 20, WHITE);

    char playback_speed_text[100];
    sprintf(playback_speed_text, "Playback speed: %f", game->playback_speed);
    DrawText(playback_speed_text, 10, 130, 20, WHITE);

    char time_text[100];
    sprintf(time_text, "Time: %f", game->time);
    DrawText(time_text, 10, 160, 20, WHITE);

    char frame_text[100];
    sprintf(frame_text, "Frame: %d", game->num_frames);
    DrawText(frame_text, 10, 190, 20, WHITE);

    for (int i = 0; i < game->current_frame.num_shots; i++)
    {
        for (int j = 0; j < game->current_frame.shot_history[i].num_events; j++)
        {
            ShotEvent event = game->current_frame.shot_history[i].events[j];
            if (event.type == BALL_POCKETED)
            {
                Ball *ball = event.ball1;
                DrawCircle(900, 20 + 30 * i, 10, ball->colour);
                DrawText("Potted", 920, 10 + 30 * i, 20, WHITE);
                break;
            }
        }
    }

    if (game->current_frame.num_shots != 0)
    {

        for (int i = 0; i < game->current_frame.shot_history[game->current_frame.num_shots - 1].num_events; i++)
        {
            ShotEvent event = game->current_frame.shot_history[game->current_frame.num_shots - 1].events[i];
            Color colour;
            if (event.time < game->time)
            {
                colour = RED;
            }
            else
            {
                colour = WHITE;
            }
            if (event.type == BALL_POCKETED)
            {
                Ball *ball = event.ball1;
                DrawCircle(1100, 20 + 30 * i, 10, ball->colour);
                DrawText("Potted", 1120, 10 + 30 * i, 20, colour);
            }
            if (event.type == BALL_BALL_COLLISION)
            {
                Ball *ball1 = event.ball1;
                Ball *ball2 = event.ball2;
                DrawCircle(1100, 20 + 30 * i, 10, ball1->colour);
                DrawCircle(1120, 20 + 30 * i, 10, ball2->colour);
                DrawText("Collision", 1140, 10 + 30 * i, 20, colour);
            }
            if (event.type == BALL_CUSHION_COLLISION)
            {
                Ball *ball = event.ball1;
                DrawCircle(1100, 20 + 30 * i, 10, ball->colour);
                DrawText("Cushion Collision", 1120, 10 + 30 * i, 20, colour);
            }
            if (event.type == BALL_ROLL)
            {
                Ball *ball = event.ball1;
                DrawCircle(1100, 20 + 30 * i, 10, ball->colour);
                DrawText("Roll", 1120, 10 + 30 * i, 20, colour);
            }
            if (event.type == BALL_STOP)
            {
                Ball *ball = event.ball1;
                DrawCircle(1100, 20 + 30 * i, 10, ball->colour);
                DrawText("Stop", 1120, 10 + 30 * i, 20, colour);
            }
            DrawText(TextFormat("%f", event.time), 1300, 10 + 30 * i, 20, colour);
        }
    }

    int *scores = malloc(game->num_players * sizeof(int));
    for (int i = 0; i < game->num_players; i++)
    {
        scores[i] = 0;
    }
    for (int i = 0; i < game->num_frames; i++)
    {
        if (game->frames[i].winner != NULL)
        {
            Player *winner = game->frames[i].winner;

            for (int j = 0; j < game->num_players; j++)
            {
                if (&game->players[j] == winner)
                {
                    scores[j]++;
                }
            }
        }
    }
    for (int i = 0; i < game->num_players; i++)
    {
        char score_text[100];
        sprintf(score_text, "Player %d: %d", i + 1, scores[i]);
        DrawText(score_text, 10, 220 + 30 * i, 20, WHITE);
    }
    free(scores);
}

void generate_shot(Game *game, Vector3 velocity, Vector3 angular_velocity)
{
    Ball *cue_ball = &(game->scene.ball_set.balls[0]);
    game->current_shot.num_events = 0;
    generate_paths(game, cue_ball, cue_ball->initial_position, velocity, angular_velocity, 0);
    double end_time = 0;
    for (int i = 0; i < game->scene.ball_set.num_balls; i++)
    {
        game->current_shot.ball_paths[i] = game->scene.ball_set.balls[i].path;
        Path path = game->scene.ball_set.balls[i].path;
        PathSegment last_segment = path.segments[path.num_segments - 1];
        if (last_segment.start_time > end_time)
        {
            end_time = last_segment.start_time;
        }
    }
    game->current_shot.end_time = end_time;
    printf("Shot generated\n");
    printf("End time: %f\n", end_time);
}

void take_shot(Game *game)
{
    if (game->current_frame.num_shots == game->current_frame.shot_capacity)
    {
        game->current_frame.shot_capacity *= 2;
        game->current_frame.shot_history = realloc(game->current_frame.shot_history, game->current_frame.shot_capacity * sizeof(Shot));
    }
    game->current_frame.shot_history[game->current_frame.num_shots] = game->current_shot;
    game->current_frame.num_shots++;
    Shot shot;
    shot.ball_paths = malloc(game->scene.ball_set.num_balls * sizeof(Path));
    shot.events = malloc(10 * sizeof(ShotEvent));
    shot.num_events = 0;
    shot.event_capacity = 10;
    game->current_shot = shot;
}

void clear_paths(Scene *scene)
{
    for (int i = 0; i < scene->ball_set.num_balls; i++)
    {
        scene->ball_set.balls[i].path.num_segments = 0;
    }
}

void player1_pot_ball(Game *game, Ball *ball)
{
    pot_ball(game, ball, 400);
}

void player2_pot_ball(Game *game, Ball *ball)
{
    pot_ball(game, ball, 800);
}

Game *new_game()
{
    Game *game = malloc(sizeof(Game));
    game->screen = MAIN_MENU;
    game->scene = new_scene();
    game->num_players = 2;
    game->players = malloc(game->num_players * sizeof(Player));
    for (int i = 0; i < game->num_players; i++)
    {
        game->players[i].game = game;
    }
    game->players[0].type = HUMAN;
    game->players[1].type = AI;
    game->players[0].pot_ball = player1_pot_ball;
    game->players[1].pot_ball = player2_pot_ball;
    game->current_player = 0;
    game->v = (Vector3){1, 0, 0};
    game->w = (Vector3){0, 1, 0};
    game->time = 0;
    game->playback_speed = 0;
    game->default_playback_speed = 1;
    game->state = BEFORE_SHOT;
    game->current_frame.num_shots = 0;
    game->current_frame.shot_capacity = 10;
    game->current_frame.shot_history = malloc(game->current_frame.shot_capacity * sizeof(Shot));
    game->current_frame.winner = NULL;
    Shot shot;
    shot.ball_paths = malloc(game->scene.ball_set.num_balls * sizeof(Path));
    shot.events = malloc(10 * sizeof(ShotEvent));
    shot.num_events = 0;
    shot.event_capacity = 10;
    game->current_shot = shot;
    game->num_frames = 0;
    game->frame_capacity = 10;
    game->frames = malloc(game->frame_capacity * sizeof(Frame));
    return game;
}

void free_game(Game *game)
{
    free_ball_set(&(game->scene.ball_set));
    free(game->players);
    free(game);
}

void render_menu()
{
    ClearBackground(RAYWHITE);
    DrawText("Press Enter to start game", 10, 10, 20, BLACK);
}

void render_game(Game *game)
{
    if (game->screen == MAIN_MENU)
    {
        render_menu();
    }
    else if (game->screen == GAMEPLAY)
    {
        ClearBackground(GREEN);
        render_scene(game->scene, game->time);
        render_UI(game, game->v, game->w);
    }
}

void setup_new_frame(Game *game)
{
    game->current_frame.num_shots = 0;
    game->current_frame.winner = NULL;
    game->current_frame.shot_capacity = 10;
    game->current_frame.shot_history = malloc(game->current_frame.shot_capacity * sizeof(Shot));

    for (int i = 0; i < game->scene.ball_set.num_balls; i++)
    {
        Ball *ball = &(game->scene.ball_set.balls[i]);
        ball->pocketed = false;
        ball->path.num_segments = 0;
        ball->initial_position = (Vector3){GetRandomValue(100, 800), GetRandomValue(100, 800), 0};
    }
}

bool apply_game_rules(Game *game)
{
    Ball *cue_ball = &(game->scene.ball_set.balls[0]);
    Ball *target_ball = NULL;
    Ball *nine_ball = &(game->scene.ball_set.balls[9]);
    for (int i = 1; i < game->scene.ball_set.num_balls; i++)
    {
        Ball *ball = &(game->scene.ball_set.balls[i]);
        if (!ball->pocketed)
        {
            target_ball = ball;
            break;
        }
    }
    if (target_ball == NULL)
    {
        game->current_player = (game->current_player + 1) % game->num_players;
        return false;
    }
    Shot last_shot = game->current_frame.shot_history[game->current_frame.num_shots - 1];
    bool ball_potted = false;
    bool legal_first_hit = false;
    bool nine_ball_potted = false;
    bool cue_ball_potted = false;
    for (int i = 0; i < last_shot.num_events; i++)
    {
        ShotEvent event = last_shot.events[i];
        if (event.type == BALL_BALL_COLLISION && (event.ball1->id == 0 && event.ball2->id == target_ball->id || event.ball1->id == target_ball->id && event.ball2->id == 0))
        {
            legal_first_hit = true;
            break;
        }
    }
    for (int i = 0; i < last_shot.num_events; i++)
    {
        ShotEvent event = last_shot.events[i];
        if (event.type == BALL_POCKETED)
        {
            event.ball1->pocketed = true;
            ball_potted = true;
            if (event.ball1->id == 9)
            {
                nine_ball_potted = true;
            }
            if (event.ball1->id == 0)
            {
                cue_ball_potted = true;
            }

            printf("%d ball potted\n", event.ball1->id);
        }
    }
    cue_ball->pocketed = false;
    printf("9 ball potted: %d\n", nine_ball_potted);
    if (nine_ball_potted)
    {
        printf("9 ball potted\n");
        if (!legal_first_hit || cue_ball_potted)
        {
            game->current_frame.winner = &(game->players[(game->current_player + 1) % game->num_players]);
        }
        else
        {
            game->current_frame.winner = &(game->players[game->current_player]);
        }
        if (game->num_frames == game->frame_capacity)
        {
            game->frame_capacity *= 2;
            game->frames = realloc(game->frames, game->frame_capacity * sizeof(Frame));
        }
        game->frames[game->num_frames] = game->current_frame;
        game->num_frames++;
        setup_new_frame(game);
        printf("Frame ended\n");
    }
    if (!legal_first_hit || !ball_potted)
    {
        game->current_player = (game->current_player + 1) % game->num_players;
        return false;
    }
    return legal_first_hit && ball_potted;
}

bool update_game(Game *game)
{
    if (game->screen == MAIN_MENU)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            game->screen = GAMEPLAY;
        }
        return true;
    }
    if (IsKeyPressed(KEY_UP))
    {
        if (game->state == DURING_SHOT)
        {
            game->playback_speed += 0.2;
            game->default_playback_speed += 0.2;
        }
    }
    else if (IsKeyPressed(KEY_DOWN))
    {
        if (game->state == DURING_SHOT)
        {
            game->playback_speed -= 0.2;
            game->default_playback_speed -= 0.2;
        }
    }
    else if (IsKeyPressed(KEY_ENTER))
    {
        if (game->state == BEFORE_SHOT)
        {
            if (game->players[game->current_player].type == HUMAN)
            {
                take_shot(game);
                game->state = DURING_SHOT;
                game->time = 0;
                game->playback_speed = game->default_playback_speed;
            }
        }
        else if (game->state == DURING_SHOT)
        {
        }
        else if (game->state == AFTER_SHOT)
        {
            game->state = BEFORE_SHOT;
        }
    }
    if (game->state == BEFORE_SHOT)
    {
        if (game->players[game->current_player].type == HUMAN)
        {
            int mx, my;
            Vector2 mouse_position = GetMousePosition();
            mx = mouse_position.x;
            my = mouse_position.y;
            if (mx > 1450 && mx < 1530 && my > 10 && my < 890)
            {
                game->v = Vector3Scale(Vector3Normalize(game->v), 890 - my);
            }
            if (mx > 1550 && mx < 1630 && my > 10 && my < 890)
            {
                game->w = Vector3Scale(Vector3Normalize(game->w), 890 - my);
            }
            if (mx > 0 && mx < 100 && my > 700 && my < 800)
            {
                double v_mag = Vector3Length(game->v);
                game->v = Vector3Normalize((Vector3){mx - 50, my - 750, 0});
                game->v = Vector3Scale(game->v, v_mag);
            }
            if (mx > 0 && mx < 100 && my > 800 && my < 900)
            {
                double w_mag = Vector3Length(game->w);
                game->w = Vector3Normalize((Vector3){mx - 50, my - 850, 0});
                game->w = Vector3Scale(game->w, w_mag);
            }
            // solve_direct_shot(&scene, scene.ball_set.balls[0].initial_position, target_position, v_roll, &required_velocity, &required_angular_velocity);
            game->v = Vector3Subtract((Vector3){mx, my, 0}, game->scene.ball_set.balls[0].initial_position);
            clear_paths(&(game->scene));
            generate_shot(game, game->v, game->w);
            game->time = 0;
            game->playback_speed = 0;
        }
        else if (game->players[game->current_player].type == AI)
        {
            // solve_direct_shot(&(game->scene), game->scene.ball_set.balls[0].initial_position, (Vector3){600, 200, 0}, (Vector3){0, 5, 0}, &(game->v), &(game->w));

            Ball *target_ball = NULL;
            for (int i = 1; i < game->scene.ball_set.num_balls; i++)
            {
                Ball *ball = &(game->scene.ball_set.balls[i]);
                if (!ball->pocketed)
                {
                    target_ball = ball;
                    break;
                }
            }
            PlayerPotBallFunction pot_ball_function = game->players[game->current_player].pot_ball;
            pot_ball_function(game, target_ball);
            clear_paths(&(game->scene));
            generate_shot(game, game->v, game->w);
            take_shot(game);
            game->time = 0;
            game->playback_speed = game->default_playback_speed;
            game->state = DURING_SHOT;
        }
    }
    else if (game->state == DURING_SHOT)
    {
        game->time += game->playback_speed / 60;
        if (game->time > game->current_frame.shot_history[game->current_frame.num_shots - 1].end_time)
        {
            game->state = AFTER_SHOT;
        }
    }
    else if (game->state == AFTER_SHOT)
    {
        apply_game_rules(game);

        game->state = BEFORE_SHOT;
        for (int i = 0; i < game->scene.ball_set.num_balls; i++)
        {
            Ball *ball = &(game->scene.ball_set.balls[i]);
            ball->initial_position = get_ball_position(*ball, game->time);
            ball->path.num_segments = 0;
        }
        clear_paths(&(game->scene));
        game->time = 0;
        game->playback_speed = 0;
    }
}

int main(int argc, char *argv[])
{
    const int SCREEN_WIDTH = 1640;
    const int SCREEN_HEIGHT = 900;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pool");

    SetTargetFPS(60);
    Game *game = new_game();

    while (!WindowShouldClose())
    {
        update_game(game);
        BeginDrawing();
        render_game(game);
        EndDrawing();
    }

    free_game(game);

    CloseWindow();
    return 0;
}