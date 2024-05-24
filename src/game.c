#include "game.h"
#include "polynomial.h"
#include <stdlib.h>
#include <raylib.h>
#include <assert.h>

Vector3 world_to_screen(Vector3 position)
{
    double scale = 200;
    return (Vector3){200 * position.x, 200 * position.y, 0};
}

int meters_to_pixels(double meters)
{
    return (int)(200 * meters);
}

void update_stats(Game *game)
{
    game->p1_stats = (Stats){0, 0, 0};
    game->p2_stats = (Stats){0, 0, 0};
    for (int i = 0; i < game->num_frames; i++)
    {
        Frame frame = game->frames[i];
        for (int j = 0; j < frame.num_shots; j++)
        {
            Shot shot = frame.shot_history[j];
            if (shot.player == &(game->players[0]))
            {
                game->p1_stats.num_shots++;
            }
            else if (shot.player == &(game->players[1]))
            {
                game->p2_stats.num_shots++;
            }
            for (int k = 0; k < shot.num_events; k++)
            {
                ShotEvent event = shot.events[k];
                if (event.type == BALL_POCKETED)
                {
                    if (event.ball1->id == 0)
                    {
                        if (shot.player == &(game->players[0]))
                        {
                            game->p1_stats.num_fouls++;
                        }
                        else if (shot.player == &(game->players[1]))
                        {
                            game->p2_stats.num_fouls++;
                        }
                    }
                    else
                    {
                        if (shot.player == &(game->players[0]))
                        {
                            game->p1_stats.num_pots++;
                        }
                        else if (shot.player == &(game->players[1]))
                        {
                            game->p2_stats.num_pots++;
                        }
                    }
                }
            }
        }
    }
}

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
        (void)acceleration;
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

