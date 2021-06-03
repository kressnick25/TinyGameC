#ifndef UTIL_H
#define UTIL_H "util.h"

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include "ZDK/cab202_graphics.h"
#include "ZDK/cab202_sprites.h"
#include "ZDK/cab202_timers.h"
#include <unistd.h>
#include "state.h"

#define A_SIZE 200
#define FRAME_TIME 16.6

typedef struct Playerstate Playerstate;

// Returns a random double floating point number between two values
double rand_number(double min, double max);

// Returns the number of columns for current terminal screen size
int get_num_columns();

// Returns the number of rows for the current terminal screen size
int get_num_rows( void );

void setDead(Playerstate *playerstate);

#endif