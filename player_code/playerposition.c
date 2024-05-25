#include "player.h"
#include <stdio.h>

char *name = "Position Player";
char *description = "This is my first attempt at a player that plays positional shots.";

double calc_final_distance(double cut_angle, double spin_ratio, Coefficients c)
{
    double v_cp_x = (1 - spin_ratio) * sin(cut_angle);
    double v_cp_y = -spin_ratio * cos(cut_angle);
    double v_cp_mag = sqrt(v_cp_x * v_cp_x + v_cp_y * v_cp_y);

    double v_roll_x = (5 + 2 * spin_ratio) * sin(cut_angle) / 7;
    double v_roll_y = (2 * spin_ratio) * cos(cut_angle) / 7;
    double v_roll_mag = sqrt(v_roll_x * v_roll_x + v_roll_y * v_roll_y);

    double final_x = (2 * v_cp_mag / (49 * c.mu_slide * c.g)) * ((6 + spin_ratio) * sin(cut_angle)) + (v_roll_mag / (2 * c.mu_roll * c.g)) * v_roll_x;
    double final_y = (2 * v_cp_mag / (49 * c.mu_slide * c.g)) * ((spin_ratio)*cos(cut_angle)) + (v_roll_mag / (2 * c.mu_roll * c.g)) * v_roll_y;

    double final_distance = sqrt(final_x * final_x + final_y * final_y);
    return final_distance;
}

double calc_final_angle(double cut_angle, double spin, Coefficients c)
{
    double v_cp_x = (1 - spin) * sin(cut_angle);
    double v_cp_y = -spin * cos(cut_angle);
    double v_cp_mag = sqrt(v_cp_x * v_cp_x + v_cp_y * v_cp_y);

    double v_roll_x = (5 + 2 * spin) * sin(cut_angle) / 7;
    double v_roll_y = (2 * spin) * cos(cut_angle) / 7;
    double v_roll_mag = sqrt(v_roll_x * v_roll_x + v_roll_y * v_roll_y);

    double final_x = (2 * v_cp_mag / (49 * c.mu_slide * c.g)) * ((6 + spin) * sin(cut_angle)) + (v_roll_mag / (2 * c.mu_roll * c.g)) * v_roll_x;
    double final_y = (2 * v_cp_mag / (49 * c.mu_slide * c.g)) * ((spin)*cos(cut_angle)) + (v_roll_mag / (2 * c.mu_roll * c.g)) * v_roll_y;

    double final_angle = atan2(final_y, final_x);
    return final_angle;
}

double required_spin(double cut_angle, double final_angle, Coefficients c)
{
    if (cut_angle + final_angle > PI / 2)
    {
        printf("Impossible shot\n");
        return 0;
    }
    double upper = 100;
    double lower = -100;

    int loops = 0;

    while (upper - lower > 0.0001 && loops < 1000)
    {
        double spin = (upper + lower) / 2;
        double final = calc_final_angle(cut_angle, spin, c);
        if (final < final_angle)
        {
            lower = spin;
        }
        else
        {
            upper = spin;
        }
        loops++;
    }
    return (upper + lower) / 2;
}

bool line_is_blocked(Game *game, Vector3 p1, Vector3 p2, Ball *ball)
{
    for (int i = 0; i < game->scene.ball_set.num_balls; i++)
    {
        Ball *current_ball = &(game->scene.ball_set.balls[i]);
        if (current_ball->id == ball->id)
        {
            continue;
        }
        Vector3 p_object = current_ball->initial_position;
        Vector3 shot_tangent = Vector3Normalize(Vector3Subtract(p2, p1));
        Vector3 shot_normal = Vector3CrossProduct(shot_tangent, (Vector3){0, 0, 1});
        double d_normal = Vector3DotProduct(Vector3Subtract(p_object, p1), shot_normal);
        double d_tangent = Vector3DotProduct(Vector3Subtract(p_object, p1), shot_tangent);
        if (fabs(d_normal) < 2 * ball->radius && d_tangent > 0 && d_tangent < Vector3Length(Vector3Subtract(p2, p1)))
        {
            return true;
        }
    }
    return false;
}

