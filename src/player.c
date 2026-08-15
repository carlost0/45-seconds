#include <math.h>
#include "../aschii/utils.h"

#include "player.h"

#include "state.h"
#include "render.h"

void move_player(struct player_t* player, f64 dt, dir_e dir, map_t map) {
    f64 nx, ny;
    f64 old_dir_x, old_plane_x;
    i32 ix, iy, iny, inx;

    switch (dir) {
        case UP:
            nx = player->pos.x + player->dir.x * player->move_speed * dt;
            ny = player->pos.y + player->dir.y * player->move_speed * dt;

            ix = (i32)floor(player->pos.x);
            iy = (i32)floor(player->pos.y);
            inx = (i32)floor(nx);
            iny = (i32)floor(ny);

            // move in X
            if (in_bounds(inx, iy) && map.data[iy * map.size.w + inx] == 0)
                player->pos.x = nx;

            // move in Y
            if (in_bounds(ix, iny) && map.data[iny * map.size.w + ix] == 0)
                player->pos.y = ny;

            break;

        case DOWN:
            nx = player->pos.x - player->dir.x * player->move_speed * dt;
            ny = player->pos.y - player->dir.y * player->move_speed * dt;

            ix = (i32)floor(player->pos.x);
            iy = (i32)floor(player->pos.y);
            inx = (i32)floor(nx);
            iny = (i32)floor(ny);

            // move in X
            if (in_bounds(inx, iy) && map.data[iy * map.size.w + inx] == 0)
                player->pos.x = nx;

            // move in Y
            if (in_bounds(ix, iny) && map.data[iny * map.size.w + ix] == 0)
                player->pos.y = ny;

            break;

        case RIGHT:
            old_dir_x = player->dir.x;
            player->dir.x = player->dir.x * cos(-player->rot_speed * dt) - player->dir.y * sin(-player->rot_speed * dt);
            player->dir.y = old_dir_x     * sin(-player->rot_speed * dt) + player->dir.y * cos(-player->rot_speed * dt);

            old_plane_x = player->plane.x;
            player->plane.x = player->plane.x * cos(-player->rot_speed * dt) - player->plane.y * sin(-player->rot_speed * dt);
            player->plane.y = old_plane_x     * sin(-player->rot_speed * dt) + player->plane.y * cos(-player->rot_speed * dt);

            break;

        case LEFT:
            old_dir_x = player->dir.x;
            player->dir.x = player->dir.x * cos(player->rot_speed * dt) - player->dir.y * sin(player->rot_speed * dt);
            player->dir.y = old_dir_x     * sin(player->rot_speed * dt) + player->dir.y * cos(player->rot_speed * dt);

            old_plane_x = player->plane.x;
            player->plane.x = player->plane.x * cos(player->rot_speed * dt) - player->plane.y * sin(player->rot_speed * dt);
            player->plane.y = old_plane_x     * sin(player->rot_speed * dt) + player->plane.y * cos(player->rot_speed * dt);

            break;

        default:
            break;
    }
}

int mine(struct player_t player, map_t *map) {
    bool hit = false;

    f64 camera_x = (2.0 * (SCREEN_W / 2)) / (f64)SCREEN_W - 1.0;

    f64 ray_dir_x = player.dir.x + player.plane.x * camera_x;
    f64 ray_dir_y = player.dir.y + player.plane.y * camera_x;

    f64 delta_dist_x = (ray_dir_x == 0) ? 1e30 : fabs(1 / ray_dir_x);
    f64 delta_dist_y = (ray_dir_y == 0) ? 1e30 : fabs(1 / ray_dir_y);

    f64 side_dist_x, side_dist_y;

    i32 map_x = (i32)floor(player.pos.x);
    i32 map_y = (i32)floor(player.pos.y);

    i32 step_x, step_y;
    i32 steps = 0;

    if (ray_dir_x < 0) {
        step_x      = -1;
        side_dist_x = (player.pos.x - map_x) * delta_dist_x;
    } else {
        step_x      = 1;
        side_dist_x = (map_x + 1.0 - player.pos.x) * delta_dist_x;
    }

    if (ray_dir_y < 0) {
        step_y      = -1;
        side_dist_y = (player.pos.y - map_y) * delta_dist_y;
    } else {
        step_y      = 1;
        side_dist_y = (map_y + 1.0 - player.pos.y) * delta_dist_y;
    }

    while (!hit) {
        if (map_x < 0 || map_x >= map->size.w || map_y < 0 || map_y >= map->size.h) {
            hit = 1;
            break;
        }

        if (side_dist_x < side_dist_y) {
            side_dist_x += delta_dist_x;
            map_x += step_x;
        } else {
            side_dist_y += delta_dist_y;
            map_y += step_y;
        }


        if (map->data[map_y * map->size.w + map_x] > 0) {
            hit = 1;
        }

        steps++;
    }

    delay(200);

    if (!hit || steps > 3 || map->data[map_y * map->size.w + map_x] == 5) return 0;

    map->data[map_y * map->size.w + map_x] -= 1;

    return map->data[map_y * map->size.w + map_x] + 1;

}

