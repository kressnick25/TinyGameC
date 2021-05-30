#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/resource.h>
#include "ZDK/cab202_graphics.h"
#include "ZDK/cab202_sprites.h"
#include "ZDK/cab202_timers.h"
#include <unistd.h>
#include "state.h"
#include "platforms.h"
#include "images.h"



const InitialState initialstate = {
    .lives = 10,
    .momentum = 0.0,
    .bitmap = 0,
};

// Draws treasure at specific co-ordinates
sprite_id chest_create( void )
{
    int chest_x = screen_width() / 2;
    int chest_y = screen_height() - 3;
    sprite_id chest = sprite_create(chest_x, chest_y, 5, 3, chest_image);
    sprite_turn_to(chest, -0.1, 0);
    return chest;
}


Playerstate* playerstate_create(sprite_id platforms[], double momentum, int bitmap) {
    Playerstate* temp = malloc(sizeof(Playerstate));

    sprite_id safe_block = get_safe_block(platforms);
    temp->player_sprite = sprite_create(safe_block->x, safe_block->y - 3, 6, 3, player_image);
    temp->momentum = momentum;
    temp->old_block = 0;
    temp->bitmap = bitmap;
    temp->dead = false;
    
    return temp;
}

// Draws status display bar at top of screen
// Updates when score, lives and time changes
void scoreboard_draw(Gamestate* gamestate, Scoreboard* scoreboard)
{
    int mx = screen_width() - 50;
    int my = 1;
    // int minutes = (Scoreboard->secondsPast /60) % 60;
    // int seconds = Scoreboard->secondsPast % 60;
    struct rusage r_usage;
    getrusage(RUSAGE_SELF,&r_usage);
     // draw text/content of scoreboard
    draw_line(mx - 2, 0, mx - 2, 3, '|');
    draw_formatted(mx, my, "n9467688 |");
    draw_formatted(mx + 11, my, "Lives: %d |", gamestate->livesRemaining );
    draw_formatted(mx + 23, my, "Score: %d", scoreboard->score);
    // draw_formatted(mx + 34, my,"| %02u:%02u", minutes, seconds); 
    // Print the maximum resident set size used (in kilobytes).
    draw_formatted(mx + 34, my, "%ld kilobytes\n",r_usage.ru_maxrss); 

    draw_line(mx -1, 3, screen_width(), 3, '~');

}


/// PROCESS FUNCTIONS

// Increases the timer displayed in status display
void timer_increase(Scoreboard* scoreboard, double game_start)
{   
    double time_past = get_current_time() - game_start;
    scoreboard->secondsPast = time_past;
}

void setDead(Playerstate* playerstate) {
    playerstate->dead = true;
}

// Called when player collides with forbidden block or moves out of bounds
// Pauses the game momentarily, resets player to safe block in starting row
void die (State* state, sprite_id platforms[]) {
    timer_pause(1000);
    sprite_hide(state->playerstate->player_sprite);
    // destroy_platforms(platforms); TODO renable new platforms
    // create_platforms(platforms); TODO renable new platforms
    //reset player variables
    sprite_turn_to(state->playerstate->player_sprite, 0, 0);
    //choose safe platform to move to
    sprite_id safe_block = get_safe_block(platforms);
    sprite_move_to(state->playerstate->player_sprite, safe_block->x, safe_block->y - 3);
    state->playerstate->player_sprite->dx = 0;
    state->playerstate->player_sprite->dy = 0;
    state->playerstate->momentum = 0;
    sprite_show(state->playerstate->player_sprite);
    state->gamestate->livesRemaining--;
    state->playerstate->dead = false;
    if (state->gamestate->livesRemaining == 0)
    {
        state->gamestate->game_over = true;
    }
}

// Alternates chest bitmap between two images
void animate_chest(Cheststate* cheststate)
{
    if(cheststate->chest_timer != NULL && timer_expired(cheststate->chest_timer))
    {
        if (cheststate->alt_chest)
        {
            cheststate->chest_sprite->bitmap = chest_image_alt;
        }
        else
        {
            cheststate->chest_sprite->bitmap = chest_image;
        }
        cheststate->alt_chest = !cheststate->alt_chest;
    }
}

// Moves chest along bottom of screen
void chest_move(Cheststate* cheststate, int key)
{
    // Uses code from ZDJ Topic 04
    // Toggle chest movement when 't' pressed.
    if (key == 't')
    {
        cheststate->stop_chest = !cheststate->stop_chest;
    }
    if (!cheststate->stop_chest)
    {
        sprite_step(cheststate->chest_sprite);
        animate_chest(cheststate);
    }
    // Change direction when wall hit
    int cx = round(sprite_x(cheststate->chest_sprite));
    double cdx = sprite_dx(cheststate->chest_sprite);
    if ( cx == 0 || cx == screen_width() - sprite_width(cheststate->chest_sprite)) {
        cdx = -cdx;
    }
    if (cdx != sprite_dx(cheststate->chest_sprite)) {
        sprite_back(cheststate->chest_sprite);
        sprite_turn_to(cheststate->chest_sprite, cdx, 0);
    }
}