bool direct_shot(Game *game, Ball *ball)
{
    Ball *next_ball = NULL;
    for (int i = 0; i < game->scene.ball_set.num_balls; i++)
    {
        Ball *current_ball = &(game->scene.ball_set.balls[i]);
        if (game->scene.ball_set.balls[i].id == ball->id || game->scene.ball_set.balls[i].id == 0 || game->scene.ball_set.balls[i].pocketed)
        {
            continue;
        }
        next_ball = current_ball;
        break;
    }
    for (int i = 0; i < game->scene.table.num_pockets; i++)
    {
        Ball *cue_ball = &(game->scene.ball_set.balls[0]);
        Vector3 cue_ball_position = game->scene.ball_set.balls[0].initial_position;
        Vector3 pocket = game->scene.table.pockets[i].position;
        Vector3 aim_point = Vector3Subtract(ball->initial_position, Vector3Scale(Vector3Normalize(Vector3Subtract(pocket, ball->initial_position)), 2 * ball->radius));
        Vector3 aim_line = Vector3Subtract(aim_point, cue_ball_position);
        Vector3 shot_line = Vector3Subtract(pocket, ball->initial_position);
        double dot_product = Vector3DotProduct(Vector3Normalize(aim_line), Vector3Normalize(shot_line));
        bool cuttable = dot_product > 0.2;
        bool object_ball_blocked = line_is_blocked(game, ball->initial_position, pocket, ball);
        bool cue_ball_blocked = line_is_blocked(game, cue_ball_position, aim_point, cue_ball);

        if (cuttable && !cue_ball_blocked && !object_ball_blocked)
        {
            Vector3 target_vector;
            if (next_ball == NULL)
            {
                target_vector = Vector3Subtract((Vector3){0, 0, 0}, aim_point);
            }
            else
            {
                target_vector = Vector3Subtract(next_ball->initial_position, aim_point);
            }
            double target_distance = Vector3Length(target_vector);
            Vector3 tangent_line = Vector3Normalize(Vector3CrossProduct(Vector3Normalize(shot_line), (Vector3){0, 0, 1}));
            double target_angle = acos(Vector3DotProduct(Vector3Normalize(target_vector), tangent_line));
            if (Vector3DotProduct(target_vector, shot_line) < 0)
            {
                target_angle *= -1;
            }
            double shot_distance = Vector3Length(shot_line);
            double g = game->scene.coefficients.g;
            double mu_s = game->scene.coefficients.mu_slide;
            double mu_r = game->scene.coefficients.mu_roll;
            double factor = ((double)12 / (49 * mu_s * g)) + ((double)25 / (98 * mu_r * g));
            double ob_speed = sqrt(shot_distance / factor);
            double cb_impact_speed = ob_speed / dot_product;
            double cut_angle = acos(dot_product);
            printf("Cut angle: %f\n", cut_angle);
            printf("Target angle: %f\n", target_angle);
            double v;
            double spin_ratio = required_spin(cut_angle, target_angle, game->scene.coefficients);
            printf("Required spin ratio at impact: %f\n", spin_ratio);
            printf("Normlised distance: %f\n", calc_final_distance(cut_angle, spin_ratio, game->scene.coefficients));
            v = sqrt(target_distance / calc_final_distance(cut_angle, spin_ratio, game->scene.coefficients));
            printf("Required impact speed: %f\n", v);
            cb_impact_speed = v;
            double impact_spin = spin_ratio * cb_impact_speed / (ball->radius);
            double aim_distance = Vector3Length(aim_line);
            double cb_speed;
            double spin;
            if (spin_ratio > 1)
            {
                printf("Ball needs to be overrolling\n");
                double min_speed = sqrt(2 * g * mu_s * aim_distance);
                // cb_impact_speed = fmax(cb_impact_speed, 1.2 * min_speed);
                impact_spin = spin_ratio * cb_impact_speed / (ball->radius);
                printf("Impact speed: %f\n", cb_impact_speed);
                printf("Impact spin: %f\n", impact_spin);
                cb_speed = sqrt(pow(cb_impact_speed, 2) - 2 * g * mu_s * aim_distance);
                printf("Speed: %f\n", cb_speed);
                spin = impact_spin - 5 * (cb_speed - cb_impact_speed) / (2 * ball->radius);
                printf("Spin: %f\n", spin);
            }
            else
            {
                printf("Ball needs to be under rolling\n");
                cb_speed = sqrt(pow(cb_impact_speed, 2) + 2 * g * mu_s * aim_distance);
                spin = impact_spin - 5 * (cb_speed - cb_impact_speed) / (2 * ball->radius);
            }
            printf("Required spin ratio: %f\n", spin * ball->radius / cb_speed);
            game->v = Vector3Scale(Vector3Normalize(aim_line), cb_speed);
            game->w = Vector3Scale(Vector3Normalize(Vector3CrossProduct(aim_line, (Vector3){0, 0, -1})), spin);
            return true;
        }
    }
    return false;
}

