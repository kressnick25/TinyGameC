#ifndef COLLISION_H
#define COLLISION_H "collision.h"

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
#include "state.h"

#include "collision.h"

// Checks for player collision with chest, hides chest upon collision
void collision_chest(State* state);

// Checks for collision between the player and each block 'Platforms' array
bool collision_platforms(Playerstate* playerstate, sprite_id Platforms[], int a_size);

// Checks collision between two sprites on a pixel level
bool collision_pixel_level(Sprite *s1, Sprite *s2);

#endif