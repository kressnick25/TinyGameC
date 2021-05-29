#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include "ZDK/cab202_graphics.h"
#include "ZDK/cab202_sprites.h"
#include "ZDK/cab202_timers.h"
#include <unistd.h>

const int GAMESTATE_INITIAL_LIVES = 3;
const float PLAYERSTATE_INITIAL_MOMENTUM = 0;
const int PLAYERSTATE_INITIAL_BITMAP = 0;

typedef struct scoreboard {
    int score;
    int secondsPast;
} scoreboard;

scoreboard* init_scoreboard (void) {
    scoreboard* temp = malloc(sizeof(scoreboard));
    
    temp->score = 0;
    temp->secondsPast = 0;

    return temp;
}

void destroy_scoreboard(scoreboard* s) {
    free(s);
}

typedef struct gamestate {
    bool game_over;
    int livesRemaining;
} gamestate;

gamestate* init_gamestate (int lives) {
    assert(lives > 0);

    gamestate* temp = malloc(sizeof(gamestate));

    temp->game_over = false;
    temp->livesRemaining = lives;

    return temp;
}

void destroy_gamestate(gamestate* s) {
    free(s);
}

typedef struct playerstate {
    sprite_id player;
    float momentum;
    int old_block;
    int bitmap;
    timer_id PlayerStillTimer;
} playerstate;

sprite_id create_player(void);
playerstate* init_playerstate(double momentum, int bitmap);

void destroy_playerstate(playerstate* state) {
    sprite_destroy(state->player);
    destroy_timer(state->PlayerStillTimer);
    free(state);
}

typedef struct cheststate {
    sprite_id chest;
    bool alt_chest;
    bool stop_chest;
    timer_id chest_timer;
    timer_id hide_chest_timer; 
} cheststate;

sprite_id make_chest(void);

cheststate* init_cheststate(void) {
    cheststate* temp = malloc(sizeof(cheststate));
    
    temp->chest_timer = create_timer(500);
    temp->chest = make_chest();
    temp->alt_chest = false;
    temp->stop_chest = false;

    return temp;
}

void destroy_cheststate(cheststate* state) {
    sprite_destroy(state->chest);
    destroy_timer(state->chest_timer);
    free(state);
}

typedef struct state {
    scoreboard* Scoreboard;
    gamestate* Gamestate;
    playerstate* Playerstate;
    cheststate* Cheststate;
} state;

state* init_state() {
    state* temp = malloc(sizeof(state));

    temp->Scoreboard = init_scoreboard();
    temp->Gamestate = init_gamestate(GAMESTATE_INITIAL_LIVES);
    temp->Playerstate = init_playerstate(PLAYERSTATE_INITIAL_MOMENTUM, PLAYERSTATE_INITIAL_BITMAP);
    temp->Cheststate = init_cheststate();
    
    return temp;
}

void destroy_state(state* s) {
    destroy_scoreboard(s->Scoreboard);
    destroy_gamestate(s->Gamestate);
    destroy_playerstate(s->Playerstate);
    destroy_cheststate(s->Cheststate);
    free(s);
}

state* STATE;

//SPRITES
char *player_image =
    " ____ "
    "{_00_}"
    "8____8";
char * right_image = 
    "  ///]"	
    " //00}"
    "//===]";
char * left_image = 
    "[\\\\\\  "
    "{00\\\\ "
    "[===\\\\\\";
char * falling_image = 
  "  __  "
  " /  \\ "
  "[_00_]";
char *chest_image =
    " ___ "
    "/___\\"
    "[_0_]";
char *chest_image_alt =
    " ___ "
    "/#-#\\"
    "{_$_}";
char *safeBlock_image =
    "=========="
    "==========";
char *badBlock_image =
    "xxxxxxxxxx"
    "xxxxxxxxxx";

sprite_id Platforms[200]; 
int PlatformArrayLength;

// Returns a random double floating point number between two values
double rand_number(double min, double max)
{ 
    double range = (max - min);
    double div = RAND_MAX / range;
    return min + (rand() / div);
}