bool position_shot(Game *game, Ball *ball, Vector3 target_position)
{
    for (int i = 0; i < game->scene.table.num_pockets; i++)
    {
        Ball *cue_ball = &(game->scene.ball_set.balls[0]);
        Vector3 cue_ball_position = game->scene.ball_set.balls[0].initial_position;
        Vector3 pocket = game->scene.table.pockets[i].position;
        Vector3 aim_point = Vector3Subtract(ball->initial_position, Vector3Scale(Vector3Normalize(Vector3Subtract(pocket, ball->initial_position)), 2 * ball->radius));
        Vector3 aim_line = Vector3Subtract(aim_point, cue_ball_position);
        Vector3 shot_line = Vector3Subtract(pocket, ball->initial_position);
        Vector3 tangent_line = Vector3Normalize(Vector3Subtract(aim_line, Vector3Scale(Vector3Normalize(shot_line), Vector3DotProduct(aim_line, Vector3Normalize(shot_line)))));
        double dot_product = Vector3DotProduct(Vector3Normalize(aim_line), Vector3Normalize(shot_line));
        bool cuttable = dot_product > 0.2;
        bool object_ball_blocked = line_is_blocked(game, ball->initial_position, pocket, ball);
        bool cue_ball_blocked = line_is_blocked(game, cue_ball_position, aim_point, cue_ball);

        if (cuttable && !cue_ball_blocked && !object_ball_blocked)
        {
            if (Vector3DotProduct(Vector3Subtract(target_position, aim_point), tangent_line) < 0)
            {
                continue;
            }
            Vector3 target_vector = Vector3Subtract(target_position, aim_point);
            printf("Target vector: %f, %f\n", target_vector.x, target_vector.y);
            printf("Tangent line: %f, %f\n", tangent_line.x, tangent_line.y);
            double target_distance = Vector3Length(target_vector);
            double target_angle = acos(Vector3DotProduct(Vector3Normalize(target_vector), tangent_line));
            printf("Target angle: %f\n", target_angle);
            if (Vector3DotProduct(target_vector, shot_line) < 0)
            {
                target_angle *= -1;
            }
            double shot_distance = Vector3Length(shot_line);
            double g = game->scene.coefficients.g;
            double mu_s = game->scene.coefficients.mu_slide;
            double mu_r = game->scene.coefficients.mu_roll;
            double factor = ((double)12 / (49 * mu_s * g)) + ((double)25 / (98 * mu_r * g));
            double ob_speed = sqrt(shot_distance / factor);
            double cb_impact_speed = ob_speed / dot_product;
            double cut_angle = acos(dot_product);
            printf("Cut angle: %f\n", cut_angle);
            printf("Target angle: %f\n", target_angle);
            double v;
            double spin_ratio = required_spin(cut_angle, target_angle, game->scene.coefficients);
            printf("Required spin ratio at impact: %f\n", spin_ratio);
            printf("Normlised distance: %f\n", calc_final_distance(cut_angle, spin_ratio, game->scene.coefficients));
            v = sqrt(target_distance / calc_final_distance(cut_angle, spin_ratio, game->scene.coefficients));
            printf("Required impact speed: %f\n", v);
            cb_impact_speed = v;
            double impact_spin = spin_ratio * cb_impact_speed / (ball->radius);
            double aim_distance = Vector3Length(aim_line);
            double cb_speed;
            double spin;
            if (spin_ratio > 1)
            {
                printf("Ball needs to be overrolling\n");
                double min_speed = sqrt(2 * g * mu_s * aim_distance);
                // cb_impact_speed = fmax(cb_impact_speed, 1.2 * min_speed);
                impact_spin = spin_ratio * cb_impact_speed / (ball->radius);
                printf("Impact speed: %f\n", cb_impact_speed);
                printf("Impact spin: %f\n", impact_spin);
                cb_speed = sqrt(pow(cb_impact_speed, 2) - 2 * g * mu_s * aim_distance);
                printf("Speed: %f\n", cb_speed);
                spin = impact_spin - 5 * (cb_speed - cb_impact_speed) / (2 * ball->radius);
                printf("Spin: %f\n", spin);
            }
            else
            {
                printf("Ball needs to be under rolling\n");
                cb_speed = sqrt(pow(cb_impact_speed, 2) + 2 * g * mu_s * aim_distance);
                spin = impact_spin - 5 * (cb_speed - cb_impact_speed) / (2 * ball->radius);
            }
            printf("Required spin ratio: %f\n", spin * ball->radius / cb_speed);
            game->v = Vector3Scale(Vector3Normalize(aim_line), cb_speed);
            game->w = Vector3Scale(Vector3Normalize(Vector3CrossProduct(aim_line, (Vector3){0, 0, -1})), spin);
            return true;
        }
    }
    return false;
}

