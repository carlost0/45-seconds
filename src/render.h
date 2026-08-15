#ifndef _RENDER_H
#define _RENDER_H

#include "state.h"

typedef struct {
    vector2_t pos;
    vector2_t dir;
    vector2_t plane;
    f64 water_height;
} render_ctx_t;

typedef struct {
    color_t  color [SCREEN_W];
    u16      y     [SCREEN_W];
    u16      h     [SCREEN_W];
    char     sprite[SCREEN_W];
} _render_component_v_t;

typedef struct {
    _render_component_v_t walls;
} lines_t;
 
bool in_bounds(int x, int y);
bool is_wall(point_t pos, map_t map);
lines_t render(render_ctx_t ctx, map_t map);
void render_floor_water(render_ctx_t ctx, map_t map, lines_t *lines);

#endif //_RENDER_H