// Draws treasure at specific co-ordinates
sprite_id make_chest( void )
{
    int chest_x = screen_width() / 2;
    int chest_y = screen_height() - 3;
    sprite_id chest = sprite_create(chest_x, chest_y, 5, 3, chest_image);
    sprite_turn_to(chest, -0.1, 0);
    return chest;
}

// Randomly selects a platform type based on probability
char* choose_platform_type ( void )
{
    int i = rand_number(0,8);
    char* type;
    if (i <= 3){
        type = safeBlock_image;
    }
    else if (i == 4){
        type = badBlock_image;
    }
    else{
        type = NULL;
    }
    return type;
}

// Returns the number of columns for current terminal screen size
int get_num_columns()
{
    int num_columns = (screen_width() - 1) / 10 + 1;
    return num_columns;
}

// Returns the number of rows for the current terminal screen size
int get_num_rows( void )
{
    int height = screen_height() - 7;
    int row_spacing = 9;    
    int num_rows = height / row_spacing;
    return num_rows;
}

// Returns a sprite_id for a new platform with set input paramenters
sprite_id setup_platform(int px, int py, int width, double dx, char* bitmap){
    sprite_id new_platform = sprite_create( px, py, width, 2, bitmap );
    sprite_turn_to(new_platform, dx, 0);
    return new_platform;
}

// Create platform co-ordinates, length, width, bitmap, dx and store in array
void create_platforms( void ) {
    memset(Platforms, 0, sizeof Platforms);
    int num_rows = get_num_rows();
    int row_spacing = 9;
    int initX = 1, initY = 11;
    int deltaY = 0;
    int c = 0;
    int k = 1;
    for (int i = 0; i < num_rows; i++)
    {
        int deltaX = 0;
        // Choose a random platform speed, alternating direction
        k = -k;
        double speed = rand_number(0.01, 0.1);
        double block_speed = speed * k;
        int num_columns = get_num_columns();
        for (int j = 0; j <= num_columns; j++)
        {
            // Choose which type of block to draw
            char *bitmap = choose_platform_type();
            int width = 10;
            // store block information in array.
            if (bitmap != NULL && c < 200)
            {
                // Choose a random width between 5 and 10
                width = rand_number(5, 10);
                // If drawing first or last row, set row speed to 0.
                if (i == 0 || i + 1 == num_rows)
                {
                    block_speed = 0;
                }
                Platforms[c] = setup_platform(initX + deltaX, initY + deltaY, 
                                                width, block_speed, bitmap);
                deltaX += width + 1;
            }
            else{
                // If not drawing a block, add space between blocks.
                deltaX += width + 1;
            }
            c++; 
        }
        deltaY += row_spacing;
    }
    // Set global variable of Array Length.
    PlatformArrayLength = sizeof(Platforms)/sizeof(Platforms[0]);
}

void destroy_platforms(void) {
    for(int i = 0; i < sizeof(Platforms)/sizeof(Platforms[0]); i++) {
        if (Platforms[i] != NULL) {
            free(Platforms[i]);
        }
    }
}

// From Array of platforms, draw platforms on screen
void draw_platforms( void )
{
    for (int j = 0; j < PlatformArrayLength; j++){
        if (Platforms[j] != NULL){ 
            sprite_draw(Platforms[j]);
        }
    }
}

// Checks wich blocks in the top row are safe blocks
// Randomly chooses and returns one of the safe blocks
sprite_id choose_safe_block()
{
    int i = rand_number(0, get_num_columns());
    while(Platforms[i] == NULL || Platforms[i]->bitmap != safeBlock_image)
    {
        i = rand_number(0, get_num_columns());
    }
    return Platforms[i];    
}

// Initial player spawn function
sprite_id create_player( void )
{
    sprite_id safe_block = choose_safe_block();
    return sprite_create(safe_block->x, safe_block->y - 3, 6, 3, player_image);
}

playerstate* init_playerstate(double momentum, int bitmap) {
    playerstate* temp = malloc(sizeof(playerstate));

    temp->player = create_player();
    temp->momentum = momentum;
    temp->old_block = 0;
    temp->bitmap = bitmap;
    
    return temp;
}