Vector3 find_target_position(Game *game, Ball *ball)
{

    for (int i = 0; i < game->scene.table.num_pockets; i++)
    {
        Ball *cue_ball = &(game->scene.ball_set.balls[0]);
        Vector3 cue_ball_position = game->scene.ball_set.balls[0].initial_position;
        Vector3 pocket = game->scene.table.pockets[i].position;
        Vector3 aim_point = Vector3Subtract(ball->initial_position, Vector3Scale(Vector3Normalize(Vector3Subtract(pocket, ball->initial_position)), 2 * ball->radius));
        Vector3 aim_line = Vector3Subtract(aim_point, cue_ball_position);
        Vector3 shot_line = Vector3Subtract(pocket, ball->initial_position);
        Vector3 tangent_line = Vector3Normalize(Vector3Subtract(aim_line, Vector3Scale(Vector3Normalize(shot_line), Vector3DotProduct(aim_line, Vector3Normalize(shot_line)))));
        double dot_product = Vector3DotProduct(Vector3Normalize(aim_line), Vector3Normalize(shot_line));
        bool cuttable = dot_product > 0.2;
        bool object_ball_blocked = line_is_blocked(game, ball->initial_position, pocket, ball);

        Vector3 target_point = Vector3Subtract(aim_point, Vector3Scale(Vector3Normalize(shot_line), 12 * ball->radius));

        if (!object_ball_blocked)
        {
            return target_point;
        }
    }

    return Vector3Zero();
}

void pot_ball(Game *game, Ball *ball)
{
    Ball *next_ball = NULL;
    bool found_ball = false;
    for (int i = 0; i < game->scene.ball_set.num_balls; i++)
    {
        Ball *current_ball = &(game->scene.ball_set.balls[i]);
        if (current_ball == ball)
        {
            found_ball = true;
            continue;
        }
        if (found_ball)
        {
            next_ball = current_ball;
            break;
        }
    }
    Vector3 target_position;
    if (next_ball != NULL)
    {
        target_position = find_target_position(game, next_ball);
    }
    else
    {
        target_position = (Vector3){0, 0, 0};
    }
    printf("Target position: %f, %f\n", target_position.x, target_position.y);
    if (position_shot(game, ball, target_position))
    {
        return;
    }
    game->v = (Vector3){0, 0, 0};
    game->w = Vector3Zero();
    return;
}