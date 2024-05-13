#ifndef PLAYER_H
#define PLAYER_H

#include <raylib.h>
#include <raymath.h>
#include "game.h"

typedef enum
{
    HUMAN,
    AI
} PlayerType;

struct Game;
struct Ball;

typedef void (*PlayerPotBallFunction)(struct Game *game, struct Ball *ball);

typedef struct
{
    char *name;
    char *description;
    PlayerPotBallFunction pot_ball;
} PlayerModule;

typedef struct Player
{
    PlayerType type;
    struct Game *game;
    PlayerModule module;
} Player;

#endif // PLAYER_H