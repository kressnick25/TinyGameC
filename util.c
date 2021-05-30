#include "util.h"

// Returns a random double floating point number between two values
double rand_number(double min, double max)
{ 
    double range = (max - min);
    double div = RAND_MAX / range;
    return min + (rand() / div);
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

void setDead(Playerstate* playerstate) {
    playerstate->dead = true;
}