// Draws status display bar at top of screen
// Updates when score, lives and time changes
void draw_scoreboard( void )
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
    draw_formatted(mx + 11, my, "Lives: %d |", STATE->Gamestate->livesRemaining );
    draw_formatted(mx + 23, my, "Score: %d", STATE->Scoreboard->score);
    // draw_formatted(mx + 34, my,"| %02u:%02u", minutes, seconds); 
    // Print the maximum resident set size used (in kilobytes).
    draw_formatted(mx + 34, my, "%ld kilobytes\n",r_usage.ru_maxrss); 

    draw_line(mx -1, 3, screen_width(), 3, '~');

}

// Move platforms according to set dx.
// Moves platform to opposite side of screen when it reaches the edge
void auto_move_platforms( void )
{
    for (int i = 0; i < PlatformArrayLength; i ++){
        if (Platforms[i] != NULL){ // Do not attempt to call empty value
            sprite_step(Platforms[i]);
            int px = Platforms[i]->x;
            int py = Platforms[i]->y;
            if (px + 10 < 0){ // Screen LHS
                sprite_move_to(Platforms[i], screen_width() - 1, py);
            }
            else if(px > screen_width()){ // Screen RHS
                sprite_move_to(Platforms[i], 0 - 10, py);
            }
        }
    }
}

/// PROCESS FUNCTIONS

// Increases the timer displayed in status display
void increase_timer(double game_start)
{   
    double time_past = get_current_time() - game_start;
    STATE->Scoreboard->secondsPast = time_past;
}

// Called when player collides with forbidden block or moves out of bounds
// Pauses the game momentarily, resets player to safe block in starting row
void die ( void )
{   
    timer_pause(1000);
    sprite_hide(STATE->Playerstate->player);
    destroy_platforms();
    create_platforms();
    //reset player variables
    sprite_turn_to(STATE->Playerstate->player, 0, 0);
    //choose safe platform to move to
    sprite_id safe_block = choose_safe_block();
    sprite_move_to(STATE->Playerstate->player, safe_block->x, safe_block->y - 3);
    STATE->Playerstate->player->dx = 0;
    STATE->Playerstate->player->dy = 0;
    STATE->Playerstate->momentum = 0;
    sprite_show(STATE->Playerstate->player);
    STATE->Gamestate->livesRemaining--;
    if (STATE->Gamestate->livesRemaining == 0)
    {
        STATE->Gamestate->game_over = true;
    }
}

// Alternates chest bitmap between two images
void animate_chest( void )
{
    if(STATE->Cheststate->chest_timer != NULL && timer_expired(STATE->Cheststate->chest_timer))
    {
        if (STATE->Cheststate->alt_chest)
        {
            STATE->Cheststate->chest->bitmap = chest_image_alt;
        }
        else
        {
            STATE->Cheststate->chest->bitmap = chest_image;
        }
        STATE->Cheststate->alt_chest = !STATE->Cheststate->alt_chest;
    }
}

// Moves chest along bottom of screen
void move_chest( int key)
{
    // Uses code from ZDJ Topic 04
    // Toggle chest movement when 't' pressed.
    if (key == 't')
    {
        STATE->Cheststate->stop_chest = !STATE->Cheststate->stop_chest;
    }
    if (!STATE->Cheststate->stop_chest)
    {
        sprite_step(STATE->Cheststate->chest);
        animate_chest();
    }
    // Change direction when wall hit
    int cx = round( sprite_x( STATE->Cheststate->chest ) );
    double cdx = sprite_dx( STATE->Cheststate->chest );
    if ( cx == 0 || cx == screen_width() - sprite_width( STATE->Cheststate->chest ) ){
        cdx = -cdx;
    }
    if ( cdx != sprite_dx( STATE->Cheststate->chest ) ){
        sprite_back( STATE->Cheststate->chest );
        sprite_turn_to( STATE->Cheststate->chest, cdx, 0 );
    }
}

