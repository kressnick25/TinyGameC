#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

// function used to safely check timers and prevent core dump.
void show_timer(timer_id timer, Sprite *sprite) {
    if (timer != NULL) {
        if (timer_expired(timer)) {
            sprite_show(sprite);
        }
    }
}

typedef struct platforms {
    int x;
    int y;
    int type;
} platforms;

typedef struct game_state {
    bool game_over;
    double game_start;
    int lives_remaining;
    int score;
    int seconds_past;
    int block_width;
    int current_block;
    int prev_block;
    int bitmap;
} game_state;

enum PLATFORM_TYPE {
    PLATFORM = 0,
    BAD_PLATFORM = 1,
    EMPTY_PLATFORM = 2,
} PLATFORM_TYPE;

platforms platformArray[160];
game_state state;
// SPRITES
sprite_id player;
sprite_id chest;

char *player_image =
    " ____ "
    "{_00_}"
    "8____8";
char *right_image =
    "  ///]"
    " //00}"
    "//===]";
char *left_image =
    "[\\\\\\  "
    "{00\\\\ "
    "[===\\\\\\";
char *falling_image =
    "  __  "
    " /  \\ "
    "[_00_]";
char *chest_image =
    " ___ "
    "/___\\"
    "[_0_]";
// TODO better alt sprite art
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

// player initial position

////FUNCTIONS
// Helper Functions

/// SETUP FUNCTIONS
void spawn_player(void) { player = sprite_create(2, 0, 6, 3, player_image); }
void make_chest(void) {
    int chest_x = screen_width() / 2;
    int chest_y = screen_height() - 3;
    chest = sprite_create(chest_x, chest_y, 5, 3, chest_image);
    sprite_turn_to(chest, -0.1, 0);
}
// draws a rectangle with defined character/s

int choose_platform_type(void) {
    int i = rand() % 7;
    int type;
    if (i <= 3) {
        type = PLATFORM;
    } else if (i == 4) {
        type = BAD_PLATFORM;
    } else {
        type = EMPTY_PLATFORM;
    }
    return type;
}
void create_platforms() {
    int column_spacing = 1;
    // total number of columns on screen
    int num_columns =
        (screen_width() - 1) / (state.block_width + column_spacing);
    int platformsPerColumn =
        (screen_height() / (3 + sprite_height(player))) - 1;
    int init_px = 1;  // first platform x position
    int init_py = 8;  // first platform y position

    // draw platforms, iterate through each column
    int deltaX = 0;
    int c = 1;
    // make first platform
    platformArray[0].x = 1;
    platformArray[0].y = 3;
    platformArray[0].type = 0;

    for (int i = 0; i < num_columns; i++) {
        int deltaY = 0;  // counter vertical change per block
        for (int j = 0; j < platformsPerColumn; j++) {
            // choose what kind of block to draw.
            int type = choose_platform_type();  // random integer between 0 and
                                                // 6

            platformArray[c].x = init_px + deltaX;
            platformArray[c].y = init_py + deltaY;
            platformArray[c].type = type;
            // TODO platform spacing
            deltaY += 3 + sprite_height(player);  // add vertical height of
                                                  // block plus hero_height + 2
            c++;
        }
        deltaX += state.block_width + 1;
    }
}

void draw_platforms(void) {
    int array_values = 0;
    for (int j = 0; j < ARRAY_SIZE(platformArray); j++) {
        if (platformArray[j].x != 0) {
            array_values++;
        }
    }

    for (int i = 0; i < ARRAY_SIZE(platformArray); i++) {
        int type = platformArray[i].type;
        int px = platformArray[i].x;
        int py = platformArray[i].y;
        if (type == PLATFORM) {
            sprite_id block =
                sprite_create(px, py, state.block_width, 2, safeBlock_image);
            sprite_draw(block);
        } else if (type == BAD_PLATFORM) {
            sprite_id block =
                sprite_create(px, py, state.block_width, 2, badBlock_image);
            sprite_draw(block);
        }
        // DEBUGGING uncomment to show empty platforms
        /**else if (type == 2){
            sprite_id block = sprite_create( platformArray[i].x,
        platformArray[i].y, block_width, 2, testBlock_image);
            sprite_draw(block);
        }**/
    }
}

