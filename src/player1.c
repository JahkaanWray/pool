#include "player.h"

char *description = "This player finds the first ball that can be potted and calculates the required power to pot it. If no balls can be potted, it hits the cue ball directly at the target ball with a fixed power.";

void pot_ball(Game *game, Ball *ball)
{
    Ball *target_ball = ball;
    Ball *cue_ball = &(game->scene.ball_set.balls[0]);

    bool object_ball_blocked = false;
    bool cue_ball_blocked = false;
    bool cuttable = false;

    for (int i = 0; i < game->scene.table.num_pockets; i++)
    {
        Vector3 p_pocket = game->scene.table.pockets[i].position;
        Vector3 p_target = target_ball->initial_position;
        Vector3 p_cue = cue_ball->initial_position;
        Vector3 aim_point = Vector3Subtract(p_target, Vector3Scale(Vector3Normalize(Vector3Subtract(p_pocket, p_target)), 2 * target_ball->radius));
        Vector3 aim_line = Vector3Subtract(aim_point, p_cue);
        Vector3 shot_line = Vector3Subtract(p_pocket, p_target);
        double dot_product = Vector3DotProduct(Vector3Normalize(aim_line), Vector3Normalize(shot_line));
        double aim_distance = Vector3Length(aim_line);
        double shot_distance = Vector3Length(shot_line);
        if (dot_product < 0.2)
        {
            continue;
        }
        cuttable = true;
        for (int j = 0; j < game->scene.ball_set.num_balls; j++)
        {
            Ball *current_ball = &(game->scene.ball_set.balls[j]);
            if (current_ball->id == target_ball->id)
            {
                continue;
            }
            Vector3 p_object = current_ball->initial_position;
            Vector3 shot_tangent = Vector3Normalize(shot_line);
            Vector3 shot_normal = Vector3CrossProduct(shot_tangent, (Vector3){0, 0, 1});
            double d_normal = Vector3DotProduct(Vector3Subtract(p_object, p_target), shot_normal);
            double d_tangent = Vector3DotProduct(Vector3Subtract(p_object, p_target), shot_tangent);
            if (fabs(d_normal) < 2 * target_ball->radius && d_tangent > 0 && d_tangent < shot_distance)
            {
                object_ball_blocked = true;
                break;
            }
        }

        for (int j = 0; j < game->scene.ball_set.num_balls; j++)
        {
            Ball *current_ball = &(game->scene.ball_set.balls[j]);
            if (current_ball->id != 0 || current_ball->id == target_ball->id)
            {
                continue;
            }

            Vector3 p_object = current_ball->initial_position;
            Vector3 aim_tangent = Vector3Normalize(aim_line);
            Vector3 aim_normal = Vector3CrossProduct(aim_tangent, (Vector3){0, 0, 1});
            double d_normal = Vector3DotProduct(Vector3Subtract(current_ball->initial_position, p_cue), aim_normal);
            double d_tangent = Vector3DotProduct(Vector3Subtract(current_ball->initial_position, p_cue), aim_tangent);
            if (fabs(d_normal) < 2 * target_ball->radius && d_tangent > 0 && d_tangent < aim_distance)
            {
                cue_ball_blocked = true;
                break;
            }
        }

        if (!object_ball_blocked && !cue_ball_blocked && cuttable)
        {
            double ob_speed = sqrt(2 * game->scene.coefficients.mu_slide * game->scene.coefficients.g * shot_distance);
            double cb_speed_impact = ob_speed / dot_product;
            double cb_speed = sqrt(cb_speed_impact * cb_speed_impact + 2 * game->scene.coefficients.mu_slide * game->scene.coefficients.g * aim_distance);
            game->v = Vector3Scale(Vector3Normalize(aim_line), cb_speed);
            game->w = (Vector3){0, 0, 0};
            return;
        }

        game->v = Vector3Scale(Vector3Normalize(Vector3Subtract(p_target, p_cue)), 800);
    }
}
