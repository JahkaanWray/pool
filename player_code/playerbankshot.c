#include "player.h"
#include <stdio.h>

char *name = "Bank Shot Player";
char *description = "This player can play bank shots.";

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
    for (int i = 0; i < game->scene.table.num_pockets; i++)
    {
        printf("Checking pocket %d\n", i);
        Ball *cue_ball = &(game->scene.ball_set.balls[0]);
        Vector3 cue_ball_position = game->scene.ball_set.balls[0].initial_position;
        Vector3 pocket = game->scene.table.pockets[i].position;
        Vector3 aim_point = Vector3Subtract(ball->initial_position, Vector3Scale(Vector3Normalize(Vector3Subtract(pocket, ball->initial_position)), 2 * ball->radius));
        Vector3 aim_line = Vector3Subtract(aim_point, cue_ball_position);
        Vector3 shot_line = Vector3Subtract(pocket, ball->initial_position);
        double dot_product = Vector3DotProduct(Vector3Normalize(aim_line), Vector3Normalize(shot_line));
        printf("Dot product: %f\n", dot_product);
        bool cuttable = dot_product > 0.7;
        bool object_ball_blocked = line_is_blocked(game, ball->initial_position, pocket, ball);
        bool cue_ball_blocked = line_is_blocked(game, cue_ball_position, aim_point, cue_ball);

        printf("Target ball: %f %f %f\n", ball->initial_position.x, ball->initial_position.y, ball->initial_position.z);
        printf("Aim point: %f %f %f\n", aim_point.x, aim_point.y, aim_point.z);
        printf("Aim line: %f %f %f\n", aim_line.x, aim_line.y, aim_line.z);
        printf("Cuttable: %d\n", cuttable);
        printf("Object ball blocked: %d\n", object_ball_blocked);
        printf("Cue ball blocked: %d\n", cue_ball_blocked);
        if (cuttable && !cue_ball_blocked && !object_ball_blocked)
        {
            game->v = Vector3Scale(Vector3Normalize(aim_line), 800);
            printf("Direct shot\n");
            return true;
        }
    }
    printf("No direct shot\n");
    return false;
}

bool bank_shot(Game *game, Ball *ball)
{
    for (int i = 0; i < game->scene.table.num_pockets; i++)
    {
        for (int j = 0; j < game->scene.table.num_cushions; j++)
        {
            // Reflect pocket position in cushion
            Vector3 cue_ball_position = game->scene.ball_set.balls[0].initial_position;
            Vector3 pocket = game->scene.table.pockets[i].position;
            Vector3 p1 = game->scene.table.cushions[j].p1;
            Vector3 p2 = game->scene.table.cushions[j].p2;
            Vector3 cushion_normal = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(p2, p1), (Vector3){0, 0, 1}));
            Vector3 pocket_image = Vector3Subtract(pocket, Vector3Scale(cushion_normal, 2 * Vector3DotProduct(cushion_normal, Vector3Subtract(pocket, p1))));
            if (pocket.x == pocket_image.x && pocket.y == pocket_image.y && pocket.z == pocket_image.z)
            {
                continue;
            }
            printf("Pocket image: %f %f %f\n", pocket_image.x, pocket_image.y, pocket_image.z);
            Vector3 shot_line = Vector3Subtract(pocket_image, ball->initial_position);
            Vector3 collision_point = Vector3Add(ball->initial_position, Vector3Scale(shot_line, (Vector3DotProduct(Vector3Subtract(p1, ball->initial_position), cushion_normal)) / (Vector3DotProduct(shot_line, cushion_normal))));
            Vector3 shot1 = Vector3Subtract(collision_point, ball->initial_position);
            Vector3 shot2 = Vector3Subtract(pocket_image, collision_point);
            (void)shot2;
            Vector3 aim_point = Vector3Subtract(ball->initial_position, Vector3Scale(Vector3Normalize(shot1), 2 * ball->radius));
            Vector3 aim_line = Vector3Subtract(aim_point, cue_ball_position);
            bool cuttable = Vector3DotProduct(Vector3Normalize(shot1), Vector3Normalize(aim_line)) > 0.7;
            bool object_ball_blocked1 = line_is_blocked(game, ball->initial_position, collision_point, ball);
            bool object_ball_blocked2 = line_is_blocked(game, collision_point, pocket, ball);
            bool cue_ball_blocked = line_is_blocked(game, cue_ball_position, aim_point, &(game->scene.ball_set.balls[0]));
            printf("Collision point: %f %f %f\n", collision_point.x, collision_point.y, collision_point.z);
            if (cuttable && !object_ball_blocked1 && !object_ball_blocked2 && !cue_ball_blocked)
            {
                game->v = Vector3Scale(Vector3Normalize(aim_line), 900);
                printf("Bank shot\n");
                return true;
            }
        }
    }
    printf("No bank shot\n");
    return false;
}

void pot_ball(Game *game, Ball *ball)
{
    if (direct_shot(game, ball))
    {
        return;
    }
    if (bank_shot(game, ball))
    {
        return;
    }
    game->v = (Vector3){1000, 0, 0};
    game->w = (Vector3){0, 0, 0};
    return;
}