void draw_scoreboard(void) {
    int mx = screen_width() / 2;
    int my = 1;
    int minutes = (state.seconds_past / 60) % 60;
    int seconds = state.seconds_past % 60;

    // draw text/content of scoreboard
    draw_line(mx - 2, 0, mx - 2, 2, '|');
    draw_formatted(mx, my, "n9467688 |");
    draw_formatted(mx + 11, my, "Lives: %d |", state.lives_remaining);
    draw_formatted(mx + 23, my, "Score: %d", state.score);
    draw_formatted(mx + 34, my, "| %02u:%02u", minutes, seconds);
    draw_line(mx - 1, 2, screen_width(), 2, '~');
}

/// PROCESS FUNCTIONS
void die(void) {
    sprite_destroy(player);
    spawn_player();
    state.lives_remaining--;
    if (state.lives_remaining == 0) {
        state.game_over = true;
    }
}

bool alt_chest = false;
bool stop_chest = false;
void move_chest(int key) {
    // using code from topic 4 zdj,
    if (key == 't') {
        stop_chest = !stop_chest;
    }
    if (!stop_chest) {
        sprite_step(chest);
        // altnernate chest image to animate chest
        // TODO slow down animation.
        if (alt_chest) {
            chest->bitmap = chest_image_alt;
        } else {
            chest->bitmap = chest_image;
        }
        alt_chest = !alt_chest;
    }
    int cx = round(sprite_x(chest));
    double cdx = sprite_dx(chest);
    if (cx == 0 || cx == screen_width() - sprite_width(chest)) {
        cdx = -cdx;
    }
    if (cdx != sprite_dx(chest)) {
        sprite_back(chest);
        sprite_turn_to(chest, cdx, 0);
    }
}

void update_current_block(int current_idx) {
    state.prev_block = state.current_block;
    state.current_block = current_idx;
}

/* Get the current position var of a sprit
 *  Returns:    0 == safe block
 *              1 == illegal block
 *              2 == not on block
 */
int get_current_position(sprite_id sprite) {
    int sprite_Y_adjusted = sprite_y(sprite) + sprite_height(sprite);
    int return_val = 2;
    int i;
    for (i = 0; i < ARRAY_SIZE(platformArray); i++) {
        int platform_x = platformArray[i].x;
        int platform_y = platformArray[i].y;
        int ptype = platformArray[i].type;

        // On bad block
        if (platform_y == sprite_Y_adjusted &&
            sprite->x + sprite_width(sprite) >= platform_x &&
            sprite->x + sprite_width(sprite) <=
                platform_x + state.block_width &&
            ptype == BAD_PLATFORM) {
            return_val = 1;
            break;
        }
        // On bad block
        else if (platform_y == sprite_Y_adjusted && sprite->x >= platform_x &&
                 sprite->x <= platform_x + state.block_width &&
                 ptype == BAD_PLATFORM) {
            return_val = 1;
            break;
        }
        // Sprite is off screen
        if (sprite_Y_adjusted == screen_height() + 6 || sprite_y(sprite) < 0) {
            return_val = 1;
            break;
        }
        // Sprite is on regular block
        if (platform_y == sprite_Y_adjusted && sprite->x > platform_x &&
            sprite->x <= platform_x + state.block_width &&
            ptype != EMPTY_PLATFORM) {
            return_val = 0;
            break;
        } else if (platform_y == sprite_Y_adjusted &&
                   sprite->x + sprite_width(sprite) >= platform_x &&
                   sprite->x + sprite_width(sprite) <=
                       platform_x + state.block_width &&
                   ptype != EMPTY_PLATFORM) {
            return_val = 0;
            break;
        }
    }
    update_current_block(i);

    return return_val;
}

bool collision() {
    int i = get_current_position(player);
    if (i == 1) {
        die();
        return false;
    }
    if (i == 0) {
        return true;
    } else {
        return false;
    }
}

void increase_score() {
    // Don't increase score until player moves to new block
    if (collision() && state.current_block != state.prev_block) {
        state.score += 10;
    }
}

