#include "platforms.h"

void set_platform_type(sprite_id platform, PLATFORM_TYPE type) {
    platform->cookie = (int*) type;
}


PLATFORM_TYPE get_platform_type(sprite_id platform) {
    if (platform->cookie == NULL) {
        return NONE;
    }
    return (PLATFORM_TYPE)(int*)(platform->cookie);
}

void platforms_update_position(sprite_id* Platforms, int a_size)
{
    for (int i = 0; i < a_size; i ++){
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

sprite_id get_safe_block(sprite_id Platforms[])
{
    int i = rand_number(0, get_num_columns());
    while(Platforms[i] == NULL || get_platform_type(Platforms[i]) != SAFE)
    {
        i = rand_number(0, get_num_columns());
    }
    return Platforms[i];    
}

PLATFORM_TYPE rand_platform_type ( void )
{
    int i = rand_number(0,8);
    PLATFORM_TYPE type;
    if (i <= 3){
        type = SAFE;
    }
    else if (i == 4){
        type = BAD;
    }
    else{
        type = NONE;
    }
    return type;
}

char* platform_get_bitmap(PLATFORM_TYPE type) {
    switch (type){
        case SAFE: return safeBlock_image; break;
        case BAD: return badBlock_image; break;
        case NONE: return NULL; break;
        default: return NULL;
    }
}

sprite_id platforms_setup(int px, int py, int width, double dx, char* bitmap, PLATFORM_TYPE type){
    sprite_id new_platform = sprite_create( px, py, width, 2, bitmap );
    set_platform_type(new_platform, type);
    sprite_turn_to(new_platform, dx, 0);
    return new_platform;
}

void platforms_create(sprite_id* Platforms) {
    memset(Platforms, 0, 200 * sizeof(sprite_id));
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
            PLATFORM_TYPE type = rand_platform_type();
            int width = 10;
            // store block information in array.
            if (type != NONE && c < A_SIZE)
            {
                // Choose a random width between 5 and 10
                width = rand_number(5, 10);
                // If drawing first or last row, set row speed to 0.
                if (i == 0 || i + 1 == num_rows)
                {
                    block_speed = 0;
                }
                Platforms[c] = platforms_setup(initX + deltaX, initY + deltaY, 
                                                width, block_speed, platform_get_bitmap(type), type);
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
}

void platforms_destroy(sprite_id Platforms[], int a_size) {
    for(int i = 0; i < a_size; i++) {
        if (Platforms[i] != NULL) {
            free(Platforms[i]);
        }
    }
}

void platforms_draw(sprite_id Platforms[], int a_size)
{
    for (int j = 0; j < a_size; j++){
        if (Platforms[j] != NULL){ 
            sprite_draw(Platforms[j]);
        }
    }
}