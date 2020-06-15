#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>
#include <unistd.h>

//function used to safely check timers and prevent core dump.
void show_timer(timer_id timer, Sprite *sprite){
    if (timer != NULL){
        if (timer_expired(timer)){
            sprite_show(sprite);
        }
    }
}

////GLOBALS
//SPRITES
sprite_id player;
sprite_id chest;

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
//TODO better alt sprite art
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
char *testBlock_image =
    "9999999999"
    "9999999999";

// game state
bool game_over = false;
// Scoreboard Variables
int livesRemaining = 10;
int score = 0;
int secondsPast;
int block_width = 10;
//player initial position



////FUNCTIONS
///SETUP FUNCTIONS
void spawn_player(){
    player = sprite_create( 2, 0, 6, 3, player_image );
}
void make_chest( void ){
    int chest_x = screen_width() / 2;
    int chest_y = screen_height() - 3;
    chest = sprite_create(chest_x, chest_y, 5, 3, chest_image);
    sprite_turn_to(chest, -0.1, 0);
}
// draws a rectangle with defined character/s

typedef struct platforms{
    int x;
    int y;
    int type;
} platforms;

platforms platformArray[160]; 

int choose_platform_type (void){
    int i = rand() % 7;
    int type;
    if (i <= 3){
        type = 0;
    }
    else if (i == 4){
        type = 1;
    }
    else{
        type = 2;
    }
    return type;
}
void create_platforms() {
    int column_spacing = 1;
    // total number of columns on screen
    int num_columns = (screen_width() -1) / (block_width + column_spacing);
    int platformsPerColumn = (screen_height() / (3 + sprite_height(player))) - 1;
    int init_px = 1; //first platform x position  
    //TODO plaform spacing
    int init_py = 8; //first platform y position

   //draw platforms, iterate through each column
    int deltaX = 0;
    int c = 1;
    // make first platform
    platformArray[0].x = 1;
    platformArray[0].y = 3;
    platformArray[0].type = 0;

    for (int i = 0; i < num_columns; i++){
        int deltaY = 0; // counter vertical change per block
        for (int j = 0; j < platformsPerColumn; j++){
            //choose what kind of block to draw.
            int type = choose_platform_type(); //random integer between 0 and 6

            platformArray[c].x = init_px + deltaX;
            platformArray[c].y = init_py + deltaY;
            platformArray[c].type = type;       
            //TODO platform spacing
            deltaY += 3 + sprite_height(player); // add vertical height of block plus hero_height + 2
            c++;
        }
        deltaX += block_width + 1;
    }
}

void draw_platforms(){

    int array_length = sizeof(platformArray) / sizeof(platformArray[0]);
    int array_values = 0;
    for (int j = 0; j < array_length; j++){
        if (platformArray[j].x != 0){
            array_values++;
        }
    }

    for (int i = 0; i < array_values; i ++){
        int type = platformArray[i].type;
        int px = platformArray[i].x;
        int py = platformArray[i].y;
        if (type == 0){
            sprite_id block = sprite_create( px, py, block_width, 2, safeBlock_image);
            sprite_draw(block);
        }
        else if (type == 1){
            sprite_id block = sprite_create( px, py, block_width, 2, badBlock_image);
            sprite_draw(block);
        }
        //DEBUGGING uncomment to show empty platforms
        /**else if (type == 2){
            sprite_id block = sprite_create( platformArray[i].x, platformArray[i].y, block_width, 2, testBlock_image);
            sprite_draw(block);
        }**/
    }
}

//TODO fix collision, atm only one side

void draw_scoreboard( void )
{
    int mx = screen_width() / 2;
    int my = 1;
    int minutes = (secondsPast /60) % 60;
    int seconds = secondsPast % 60;
     // draw text/content of scoreboard
    draw_line(mx - 2, 0, mx - 2, 2, '|');
    draw_formatted(mx, my, "n9467688 |");
    draw_formatted(mx + 11, my, "Lives: %d |", livesRemaining );
    draw_formatted(mx + 23, my, "Score: %d", score);
    draw_formatted(mx + 34, my,"| %02u:%02u", minutes, seconds); 
    draw_line(mx -1, 2, screen_width(), 2, '~');

}
///PROCESS FUNCTIONS
void increase_timer(double game_start)
{   
    double time_past = get_current_time() - game_start;
    secondsPast = time_past;
}

void die (){   
    sprite_destroy(player);
    spawn_player();
    livesRemaining--;
    if (livesRemaining == 0){
        game_over = true;
    }

}