// Increases score only when player moves to or lands on a new safe block
void score_increase(State* state, int new_block)
{
    if(state->playerstate->old_block != new_block)
    {
        state->scoreboard->score++;
    }
    state->playerstate->old_block = new_block;
}

// Checks collision between two sprites on a pixel level
bool collision_pixel_level( Sprite *s1, Sprite *s2 )
{       // Uses code from AMS wk5.
        // Only check bottom of player model, stops player getting stuck in blocks.
    int y = round(s1->y + 2);
    for (int x = s1->x; x < s1->x + sprite_width(s1); x++){
        // Get relevant values of from each sprite
        int sx1 = x - round(s1->x);
        int sy1 = y - ceil(s1->y);
        int offset1 = sy1 * s1->width + sx1;

        int sx2 = x - round(s2->x);
        int sy2 = y - round(s2->y-1);
        int offset2 = sy2 * s2->width + sx2;
        
        // If opaque at both points, collisio has occured
        if (0 <= sx1 && sx1 < s1->width &&
            0 <= sy2 && sy1 < s1->height &&
            s1->bitmap[offset1] != ' ')
            {
            if (0 <= sx2 && sx2 < s2->width &&
                0 <= sy2 && sy2 < s2->height && 
                s2->bitmap[offset2] != ' ')
                {
                return true;
            }
        }
    }
    return false;
}

// Checks for collision between the player and each block 'Platforms' array
bool collision_platforms(Playerstate* playerstate, sprite_id* Platforms, int a_size) 
{
    bool output = false;
    int c = 0;
    for (int i = 0; i < a_size; i++){
        if(Platforms[i] != NULL)    // Do not check empty platforms
        { 
            bool collide = collision_pixel_level(playerstate->player_sprite, Platforms[i]);
            if (collide)
            {
                if(Platforms[i]->bitmap == badBlock_image){
                    setDead(playerstate);
                    output = true;
                    break;
                }
                else
                {   // Die if any part of current block is off screen
                    if(Platforms[i]->x > screen_width() - Platforms[i]->width ||
                        Platforms[i]->x < 0){
                        setDead(playerstate);
                    }
                    // Update player speed so that player moves with platform on
                    playerstate->player_sprite->dx = sprite_dx(Platforms[i]);
                    output = true;
                    if (c == 0){
                        c = i;
                    }
                }
            }
        } 
    }
    if (output == true){
        // TODO increase score by c
    }
    return output;
}

// Cause the player to die if moved out of bounds to left, right or bottom of screen
bool is_out_of_bounds(Playerstate* playerstate) {
    return playerstate->player_sprite->y >= screen_height() + 6 
            || playerstate->player_sprite->x + sprite_width(playerstate->player_sprite) < 0
            || playerstate->player_sprite->x > screen_width();
}

// Changes the player model/image based on the last key pressed.
// Holds the image for certain time after button stopped pressed to prevent flickering
void animate_player(Playerstate* playerstate, bool on_platform)
{
    if (playerstate->bitmap == 1)
    {
        playerstate->player_sprite->bitmap = right_image;
        playerstate->PlayerStillTimer = create_timer(150);
        playerstate->bitmap = 0;
    }
    else if (playerstate->bitmap == 2)
    {
        playerstate->player_sprite->bitmap = left_image;
        playerstate->PlayerStillTimer = create_timer(150);
        playerstate->bitmap = 0;
    }
    else if (playerstate->bitmap == 3)
    {
        playerstate->player_sprite->bitmap = falling_image;
        playerstate->PlayerStillTimer = create_timer(150);
        playerstate->bitmap = 0;
    }
    // Also reset player momentum if no keys pressed for long enough
    if (playerstate->PlayerStillTimer != NULL)
    {
        if (timer_expired(playerstate->PlayerStillTimer) == true && on_platform)
        {
            playerstate->player_sprite->bitmap = player_image;
            playerstate->momentum = 0;
        }
    }
}

// Moves player according to key pressed
void movement_player(Playerstate* playerstate, int key, bool on_platform)
{
    int visible_width = screen_width() - 1;
    int px = sprite_x(playerstate->player_sprite);
    if (!on_platform) return;
    if ( key == 'd' && px < visible_width - 6)
    {
        sprite_move(playerstate->player_sprite, 1, 0);
        playerstate->bitmap = 1;
        if (playerstate->momentum < 0.4)
        {
            playerstate->momentum += 0.3;
        }        
    }
    else if (key == 'a' && px > 0)
    {
        sprite_move( playerstate->player_sprite, -1, 0);
        playerstate->bitmap = 2;
        if (playerstate->momentum > -0.4)
        {
            playerstate->momentum += -0.3;
        }
    }
    else if (key == 'w')
    {
        playerstate->player_sprite->dy = -2;
        // Jumping with horizontal velocity moves further than
        // falling with horizontal velocity
        if (playerstate->momentum != 0)
        { 
            playerstate->momentum *= 1.5;
        }
    }
}

