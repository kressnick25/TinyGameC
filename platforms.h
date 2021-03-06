#ifndef PLATFORMS_H
#define PLATFORMS_H "platforms.h"

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include "ZDK/cab202_graphics.h"
#include "ZDK/cab202_sprites.h"
#include "ZDK/cab202_timers.h"
#include <unistd.h>
#include "util.h"
#include "images.h"
#include "state.h"

#define PLATFORM_MAX_WIDTH 10
#define PLATFORM_SPACING 9
#define FIRST_PLATFORM_X 1
#define FIRST_PLATFORM_Y 11

typedef enum PLATFORM_TYPE {
    SAFE = 1,
    BAD = 2,
    NONE = 3
} PLATFORM_TYPE;

void set_platform_type(sprite_id platform, PLATFORM_TYPE type);

PLATFORM_TYPE get_platform_type(sprite_id platform);

// Move platforms according to set dx.
// Moves platform to opposite side of screen when it reaches the edge
void platforms_update_position(sprite_id* Platforms, int a_size);


// Checks wich blocks in the top row are safe blocks
// Randomly chooses and returns one of the safe blocks
sprite_id get_safe_block(sprite_id Platforms[]);

// Randomly selects a platform type based on probability
PLATFORM_TYPE rand_platform_type (void);

char* platform_get_bitmap(PLATFORM_TYPE type);

// Returns a sprite_id for a new platform with set input paramenters
sprite_id platforms_setup(int px, int py, int width, double dx, char* bitmap, PLATFORM_TYPE type);

// Create platform co-ordinates, length, width, bitmap, dx and store in array
// a pointer to an array of spride ids
void platforms_create(sprite_id* Platforms);

void platforms_destroy(sprite_id Platforms[], int a_size);

// From Array of platforms, draw platforms on screen
void platforms_draw(sprite_id Platforms[], int a_size);

#endif