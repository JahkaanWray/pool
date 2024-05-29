#ifndef GAME_H
#define GAME_H
#include <raylib.h>
#include "player.h"

struct Game;

struct Ball;

struct Player;

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
    Quaternion *orientations;
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

typedef struct Ball
{
    int id;
    Vector3 initial_position;
    Quaternion initial_orientation;
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
    struct Player *player;
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

    struct Player *winner;
} Frame;

typedef struct
{
    int num_shots;
    int num_pots;
    int num_fouls;
} Stats;

typedef enum
{
    POT,
    MISS,
    FOUL
} ShotType;

typedef struct Game
{
    Scene scene;
    struct Player *players;
    int num_players;
    int current_player;

    Shot current_shot;

    Frame *frames;
    int num_frames;
    int frame_capacity;

    GameState state;

    int consecutive_fouls;

    double time;
    double playback_speed;
    double default_playback_speed;

    Vector3 v;
    Vector3 w;

    Stats p1_stats;
    Stats p2_stats;
} Game;

Game *create_game(struct Player *players, int num_players);

void update_game(Game *game);

void render_game(Game *game);

Scene create_scene();

void generate_shot(Game *game, Vector3 v, Vector3 w);

void clear_paths(Scene *scene);

#endif // GAME_H