bool detect_ball_cushion_collision(Game *game, Ball ball, Cushion *cushion, double *t)
{
    double collision_time = INFINITY;
    PathSegment *segment = &(ball.path.segments[ball.path.num_segments - 1]);
    Vector3 p1 = segment->initial_position;
    Vector3 v1 = segment->initial_velocity;
    Vector3 line_normal = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(cushion->p2, cushion->p1), (Vector3){0, 0, 1}));
    double sn = Vector3DotProduct(Vector3Subtract(cushion->p1, p1), line_normal);
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
    Shot *current_shot = &(game->current_shot);
    ShotEvent *events = current_shot->events;
    ShotEvent last_event = {NONE, NULL, NULL, NULL, NULL, 0};
    if (current_shot->num_events > 0)
    {
        last_event = events[current_shot->num_events - 1];
    }
    double min_time = last_event.time;

    if (last_event.type == BALL_CUSHION_COLLISION)
    {
        if (last_event.ball1->id == ball.id && last_event.cushion == cushion)
        {
            repeat_collision = true;
        }
    }
    double tolerance = repeat_collision ? 1e-3 : 0;
    if (collision_time1 > segment->start_time + tolerance && collision_time1 < segment->end_time && collision_time1 > min_time && collision_time1 < collision_time)
    {
        collision_time = collision_time1;
    }
    if (collision_time2 > segment->start_time + tolerance && collision_time2 < segment->end_time && collision_time2 > min_time && collision_time2 < collision_time)
    {
        collision_time = collision_time2;
    }
    if (collision_time == INFINITY)
    {
        return false;
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
    Shot *current_shot = &(game->current_shot);
    ShotEvent *events = current_shot->events;
    ShotEvent last_event = {NONE, NULL, NULL, NULL, NULL, 0};
    if (current_shot->num_events > 0)
    {
        last_event = events[current_shot->num_events - 1];
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
    Vector3 a1 = segment1->acceleration;
    double t1 = segment1->start_time;
    double r2 = pocket.radius;

    if (segment1->rolling)
    {
        Vector3 p3 = get_position(*segment1, segment1->end_time);
        double a = (p3.x - p1.x) * (p3.x - p1.x) + (p3.y - p1.y) * (p3.y - p1.y);
        double b = 2 * ((p3.x - p1.x) * (p1.x - p2.x) + (p3.y - p1.y) * (p1.y - p2.y));
        double c = (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y) - (r2) * (r2);
        double x1, x2;
        solve_quadratic(a, b, c, &x1, &x2);
        double x = INFINITY;
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
        double distance = x * Vector3Length(Vector3Subtract(p3, p1));
        double v = Vector3Length(v1);
        a = -Vector3Length(a1);
        solve_quadratic(0.5 * a, v, -distance, &x1, &x2);
        double collision_time = INFINITY;
        if (x1 < collision_time && x1 > 0)
        {
            collision_time = x1;
        }
        if (x2 < collision_time && x2 > 0)
        {
            collision_time = x2;
        }
        *t = collision_time + segment1->start_time;
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
    double e = (C1 - C2) * (C1 - C2) + (C3 - C4) * (C3 - C4) - (r2) * (r2);

    double x1, x2, x3, x4;
    solve_quartic(a, b, c, d, e, &x1, &x2, &x3, &x4);

    double collision_time = INFINITY;
    bool repeat_collision = false;
    Shot *current_shot = &(game->current_shot);
    ShotEvent *events = current_shot->events;
    ShotEvent last_event = {NONE, NULL, NULL, NULL, NULL, 0};
    if (current_shot->num_events > 0)
    {
        last_event = events[current_shot->num_events - 1];
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

void resolve_ball_cushion_collision(Ball *ball, Cushion *cushion, double time, Coefficients coefficients)
{
    PathSegment *segment = &(ball->path.segments[ball->path.num_segments - 1]);
    Vector3 p = get_position(*segment, time);
    Vector3 v = get_velocity(*segment, time);
    Vector3 w = get_angular_velocity(*segment, time);
    Vector3 normal = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(cushion->p2, cushion->p1), (Vector3){0, 0, 1}));
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
    (void)pocket;
    Vector3 p;
    if (ball->id == 0)
    {
        p = (Vector3){0.3, 0.3, 0};
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
    PathSegment stop_segment = {p, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, false, time, INFINITY};
    add_segment(&(ball->path), stop_segment);
}

bool update_path(Game *game)
{
    ShotEventType update_type = NONE;
    double first_time = INFINITY;
    double time = INFINITY;
    Cushion *cushion;
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
            Cushion *current_cushion = &(game->scene.table.cushions[j]);
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
    ShotEvent event = {update_type, ball1, ball2, cushion, &pocket, first_time};
    Shot *current_shot = &(game->current_shot);
    if (current_shot->num_events > 0)
    {
        assert(first_time >= current_shot->events[current_shot->num_events - 1].time);
    }
    shot_add_event(current_shot, event);
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
        Vector3 p1 = world_to_screen(segment.initial_position);
        Vector3 p2 = world_to_screen(get_position(segment, segment.end_time));
        DrawLine(p1.x, p1.y, p2.x, p2.y, BLUE);
    }
    else
    {
        for (int i = 0; i < 100; i++)
        {
            double t1 = segment.start_time + i * (segment.end_time - segment.start_time) / 100;
            double t2 = segment.start_time + (i + 1) * (segment.end_time - segment.start_time) / 100;
            Vector3 p1 = world_to_screen(get_position(segment, t1));
            Vector3 p2 = world_to_screen(get_position(segment, t2));
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
    (void)v;
    (void)w;
    double mu_slide = scene->coefficients.mu_slide;
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
        cushion_coord += 10;
    }
}

void render_table(Table table)
{
    for (int i = 0; i < table.num_cushions; i++)
    {
        Cushion cushion = table.cushions[i];
        Vector3 p1 = world_to_screen(cushion.p1);
        Vector3 p2 = world_to_screen(cushion.p2);
        DrawLine(p1.x, p1.y, p2.x, p2.y, BLACK);
    }
    for (int i = 0; i < table.num_pockets; i++)
    {
        Pocket pocket = table.pockets[i];
        Vector3 p = world_to_screen(pocket.position);
        DrawCircle(p.x, p.y, meters_to_pixels(pocket.radius), BLACK);
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
        ball.initial_position = (Vector3){1, 1 + 0.2 * i, 0};
        ball.radius = 0.05;
        ball.mass = 0.16;
        ball.path = new_path();
        ball_set.balls[i] = ball;
        ball_set.balls[i].pocketed = false;
    }
    ball_set.balls[0].initial_position.x = 0.5;
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

Table create_table()
{
    Table table;
    table.cushions = malloc(4 * sizeof(Cushion));
    table.num_cushions = 4;
    table.cushion_capacity = 4;
    table.cushions[0] = (Cushion){{0, 0, 0}, {2, 0, 0}};
    table.cushions[1] = (Cushion){{2, 0, 0}, {2, 4, 0}};
    table.cushions[2] = (Cushion){{2, 4, 0}, {0, 4, 0}};
    table.cushions[3] = (Cushion){{0, 4, 0}, {0, 0, 0}};
    table.num_pockets = 4;
    table.pockets = malloc(table.num_pockets * sizeof(Pocket));
    table.pocket_capacity = table.num_pockets;
    table.pockets[0] = (Pocket){{0, 0, 0}, 0.15};
    table.pockets[1] = (Pocket){{2, 0, 0}, 0.15};
    table.pockets[2] = (Pocket){{2, 4, 0}, 0.15};
    table.pockets[3] = (Pocket){{0, 4, 0}, 0.15};
    return table;
}

Scene create_scene()
{
    Scene scene;
    scene.table = create_table();
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

void setup_new_frame(Game *game)
{
    if (game->num_frames == game->frame_capacity)
    {
        game->frame_capacity *= 2;
        game->frames = realloc(game->frames, game->frame_capacity * sizeof(Frame));
    }
    Frame *current_frame = &(game->frames[game->num_frames++]);
    current_frame->num_shots = 0;
    current_frame->winner = NULL;
    current_frame->shot_capacity = 10;
    current_frame->shot_history = malloc(current_frame->shot_capacity * sizeof(Shot));

    for (int i = 0; i < game->scene.ball_set.num_balls; i++)
    {
        Ball *ball = &(game->scene.ball_set.balls[i]);
        ball->pocketed = false;
        ball->path.num_segments = 0;
        ball->initial_position = (Vector3){(double)GetRandomValue(30, 170) / 100, 0.3 + 0.2 * i, 0};
    }
}
bool apply_game_rules(Game *game)
{
    Frame *current_frame = &(game->frames[game->num_frames - 1]);
    if (current_frame->num_shots >= 200)
    {
        current_frame->winner = &(game->players[(game->current_player + 1) % game->num_players]);
        game->consecutive_fouls = 0;
        setup_new_frame(game);
        return false;
    }
    if (game->consecutive_fouls >= 3)
    {
        current_frame->winner = &(game->players[(game->current_player + 1) % game->num_players]);
        game->consecutive_fouls = 0;
        setup_new_frame(game);
        return false;
    }
    Ball *cue_ball = &(game->scene.ball_set.balls[0]);
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
    if (target_ball == NULL)
    {
        game->current_player = (game->current_player + 1) % game->num_players;
        game->consecutive_fouls++;
        return false;
    }
    Shot last_shot = game->frames[game->num_frames - 1].shot_history[game->frames[game->num_frames - 1].num_shots - 1];
    bool ball_potted = false;
    bool legal_first_hit = false;
    bool nine_ball_potted = false;
    bool cue_ball_potted = false;
    for (int i = 0; i < last_shot.num_events; i++)
    {
        ShotEvent event = last_shot.events[i];
        if (event.type == BALL_BALL_COLLISION && ((event.ball1->id == 0 && event.ball2->id == target_ball->id) || (event.ball1->id == target_ball->id && event.ball2->id == 0)))
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
        }
    }
    cue_ball->pocketed = false;
    if (nine_ball_potted)
    {
        if (!legal_first_hit || cue_ball_potted)
        {
            current_frame->winner = &(game->players[(game->current_player + 1) % game->num_players]);
        }
        else
        {
            current_frame->winner = &(game->players[game->current_player]);
        }
        game->consecutive_fouls = 0;
        setup_new_frame(game);
    }
    if (!legal_first_hit)
    {
        game->current_player = (game->current_player + 1) % game->num_players;
        game->consecutive_fouls++;
        return false;
    }
    if (!ball_potted)
    {
        game->current_player = (game->current_player + 1) % game->num_players;
        return false;
    }
    if (legal_first_hit && cue_ball_potted)
    {
        game->current_player = (game->current_player + 1) % game->num_players;
        game->consecutive_fouls++;
        return false;
    }
    game->consecutive_fouls = 0;
    return legal_first_hit && ball_potted;
}

void generate_shot(Game *game, Vector3 velocity, Vector3 angular_velocity)
{
    Ball *cue_ball = &(game->scene.ball_set.balls[0]);
    game->current_shot.num_events = 0;
    generate_paths(game, cue_ball, cue_ball->initial_position, velocity, angular_velocity, 0);
    double end_time = 0;
    for (int i = 0; i < game->scene.ball_set.num_balls; i++)
    {
        Path path = game->scene.ball_set.balls[i].path;
        game->current_shot.ball_paths[i] = game->scene.ball_set.balls[i].path;
        game->current_shot.ball_paths[i].num_segments = path.num_segments;
        game->current_shot.ball_paths[i].segments = malloc(path.num_segments * sizeof(PathSegment));
        game->current_shot.ball_paths[i].capacity = path.num_segments;
        for (int j = 0; j < path.num_segments; j++)
        {
            game->current_shot.ball_paths[i].segments[j] = path.segments[j];
        }
        PathSegment last_segment = path.segments[path.num_segments - 1];
        if (last_segment.start_time > end_time)
        {
            end_time = last_segment.start_time;
        }
    }
    game->current_shot.end_time = end_time + 1;
}

void clear_paths(Scene *scene)
{
    for (int i = 0; i < scene->ball_set.num_balls; i++)
    {
        scene->ball_set.balls[i].path.num_segments = 0;
    }
}

void take_shot(Game *game)
{
    Frame *current_frame = &(game->frames[game->num_frames - 1]);
    Shot current_shot = game->current_shot;
    if (current_frame->num_shots == current_frame->shot_capacity)
    {
        current_frame->shot_capacity *= 2;
        current_frame->shot_history = realloc(current_frame->shot_history, current_frame->shot_capacity * sizeof(Shot));
    }
    current_shot.player = &(game->players[game->current_player]);
    current_frame->shot_history[current_frame->num_shots++] = current_shot;
    Shot shot;
    shot.ball_paths = malloc(game->scene.ball_set.num_balls * sizeof(Path));
    shot.events = malloc(10 * sizeof(ShotEvent));
    shot.num_events = 0;
    shot.event_capacity = 10;
    game->current_shot = shot;
}

Game *create_game(Player *players, int num_players)
{
    Game *game = malloc(sizeof(Game));
    game->scene = create_scene();
    game->num_players = num_players;
    game->players = malloc(sizeof(Player) * num_players);
    for (int i = 0; i < num_players; i++)
    {
        game->players[i] = players[i];
        game->players[i].game = game;
    }
    game->current_player = 0;
    game->num_frames = 0;
    game->frame_capacity = 10;
    game->frames = malloc(sizeof(Frame) * game->frame_capacity);
    setup_new_frame(game);
    Shot shot;
    shot.ball_paths = malloc(game->scene.ball_set.num_balls * sizeof(Path));
    shot.event_capacity = 10;
    shot.events = malloc(shot.event_capacity * sizeof(ShotEvent));
    shot.num_events = 0;
    game->current_shot = shot;

    game->state = BEFORE_SHOT;
    game->consecutive_fouls = 0;
    game->time = 0;
    game->playback_speed = 1;
    game->default_playback_speed = 1;

    game->v = (Vector3){0, 0, 0};
    game->w = (Vector3){0, 0, 0};

    game->p1_stats = (Stats){0, 0, 0};
    game->p2_stats = (Stats){0, 0, 0};
    return game;
}

void update_game(Game *game)
{
    if (IsKeyPressed(KEY_UP))
    {
        if (game->state == DURING_SHOT)
        {
            game->playback_speed += 2;
            game->default_playback_speed += 2;
        }
    }
    else if (IsKeyPressed(KEY_DOWN))
    {
        if (game->state == DURING_SHOT)
        {
            game->playback_speed -= 2;
            game->default_playback_speed -= 2;
        }
    }
    else if (IsKeyPressed(KEY_SPACE))
    {
        if (game->state == DURING_SHOT)
        {
            if (game->playback_speed == 0)
            {
                game->playback_speed = game->default_playback_speed;
            }
            else
            {
                game->playback_speed = 0;
            }
        }
    }
    else if (IsKeyPressed(KEY_R))
    {
        update_stats(game);
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
                if (game->w.x == 0 && game->w.y == 0 && game->w.z == 0)
                {
                    game->w = Vector3Scale((Vector3){1, 0, 0}, 890 - my);
                }
                else
                {
                    game->w = Vector3Scale(Vector3Normalize(game->w), 890 - my);
                }
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
            game->v = Vector3Scale(Vector3Subtract((Vector3){mx, my, 0}, world_to_screen(game->scene.ball_set.balls[0].initial_position)), 0.05);
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
            PlayerPotBallFunction pot_ball_function = game->players[game->current_player].module.pot_ball;
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
        Frame *current_frame = &(game->frames[game->num_frames - 1]);
        if (game->time > current_frame->shot_history[current_frame->num_shots - 1].end_time)
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

void render_ball(Ball ball, double time)
{
    Vector3 position = world_to_screen(get_ball_position(ball, time));
    DrawCircle(position.x, position.y, meters_to_pixels(ball.radius), ball.colour);
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

    Frame current_frame = game->frames[game->num_frames - 1];

    for (int i = 0; i < current_frame.num_shots; i++)
    {
        for (int j = 0; j < current_frame.shot_history[i].num_events; j++)
        {
            ShotEvent event = current_frame.shot_history[i].events[j];
            if (event.type == BALL_POCKETED)
            {
                Ball *ball = event.ball1;
                DrawCircle(900, 20 + 30 * i, 10, ball->colour);
                DrawText("Potted", 920, 10 + 30 * i, 20, WHITE);
                break;
            }
        }
    }

    if (current_frame.num_shots != 0)
    {

        for (int i = 0; i < current_frame.shot_history[current_frame.num_shots - 1].num_events; i++)
        {
            ShotEvent event = current_frame.shot_history[current_frame.num_shots - 1].events[i];
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

    DrawText("Stats", 10, 250, 20, WHITE);

    char player1_stats[100];
    sprintf(player1_stats, "Player 1: %d shots, %d pots, %d fouls", game->p1_stats.num_shots, game->p1_stats.num_pots, game->p1_stats.num_fouls);
    DrawText(player1_stats, 10, 280, 20, WHITE);

    char player2_stats[100];
    sprintf(player2_stats, "Player 2: %d shots, %d pots, %d fouls", game->p2_stats.num_shots, game->p2_stats.num_pots, game->p2_stats.num_fouls);
    DrawText(player2_stats, 10, 310, 20, WHITE);

    DrawFPS(120, 850);
}

void render_game(Game *game)
{
    ClearBackground(GREEN);
    render_scene(game->scene, game->time);
    render_UI(game, game->v, game->w);
}