#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>

#include "../aschii/utils.h"
#include "state.h"

#include "maze.h"


struct cell_t *current;
struct stack_t stack;

static inline int idx(int x, int y, int w) {
    return y * w + x;
}

static void init_stack(struct stack_t *stack) {
    stack->top = -1;
}

static error_e push(struct stack_t *stack, int n) {
    if (stack->top >= MAX_STACK_SIZE - 1) {
        perror("unable to init stack, stack overflow");
        return error;
    }

    stack->arr[++stack->top] = n;
    return success;
}

static int pop(struct stack_t *stack) {
    int popped = stack->arr[stack->top];
    stack->top--;

    return popped;
} 

static void remove_walls(struct cell_t *a, struct cell_t *b) {
    int x = a->pos.x - b->pos.x;
    if (x == 1) {
        a->walls[3] = false;
        b->walls[1] = false;
    }

    if (x == -1) {
        a->walls[1] = false;
        b->walls[3] = false;
    }

    int y = a->pos.y - b->pos.y;
    if (y == 1) {
        a->walls[0] = false;
        b->walls[2] = false;
    }

    if (y == -1) {
        a->walls[2] = false;
        b->walls[0] = false;
    }
}

static int check_neighbors(struct cell_t *cells, struct cell_t cell, int w, int h) {
    int x = cell.pos.x;
    int y = cell.pos.y;

    assert(cell.pos.x >= 0 && cell.pos.x < w);
    assert(cell.pos.y >= 0 && cell.pos.y < h);

    int top_avail    = (y - 1 >= 0);
    int left_avail   = (x - 1 >= 0);
    int bottom_avail = (y + 1 < h);
    int right_avail  = (x + 1 < w);

    int candidates[4];
    int i = 0;

    if (top_avail) {
        int idx_top = idx(x, y - 1, w);
        if (!cells[idx_top].visited) candidates[i++] = idx_top;
    }
    if (left_avail) {
        int idx_left = idx(x - 1, y, w);
        if (!cells[idx_left].visited) candidates[i++] = idx_left;
    }
    if (bottom_avail) {
        int idx_bottom = idx(x, y + 1, w);
        if (!cells[idx_bottom].visited) candidates[i++] = idx_bottom;
    }
    if (right_avail) {
        int idx_right = idx(x + 1, y, w);
        if (!cells[idx_right].visited) candidates[i++] = idx_right;
    }

    if (i > 0) return candidates[rand() % i];
    return -1; // no move
}

static void draw_cell(map_t *map, point_t pos, box_t size) {
    for (int i = pos.y; i < pos.y + size.h; i++) {
        for (int j = pos.x; j < pos.x + size.w; j++) {
            map->data[i * map->size.w + j] = 5;
        }
    }
}

static void show_cell(map_t *map, struct cell_t cell) {

    if (cell.walls[0]) {
        draw_cell(map, 
            (point_t) {cell.pos.x * CELL_W, cell.pos.y * CELL_H},
            (box_t)   {CELL_W, 1});
    }

    if (cell.walls[1]) {
        draw_cell(map, 
            (point_t) {cell.pos.x * CELL_W + (CELL_W - 1), cell.pos.y * CELL_H},
            (box_t)   {1, CELL_H});
    }

    if (cell.walls[2]) {
        draw_cell(map, 
            (point_t) {cell.pos.x * CELL_W, cell.pos.y * CELL_H + (CELL_H - 1)},
            (box_t)   {CELL_W, 1});
    }

    if (cell.walls[3]) {
        draw_cell(map, 
            (point_t) {cell.pos.x * CELL_W, cell.pos.y * CELL_H},
            (box_t)   {1, CELL_H});
    }

}


void generate_maze(map_t *map) {
    srand(now_ms());
    int w = map->size.w / CELL_W;
    int h = map->size.h / CELL_H;

    struct cell_t *cells = malloc(w * h * sizeof(struct cell_t));
    if (!cells)
        return;

    init_stack(&stack);

    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            cells[y * w + x] = (struct cell_t) {
                .pos = {x, y},
                .walls = {true, true, true, true},
                .visited = false
            };
        }
    }

    current = &cells[0];

    
    struct cell_t next;

    while (1) {
        for (int i = 0; i < map->size.w * map->size.h; ++i)
            map->data[i] = 0;

        for (int i = 0; i < w * h; ++i) {
            show_cell(map, cells[i]);
        }


        current->visited = true;
        int index = check_neighbors(cells, *current, w, h);

        if (index > -1) {
            int c_index = idx(current->pos.x, current->pos.y, w);
            push(&stack, c_index);
            remove_walls(current, &cells[index]);
            current = &cells[index];
        } else if (stack.top != -1) {
            int p_index = pop(&stack);     
            assert(p_index >= 0 && p_index < w*h);
            current = &cells[p_index];
        } else {
            break;
        }

    }

    point_t goal_pos = {0};
    point_t pos = {0};
    int max_dist = -1;

    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            int dist = abs(x - 0) + abs(y - 0);

            bool has_open_wall =
                !cells[y * w + x].walls[0] ||
                !cells[y * w + x].walls[1] ||
                !cells[y * w + x].walls[2] ||
                !cells[y * w + x].walls[3];

            if (dist > max_dist 
                && map->data[y * CELL_H * w + x * CELL_W] == 0
                && has_open_wall) {
                max_dist = dist;
                goal_pos.x = x;
                goal_pos.y = y;
            }
        }
    }

    pos.x = goal_pos.x * CELL_W;
    pos.y = goal_pos.y * CELL_H;
    map->data[pos.y * map->size.w + pos.x] = 4;

    assert(max_dist > 0);
    free(cells);
}


