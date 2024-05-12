#include <raylib.h>

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
    Player *player;
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
    SELECT,
    GAMEPLAY,
    GAME_OVER
} GameScreen;

typedef struct
{
    int num_shots;
    int num_pots;
    int num_fouls;
} Stats;

typedef struct
{
    void (*update)(struct Game *game);
    void (*render)(struct Game *game);
} Screen;

typedef struct Game
{
    GameScreen screen;

    bool playing;

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

    Stats p1_stats;
    Stats p2_stats;

    PlayerPotBallFunction *pot_ball_functions;
    int num_pot_ball_functions;

} Game;