#include "collision.h"

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

bool is_offscreen(double x_pos, int sprite_width) {
    return x_pos > screen_width() - sprite_width || x_pos < 0;
}

bool collision_platforms(Playerstate* playerstate, sprite_id Platforms[], int a_size) {
    bool result = false;
    for (int i = 0; i < a_size; i++) {
        sprite_id plt = Platforms[i];
        if (plt == NULL) continue;
        result = collision_pixel_level(playerstate->player_sprite, plt);
        if (result) {
            if(get_platform_type(plt) == BAD) {
                setDead(playerstate);
            }
            else {   
                if(is_offscreen(plt->x, plt->width)) {
                    setDead(playerstate);
                }
                // Update player speed so that player moves with platform on
                playerstate->player_sprite->dx = sprite_dx(plt);
            }
            break;
        }
    }
    return result;
}

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