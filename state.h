#ifndef STATE_H
#define STATE_H "state.h"

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include "ZDK/cab202_graphics.h"
#include "ZDK/cab202_sprites.h"
#include "ZDK/cab202_timers.h"
#include <unistd.h>
#include "platforms.h"

typedef struct InitialState {
    int lives;
    double momentum;
    int bitmap;
} InitialState;

typedef struct Scoreboard {
    int score;
    int secondsPast;
} Scoreboard;

Scoreboard* scoreboard_create (void);

void scoreboard_destroy(Scoreboard* s);

typedef struct Gamestate {
    bool game_over;
    int livesRemaining;
} Gamestate;

Gamestate* gamestate_create (int lives);

void gamestate_destroy(Gamestate* s);

typedef struct Playerstate {
    sprite_id player_sprite;
    float momentum;
    int old_block;
    int bitmap;
    timer_id PlayerStillTimer;
    bool dead;
} Playerstate;

Playerstate* playerstate_create(sprite_id platforms[], double momentum, int bitmap);

void playerstate_destroy(Playerstate* state);

typedef struct Cheststate {
    sprite_id chest_sprite;
    bool alt_chest;
    bool stop_chest;
    timer_id chest_timer;
    timer_id hide_chest_timer; 
} Cheststate;

sprite_id chest_create(void);

Cheststate* cheststate_create(void);

void cheststate_destroy(Cheststate* state);

typedef struct State {
    Scoreboard* scoreboard;
    Gamestate* gamestate;
    Playerstate* playerstate;
    Cheststate* cheststate;
} State;

State* state_create(InitialState initialState, sprite_id platforms[]);

void state_destroy(State* s);

#endif