timer_id still;
void animate_player() {
    if (still != NULL) {
        if (timer_expired(still)) {
            player->bitmap = player_image;
        }
    }
    if (state.bitmap == 1) {
        player->bitmap = right_image;
        if (still != NULL) {
            destroy_timer(still);
        }
        still = create_timer(100);
    } else if (state.bitmap == 2) {
        player->bitmap = left_image;
        if (still != NULL) {
            destroy_timer(still);
        }
        still = create_timer(100);
    } else if (state.bitmap == 0) {
        player->bitmap = player_image;
    }
}
void move_player(int key) {
    int visible_width = screen_width() - 1;
    // int visible_height = screen_width()- 1;
    int px = sprite_x(player);
    // int py = sprite_y(player);
    if (key == 'd' && px < visible_width - 6 && collision()) {
        sprite_move(player, 1, 0);
        state.bitmap = 1;
    } else if (key == 'a' && px > 0 && collision()) {
        sprite_move(player, -1, 0);
        state.bitmap = 2;
    } else if (key == 'z') {
        die();
    }
    animate_player();
}

void apply_gravity() {
    bool collide = collision();
    if (!collide) {
        sprite_move(player, 0, 0.2);
    }
}

timer_id hide_chest;
// uses code from my own ams_wk5 task 3 solution
void chest_collide() {
    int px = sprite_x(player);
    int py = sprite_y(player) + sprite_height(player);
    int pw = sprite_width(player) + sprite_x(player);

    int cx = sprite_x(chest);
    int cw = sprite_width(chest) + sprite_x(chest);
    int h = screen_height();

    if (py >= h && py <= h + 3 && px >= cx && px <= cw) {
        state.lives_remaining += 3;
        hide_chest = create_timer(2000);
        sprite_hide(chest);
        die();
    } else if (py >= h && py <= h + 3 && pw >= cx && pw <= cw) {
        state.lives_remaining += 3;
        hide_chest = create_timer(2000);
        sprite_hide(chest);
        die();
    }
    show_timer(hide_chest, chest);
}

void draw_game_over_screen() {
    clear_screen();
    int tx = screen_width() / 2 - 5;
    int ty = screen_height() / 2 - 3;
    const int minutes = (state.seconds_past / 60) % 60;
    const int seconds = state.seconds_past % 60;
    draw_formatted(tx, ty, " game over");
    draw_formatted(tx, ty + 1, " Score: %d", state.score);
    draw_formatted(tx, ty + 2, "Time:  %02u:%02u", minutes, seconds);
    draw_formatted(tx, ty + 3, "press r to restart or");
    draw_formatted(tx, ty + 4, "press q to quit");
    show_screen();

    // Show screen unil user quit or restart
    while (true) {
        int key = get_char();
        if (key == 'q') {
            exit(0);
        } else if (key == 'r') {
            state.game_over = false;
            break;
        }
        show_screen();
    }
}

void draw_all(void) {
    clear_screen();
    sprite_draw(chest);
    sprite_draw(player);
    draw_platforms();
    draw_scoreboard();
    show_screen();
}

//// PROCESS
void process(void) {
    int key = get_char();
    move_player(key);
    move_chest(key);
    increase_score();
    apply_gravity();
    chest_collide();
    draw_all();
}

int main(void) {
    // SETUP

    // init game state
    state.game_over = false;
    // Scoreboard Variables init
    state.lives_remaining = 10;
    state.score = 0;
    // state.seconds_past;
    state.block_width = 10;
    state.game_start = get_current_time();
    state.bitmap = 0;  // 0 == still, 1 = right, 2 = left, 3 = falling.

    // seed rand() so no the same platforms every time.
    srand(state.game_start);
    setup_screen();  // ZDK fn

    // setup initial screen here
    state.lives_remaining = 10;
    state.score = 0;
    make_chest();
    spawn_player();
    create_platforms(platformArray);
    draw_all();
    show_screen();

    // GAME LOOP
    while (!state.game_over) {
        process();
        // increase timer
        double time_past = get_current_time() - state.game_start;
        state.seconds_past = time_past;
        show_screen();
        timer_pause(10);
    }
    draw_game_over_screen();
    main();

    return 0;
}

// USE DRAW_FORMATTED
// TO CHANGE ANIMATION -- change sprite[spritename]->bitmap.
