#ifndef IMAGES_H
#define IMAGES_H "images.h"

#define player_image " ____ {_00_}8____8"
#define right_image "  ///] //00}//===]"
#define left_image "[\\\\\\  {00\\\\ [===\\\\\\"
#define falling_image "  __   /  \\ [_00_]"
#define chest_image " ___ /___\\[_0_]"
#define chest_image_alt " ___ /#-#\\{_$_}"
#define safeBlock_image "===================="
#define badBlock_image "xxxxxxxxxxxxxxxxxxxx"

typedef enum PLAYER_BITMAP {
    MOVING_RIGHT = 1,
    MOVING_LEFT = 2,
    FALLING = 3,
} PLAYER_BITMAP;

#endif