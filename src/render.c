#include <math.h>
#include "../aschii/utils.h"
#include "state.h"
#include "render.h"

bool in_bounds(int x, int y) {
    return x >= 0 && x < MAP_W && y >= 0 && y < MAP_H;
}

bool is_wall(point_t pos, map_t map) {
    return map.data[pos.y * map.size.w + pos.x] != 0;
}

lines_t render(render_ctx_t ctx, map_t map) {
    f64 pos_x    =  ctx.pos.x,   pos_y    = ctx.pos.y;
    f64 dir_x    =  ctx.dir.x,   dir_y    = ctx.dir.y;
    f64 plane_x  =  ctx.plane.x, plane_y  = ctx.plane.y;

    lines_t lines = {0};
    for (i32 x = 0; x < SCREEN_W; ++x) {
        f64 camera_x = (2.0 * x) / (f64)SCREEN_W - 1.0;

        f64 ray_dir_x = dir_x + plane_x * camera_x;
        f64 ray_dir_y = dir_y + plane_y * camera_x;

        i32 map_x = (i32)pos_x;
        i32 map_y = (i32)pos_y;

        f64 side_dist_x, side_dist_y;

        f64 delta_dist_x = (ray_dir_x == 0) ? 1e30 : fabs(1 / ray_dir_x);
        f64 delta_dist_y = (ray_dir_y == 0) ? 1e30 : fabs(1 / ray_dir_y);
        f64 perp_wall_dist = 0.0;

        i32 step_x, step_y;
        i32 steps = 0;

        i32 hit  = 0;
        i32 side = 0;
        
        if (ray_dir_x < 0) {
            step_x      = -1;
            side_dist_x = (pos_x - map_x) * delta_dist_x;
        } else {
            step_x      = 1;
            side_dist_x = (map_x + 1.0 - pos_x) *delta_dist_x;
        }

        if (ray_dir_y < 0) {
            step_y      = -1;
            side_dist_y = (pos_y - map_y) * delta_dist_y;
        } else {
            step_y      = 1;
            side_dist_y = (map_y + 1.0 - pos_y) *delta_dist_y;
        }



        while (!hit) {
            if (map_x < 0 || map_x >= MAP_W || map_y < 0 || map_y >= MAP_H) {
                hit = 1;
                perp_wall_dist = 1e30;
                break;
            }

            if (side_dist_x < side_dist_y) {
                side_dist_x += delta_dist_x;
                map_x += step_x;
                side = 0;
            } else {
                side_dist_y += delta_dist_y;
                map_y += step_y;
                side = 1;
            }

            if (map.data[map_y * map.size.w + map_x] > 0) {
                hit = 1;
            }
            steps++;
        }

        if (perp_wall_dist < 1e29) {
            if (side == 0)
                perp_wall_dist = side_dist_x - delta_dist_x;
            else
                perp_wall_dist = side_dist_y - delta_dist_y;
        }

        i32 line_h = (i32)(SCREEN_H / perp_wall_dist);

        i32 draw_start = -line_h / 2 + SCREEN_H / 2;
        i32 draw_end   = line_h / 2 + SCREEN_H / 2;

        if (draw_start < 0)         draw_start = 0;
        if (draw_end   < 0)         draw_end   = 0;
        if (draw_start >= SCREEN_H) draw_start = SCREEN_H - 1;
        if (draw_end   >= SCREEN_H) draw_end   = SCREEN_H - 1;

        if (draw_start > draw_end) {
            i32 tmp = draw_start;
            draw_start = draw_end;
            draw_end = tmp;
        }
        color_t color;

        if (map_x < 0 || map_x >= map.size.w || map_y < 0 || map_y >= map.size.h) {
            continue; // or set color to black and skip switch
        }

        switch (map.data[map_y * map.size.w + map_x]) {
            case 1:
                color = (color_t){160, 150, 150};
                lines.walls.sprite[x] = 'r';
                break;
            case 2:  
                color = (color_t){201, 153, 137};
                lines.walls.sprite[x] = 'S';
                break;
            case 3:
                color = (color_t){232, 163, 102};
                lines.walls.sprite[x] = 'O';
                break;
            case 4:
                color = (color_t){244, 182, 24};
                lines.walls.sprite[x] = 'G';
                break;
            default:
                color = (color_t){120, 100, 100};
                lines.walls.sprite[x] = '#';
                break;
        }
        
        if (side == 1) {
            color.r /= 1.5;
            color.g /= 1.5;
            color.b /= 1.5;
        }

        // fog
        f64 t = (f64)steps / (f64)max_fog_steps;
        if (t < 0.0) t = 0.0;
        if (t > 1.0) t = 1.0;

        f64 fog = 1.0 - exp(-fog_density * t); // 0..~1
        if (fog < 0.0) fog = 0.0;
        if (fog > 1.0) fog = 1.0;

        color.r = (u8)(color.r * (1.0 - fog) + fog_color.r * fog);
        color.g = (u8)(color.g * (1.0 - fog) + fog_color.g * fog);
        color.b = (u8)(color.b * (1.0 - fog) + fog_color.b * fog);

        lines.walls.y[x]      = draw_start;
        lines.walls.h[x]      = draw_end - draw_start;
        lines.walls.color[x]  = color;

    }
    return lines;
}