// Increases score only when player moves to or lands on a new safe block
void increase_score(int new_block)
{
    if(STATE->Playerstate->old_block != new_block)
    {
        STATE->Scoreboard->score++;
    }
    STATE->Playerstate->old_block = new_block;
}

// Checks collision between two sprites on a pixel level
bool pixel_level_collision( Sprite *s1, Sprite *s2 )
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
bool platforms_collide( void ) 
{
    bool output = false;
    int c = 0;
    for (int i = 0; i < PlatformArrayLength; i++){
        if(Platforms[i] != NULL)    // Do not check empty platforms
        { 
            bool collide = pixel_level_collision(STATE->Playerstate->player, Platforms[i]);
            if (collide)
            {
                if(Platforms[i]->bitmap == badBlock_image){
                    die();
                    output = true;
                    break;
                }
                else
                {   // Die if any part of current block is off screen
                    if(Platforms[i]->x > screen_width() - Platforms[i]->width ||
                        Platforms[i]->x < 0){
                        die();
                    }
                    // Update player speed so that player moves with platform on
                    STATE->Playerstate->player->dx = sprite_dx(Platforms[i]);
                    output = true;
                    if (c == 0){
                        c = i;
                    }
                }
            }
        } 
    }
    if (output == true){
        increase_score(c);
    }
    return output;
}

// Cause the player to die if moved out of bounds to left, right or bottom of screen
void check_out_of_bounds( void )
{
    if (STATE->Playerstate->player->y >= screen_height() + 6 || 
        STATE->Playerstate->player->x + sprite_width(STATE->Playerstate->player) < 0 || 
        STATE->Playerstate->player->x > screen_width())
        {
        die();
    }     
}

// Changes the player model/image based on the last key pressed.
// Holds the image for certain time after button stopped pressed to prevent flickering
void animate_player( void )
{
    if (STATE->Playerstate->bitmap == 1)
    {
        STATE->Playerstate->player->bitmap = right_image;
        STATE->Playerstate->PlayerStillTimer = create_timer(150);
        STATE->Playerstate->bitmap = 0;
    }
    else if (STATE->Playerstate->bitmap == 2)
    {
        STATE->Playerstate->player->bitmap = left_image;
        STATE->Playerstate->PlayerStillTimer = create_timer(150);
        STATE->Playerstate->bitmap = 0;
    }
    else if (STATE->Playerstate->bitmap == 3)
    {
        STATE->Playerstate->player->bitmap = falling_image;
        STATE->Playerstate->PlayerStillTimer = create_timer(150);
        STATE->Playerstate->bitmap = 0;
    }
    // Also reset player momentum if no keys pressed for long enough
    if (STATE->Playerstate->PlayerStillTimer != NULL)
    {
        if (timer_expired(STATE->Playerstate->PlayerStillTimer) == true && platforms_collide())
        {
            STATE->Playerstate->player->bitmap = player_image;
            STATE->Playerstate->momentum = 0;
        }
    }
}

// Moves player according to key pressed
void move_player(int key)
{
    int visible_width = screen_width() - 1;
    int px = sprite_x(STATE->Playerstate->player);
    if (!platforms_collide()) return;
    if ( key == 'd' && px < visible_width - 6)
    {
        sprite_move( STATE->Playerstate->player, 1, 0);
        STATE->Playerstate->bitmap = 1;
        if (STATE->Playerstate->momentum < 0.4)
        {
            STATE->Playerstate->momentum += 0.3;
        }        
    }
    else if (key == 'a' && px > 0)
    {
        sprite_move( STATE->Playerstate->player, -1, 0);
        STATE->Playerstate->bitmap = 2;
        if (STATE->Playerstate->momentum > -0.4)
        {
            STATE->Playerstate->momentum += -0.3;
        }
    }
    else if (key == 'w')
    {
        STATE->Playerstate->player->dy = -2;
        // Jumping with horizontal velocity moves further than
        // falling with horizontal velocity
        if (STATE->Playerstate->momentum != 0)
        { 
            STATE->Playerstate->momentum *= 1.5;
        }
    }
}

