#ifndef _PLAYER_H
#define _PLAYER_H

#include "state.h"
#include "render.h"
#include "../aschii/utils.h"

typedef enum {
    UP,
    RIGHT,
    DOWN,
    LEFT,
    NONE,
} dir_e;

struct player_t {
    vector2_t pos;
    vector2_t plane;
    vector2_t dir;
    f64       move_speed;
    f64       rot_speed;
};

void move_player(struct player_t* player, f64 dt, dir_e dir, map_t map);
int  mine(struct player_t player, map_t *map);

#endif //_PLAYER_H
