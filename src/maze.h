#ifndef _MAZE_H
#define _MAZE_H

#include <stdbool.h>
#include "state.h"
#include "../aschii/utils.h"

#define CELL_W 3
#define CELL_H 3

#define MAX_STACK_SIZE ((SCREEN_W / CELL_W) * (SCREEN_H / CELL_H))

struct cell_t {
    point_t pos;
    bool    walls[4];
    bool    visited;
};

struct stack_t {
    int arr[MAX_STACK_SIZE];
    int top;
};

static inline int idx(int x, int y, int w);

static void init_stack(struct stack_t *stack);

static error_e push(struct stack_t *stack, int n);

static int pop(struct stack_t *stack);

static void remove_walls(struct cell_t *a, struct cell_t *b);

static int check_neighbors(struct cell_t *cells, struct cell_t cell, int w, int h);

static void show_cell(map_t *map, struct cell_t cell);

void generate_maze(map_t *map);


#endif //_MAZE_H