// Decays momentum to allow for distance control over jumps
void movement_horizontal_apply(Playerstate* playerstate, bool on_platform)
{
    if (playerstate->momentum != 0 && on_platform) {
        if (playerstate->momentum > 0 && playerstate->momentum < 0.5){
            playerstate->momentum -= 0.05;
        }
        else if (playerstate->momentum > -0.5 && playerstate->momentum < 0){
            playerstate->momentum += 0.05;
        }
    }
}


void movement_gravity_apply(Playerstate* playerstate, bool is_colliding)
{
    // When on block, kill velocity
    if (is_colliding)
    {
       playerstate->player_sprite->dy = 0;
   }
    // When jumping, half velocity each step
    else if(playerstate->player_sprite->dy <= -0.01)
    {
        playerstate->player_sprite->dy *= 0.3;
    }
    // When falling (negative velocity) double velocity
    else if (playerstate->player_sprite->dy > 0 && playerstate->player_sprite->dy < 0.8)
    {
        playerstate->player_sprite->dy *= 1.3;
    }
    // When jump peak reached, flip velocity to negative
    else if(playerstate->player_sprite->dy > -0.2 && playerstate->player_sprite->dy < 0)
    {
         playerstate->player_sprite->dy = -playerstate->player_sprite->dy;
    }
    // Else give player falling velocity
    else if(!is_colliding && playerstate->player_sprite->dy == 0.0)
    {
       playerstate->player_sprite->dy = 0.01;
    }
    // Apply accumulated 'momentum' to player speed when off block.
    if(!is_colliding){
       playerstate->player_sprite->dx = playerstate->momentum;
    }
}

// Checks for player collision with chest, hides chest upon collision
void collision_chest(State* state)
{ 
    bool collide = collision_pixel_level(state->playerstate->player_sprite, state->cheststate->chest_sprite);
    if (collide)
    {
        state->gamestate->livesRemaining += 3;
        state->cheststate->hide_chest_timer = create_timer(2000);
        //sprite_hide(chest);
        // move chest off screen, can't collide with player
        // todo destroy sprite

        setDead(state->playerstate);
    }
}


// Game over screen displayed when player loses all lives
void screen_game_over(State* state)
{
    clear_screen();
    int tx = screen_width() / 2 - 5;
    int ty = screen_height() / 2 - 5;
    const int minutes = (state->scoreboard->secondsPast /60) % 60;
    const int seconds = state->scoreboard->secondsPast % 60;
    // Draw text in middle of screen
    draw_formatted(tx, ty, " Game Over");
    draw_formatted(tx, ty+1, " Score: %d", state->scoreboard->score);
    draw_formatted(tx, ty+2, "Time:  %02u:%02u", minutes, seconds);
    draw_formatted(tx - 3, ty+3, "Press 'r' to restart");
    draw_formatted(tx + 5, ty + 4, "or");
    draw_formatted(tx - 2, ty + 5, "Press 'q' to quit");
    show_screen();
    bool yes = true;
    // Allow player to restart game or exit gracefully
    while(yes){
        int key = get_char();
        if (key == 'q'){
            exit(0);
        }
        else if (key == 'r'){
            state->gamestate->game_over = false;
            yes = false;
        }
        show_screen();
    }
}


// Draw all sprites in new positions
void draw_all(State* state, sprite_id platforms[])
{
    sprite_draw(state->cheststate->chest_sprite);
    sprite_draw(state->playerstate->player_sprite);
    platforms_draw(platforms, A_SIZE);
    scoreboard_draw(state->gamestate, state->scoreboard);
}

void setup(State** state, sprite_id platforms[])
{
    platforms_create(platforms);
    *state = state_create(initialstate, platforms);
    draw_all(*state, platforms);
    
}

void cleanup(State* state, sprite_id *platforms) {
    state_destroy(state);
    platforms_destroy(platforms, A_SIZE);
}


//// MAIN
int main( void )
{  
    State *state = NULL;
    sprite_id* platforms = malloc(A_SIZE * sizeof(sprite_id));
    
    while (true){ 
        // Seed rand() so not the same platforms every time
        srand(get_current_time());
        // Get time at start of game for timekeeping
        double game_start = get_current_time();
        setup_screen();
        setup(&state, platforms);
        show_screen();
        while (!state->gamestate->game_over)
        {
            if (state->playerstate->dead) {
                die(state, platforms);
            }
            bool is_colliding = collision_platforms(state->playerstate, platforms, A_SIZE);
            clear_screen();
            collision_chest(state); 
            movement_gravity_apply(state->playerstate, is_colliding);
            int key = get_char();
            movement_player(state->playerstate, key, is_colliding); 
            animate_player(state->playerstate, is_colliding);
            chest_move(state->cheststate, key);
            sprite_step(state->playerstate->player_sprite); 
            platforms_update_position(platforms, A_SIZE);
            if (is_out_of_bounds(state->playerstate)) {
                setDead(state->playerstate);
            }
            draw_all(state, platforms);
            timer_increase(state->scoreboard, game_start);
            show_screen();
            fflush(stdin); //flush buffer.
            timer_pause(10);
        }

        screen_game_over(state);
        cleanup(state, platforms);
    }

    return 0;
}