// Decays momentum to allow for distance control over jumps
void horizonal_movement( void )
{
    if (STATE->Playerstate->momentum != 0 && platforms_collide()){
        if (STATE->Playerstate->momentum > 0 && STATE->Playerstate->momentum < 0.5){
            STATE->Playerstate->momentum -= 0.05;
        }
        else if (STATE->Playerstate->momentum > -0.5 && STATE->Playerstate->momentum < 0){
            STATE->Playerstate->momentum += 0.05;
        }
    }
}


void gravity( void )
{
    bool is_colliding = platforms_collide();
    // When on block, kill velocity
    if (is_colliding)
    {
       STATE->Playerstate->player->dy = 0;
   }
    // When jumping, half velocity each step
    else if(STATE->Playerstate->player->dy <= -0.01)
    {
        STATE->Playerstate->player->dy *= 0.3;
    }
    // When falling (negative velocity) double velocity
    else if (STATE->Playerstate->player->dy > 0 && STATE->Playerstate->player->dy < 0.8)
    {
        STATE->Playerstate->player->dy *= 1.3;
    }
    // When jump peak reached, flip velocity to negative
    else if(STATE->Playerstate->player->dy > -0.2 && STATE->Playerstate->player->dy < 0)
    {
         STATE->Playerstate->player->dy = -STATE->Playerstate->player->dy;
    }
    // Else give player falling velocity
    else if(!is_colliding && STATE->Playerstate->player->dy == 0.0)
    {
       STATE->Playerstate->player->dy = 0.01;
    }
    // Apply accumulated 'momentum' to player speed when off block.
    if(!is_colliding){
       STATE->Playerstate->player->dx = STATE->Playerstate->momentum;
    }
}

// Checks for player collision with chest, hides chest upon collision
void chest_collide( void )
{ 
    bool collide = pixel_level_collision(STATE->Playerstate->player, STATE->Cheststate->chest);
    if (collide)
    {
        STATE->Gamestate->livesRemaining += 3;
        STATE->Cheststate->hide_chest_timer = create_timer(2000);
        //sprite_hide(chest);
        // move chest off screen, can't collide with player
        // todo destroy sprite

        die();
    }
}


// Game over screen displayed when player loses all lives
void game_over_screen( void )
{
    clear_screen();
    int tx = screen_width() / 2 - 5;
    int ty = screen_height() / 2 - 5;
    const int minutes = (STATE->Scoreboard->secondsPast /60) % 60;
    const int seconds = STATE->Scoreboard->secondsPast % 60;
    // Draw text in middle of screen
    draw_formatted(tx, ty, " Game Over");
    draw_formatted(tx, ty+1, " Score: %d", STATE->Scoreboard->score);
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
            STATE->Gamestate->game_over = false;
            yes = false;
        }
        show_screen();
    }
}


// Draw all sprites in new positions
void draw_all( void )
{
    sprite_draw(STATE->Cheststate->chest);
    sprite_draw(STATE->Playerstate->player);
    draw_platforms();
    draw_scoreboard();
}

//// SETUP
void setup(void)
{
    // setup initial screen here
    create_platforms();
    STATE = init_state();
    draw_all();
    
}

void cleanup(void) {
    destroy_state(STATE);
    destroy_platforms();
}

//// MAIN
int main( void )
{  
    while (true){ 
        // Seed rand() so not the same platforms every time
        srand(get_current_time());
        // Get time at start of game for timekeeping
        double game_start = get_current_time();
        setup_screen();
        setup();
        show_screen();
        while (!STATE->Gamestate->game_over)
        {
            clear_screen();
            chest_collide(); 
            gravity();
            int key = get_char();
            move_player(key); 
            animate_player();
            move_chest(key);
            sprite_step(STATE->Playerstate->player); 
            auto_move_platforms();
            check_out_of_bounds();
            draw_all();
            increase_timer(game_start);
            show_screen();
            fflush(stdin); //flush buffer.
            timer_pause(10);
        }

        game_over_screen();
        cleanup();
    }

    return 0;
}