bool alt_chest = false;
bool stop_chest = false;
void move_chest( int key){
    //using code from topic 4 zdj,
    if (key == 't'){
        stop_chest = !stop_chest;
    }
    if (!stop_chest){
        sprite_step(chest);
        // altnernate chest image to animate chest
        //TODO slow down animation.
        if (alt_chest){
            chest->bitmap = chest_image_alt;
        }
        else{
            chest->bitmap = chest_image;
        }
        alt_chest = !alt_chest;
    }
    int cx = round( sprite_x( chest ) );
    double cdx = sprite_dx( chest );
    if ( cx == 0 || cx == screen_width() - sprite_width( chest ) ){
        cdx = -cdx;
    }
    if ( cdx != sprite_dx( chest ) ){
        sprite_back( chest );
        sprite_turn_to( chest, cdx, 0 );
    }
}
int get_current_position ( void ){ //TODO split into seperate functions ie shorten
            //TODO return 0 if death, 1 on safe block, 2 if not on block;
    int spritey = sprite_y(player) + sprite_height(player);
    int spritex = sprite_x(player);
    int array_length = sizeof(platformArray) / sizeof(platformArray[0]);
    for (int i = 0; i < array_length; i++){
        int platformx = platformArray[i].x;
        int platformy = platformArray[i].y;
        int ptype = platformArray[i].type;

        if (platformy == spritey && spritex + sprite_width(player) >= platformx &&
            spritex + sprite_width(player) <= platformx + block_width && ptype == 1){
            return 0;
        }
        else if ( platformy == spritey && spritex >= platformx 
                    && spritex <= platformx + block_width && ptype == 1){
            return 0;
        }
        else if (spritey == screen_height()+6){
            return 0;
        }        
        else if ( platformy == spritey && spritex > platformx 
                    && spritex <= platformx + block_width &&
                  ptype != 2){
            return 1;
        }
        else if (platformy == spritey && spritex + sprite_width(player) >= platformx &&
                 spritex + sprite_width(player) <= platformx + block_width &&
                 ptype != 2){
            return 1;
        }
    }
    return 2;
}

bool collision() {
    int i = get_current_position();
    if (i == 0){
        die();
    }
    if (i == 1){
        return true;
    }
    else {
        return false;
    }
}

void increase_score(){
    bool i = collision();
    if (i){
        score += 10;
    }
}
int bitmap = 0; // 0 == still, 1 = right, 2 = left, 3 = falling.
timer_id still;
void animate_player(){
    if (still != NULL){
        if (timer_expired(still)){
            player->bitmap = player_image;
        }
    }
    if (bitmap == 1){
        player->bitmap = right_image;
        if (still != NULL){
            destroy_timer(still);
        }
        still = create_timer(100);
    }
    else if (bitmap == 2){
        player->bitmap = left_image;
        if (still != NULL){
            destroy_timer(still);
        }
        still = create_timer(100);
    }
    else if (bitmap == 0) {
        player->bitmap = player_image;
    }
}
void move_player(int key){
    int visible_width = screen_width() - 1;
    //int visible_height = screen_width()- 1;
    int px = sprite_x(player);
    //int py = sprite_y(player);
    if ( key == 'd' && px < visible_width - 6 && collision()){
        sprite_move( player, 1, 0);
        bitmap = 1;        
    }
    else if (key == 'a' && px > 0 && collision()){
        sprite_move( player, -1, 0);
        bitmap = 2;
    }
    else if (key == 'z'){
        die();
    }
    animate_player();
    
}

void gravity(){
    bool collide = collision();
    if (!collide){
        sprite_move(player, 0, 0.2);
    }
}



timer_id hide_chest;
//uses code from my own ams_wk5 task 3 solution
void chest_collide(){
    int px = sprite_x(player);
    int py = sprite_y(player) + sprite_height(player);
    int pw = sprite_width(player) + sprite_x(player);

    int cx = sprite_x(chest);
    int cw = sprite_width(chest) + sprite_x(chest);
    int h = screen_height();

    if (py >= h && py <= h + 3 && px >= cx && px <= cw){
        livesRemaining += 3;
        hide_chest = create_timer(2000);
        sprite_hide(chest);
        die();
    }
    else if (py >= h && py <= h + 3 && pw >= cx 
                && pw <= cw){
        livesRemaining += 3;
        hide_chest = create_timer(2000);
        sprite_hide(chest);
        die();
    }
    show_timer(hide_chest, chest);
}



void game_over_screen(){
    clear_screen();
    int tx = screen_width() / 2 - 5;
    int ty = screen_height() / 2 - 3;
    const int minutes = (secondsPast /60) % 60;
    const int seconds = secondsPast % 60;
    draw_formatted(tx, ty, " game over");
    draw_formatted(tx, ty+1, " Score: %d", score);
    draw_formatted(tx, ty+2, "Time:  %02u:%02u", minutes, seconds);
    draw_formatted(tx, ty+3, "press r to restart or");
    draw_formatted(tx, ty+4, "press q to quit");
    show_screen();
    bool yes = true;
    while(yes){
        int key = get_char();
        if (key == 'q'){
            exit(0);
        }
        else if (key == 'r'){
            game_over = false;
            yes = false;

        }
        show_screen();
    }
}


void draw_all( void ){
    clear_screen();
    sprite_draw(chest);
    sprite_draw(player);
    draw_platforms();
    draw_scoreboard();
    show_screen();
}

//// SETUP
void setup(void)
{
    // setup initial screen here
    livesRemaining = 10;
    score = 0;
    make_chest();
    spawn_player();
    platforms platformArray[160];
    create_platforms(platformArray);
    draw_all();
    
}

//// PROCESS
void process( void )
{   int key = get_char();
    move_player(key);
    move_chest(key);
    increase_score();
    gravity();
    chest_collide();
    draw_all();
}


//// MAIN
int main( void )
{   
    srand(get_current_time()); //seed rand() so no the same platforms every time.
    double game_start = get_current_time(); 
    setup_screen();
    setup();
    show_screen();

    while (!game_over)
    {
        process();
        increase_timer(game_start);
        show_screen();
        timer_pause(10);
    }
    game_over_screen();
    main();


    return 0;
}

// USE DRAW_FORMATTED
//TO CHANGE ANIMATION -- change sprite[spritename]->bitmap.
