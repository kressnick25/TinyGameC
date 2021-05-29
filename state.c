#include "state.h"

Scoreboard* scoreboard_create (void) {
    Scoreboard* temp = malloc(sizeof(Scoreboard));
    
    temp->score = 0;
    temp->secondsPast = 0;

    return temp;
}

void scoreboard_destroy(Scoreboard* s) {
    free(s);
}

Gamestate* gamestate_create (int lives) {
    assert(lives > 0);

    Gamestate* temp = malloc(sizeof(Gamestate));

    temp->game_over = false;
    temp->livesRemaining = lives;

    return temp;
}

void gamestate_destroy(Gamestate* s) {
    free(s);
}
void playerstate_destroy(Playerstate* state) {
    sprite_destroy(state->player_sprite);
    destroy_timer(state->PlayerStillTimer);
    free(state);
}

Cheststate* cheststate_create(void) {
    Cheststate* temp = malloc(sizeof(Cheststate));
    
    temp->chest_timer = create_timer(500);
    temp->chest_sprite = chest_create();
    temp->alt_chest = false;
    temp->stop_chest = false;

    return temp;
}

void cheststate_destroy(Cheststate* state) {
    sprite_destroy(state->chest_sprite);
    destroy_timer(state->chest_timer);
    free(state);
}

State* state_create(InitialState initialState) {
    State* temp = malloc(sizeof(State));
    
    temp->scoreboard = scoreboard_create();
    temp->gamestate = gamestate_create(initialState.lives);
    temp->playerstate = playerstate_create(initialState.momentum, initialState.bitmap);
    temp->cheststate = cheststate_create();
    
    return temp;
}

void state_destroy(State* s) {
    scoreboard_destroy(s->scoreboard);
    gamestate_destroy(s->gamestate);
    playerstate_destroy(s->playerstate);
    cheststate_destroy(s->cheststate);
    free(s);
}