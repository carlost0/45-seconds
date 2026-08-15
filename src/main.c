#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#ifndef _WIN32
#include <pthread.h>

#else
#include <windows.h>
#endif

#include "../aschii/utils.h"
#include "../aschii/keyboard.h"

#include "state.h"
#include "render.h"
#include "player.h"
#include "maze.h"

error_e err;
static const u32 max_time = 45;

error_e init_map(map_t *map, u8 src[MAP_H][MAP_W]) {
    if (!map) {
        perror("unable to init map, map is a null pointer");
        return error;
    }
    map->data = (u8 *) malloc((map->size.w * map->size.h) * sizeof(u8));

    if (map->data == NULL) {
        perror("unable to init map, failed to allocate data memory");
        return error;
    }

    if (!map->data) {
        perror("unable to init map, map is a null pointer");
        return error;
    }

    for (int i = 0; i < MAP_H; ++i) {
        for (int j = 0; j < MAP_W; ++j) {
            map->data[i * map->size.w + j] = src[i][j];
        }
    }

    return success;
}

void u32_to_str(u32 n, char *str, size_t cap, char *src) {
    snprintf(str, cap, "%u%s", (unsigned)n, src);
}

int main_level(void) {
    bool sound = true;
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        perror("unable to init sdl, continuing without sound");
        sound = false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048)) {
        perror("unable to init sdl, continuing without sound");
        sound = false;
    }

    Mix_Chunk *sounds[SOUND_AMOUNT];

    sounds[0] = Mix_LoadWAV("assets/mine_success.wav");
    sounds[1] = Mix_LoadWAV("assets/mine_fail.wav");
    sounds[2] = Mix_LoadWAV("assets/monster_close.wav");
    sounds[3] = Mix_LoadWAV("assets/loss.wav");
    sounds[4] = Mix_LoadWAV("assets/win.wav");
    sounds[5] = Mix_LoadWAV("assets/mine_gold.wav");

    Mix_MasterVolume(volume);
    INIT_INPUT;
    i32 fps = 60;

    scene_t scene = {
        .size   = {SCREEN_W, SCREEN_H},
        .screen = 0,
        .colors = 0,
    };
    
    err = init_scene(&scene);

    if (err != success) {
        END_INPUT;
        return -1;
    }

    u8 world_map[MAP_H][MAP_W] = {
        0
    };

    map_t map = {

        .size = {MAP_W, MAP_H},
        .data = 0
    };

    err = init_map(&map, world_map);
    if (err != success) {
        END_INPUT;
        return -1;
    }

    generate_maze(&map);
    lines_t lines  = {0};
    render_ctx_t ctx = {0};

    struct player_t player = {
        .pos   = { 1.3,   1.3},
        .dir   = { 0.75,  0.25},
        .plane = { 0,    -0.66},
    #ifndef _WIN32
        .move_speed = 6,
    #else
        .move_speed = 5,
    #endif
        .rot_speed  = camera_sens,

    };

    for (int i = 0; i < MAP_W * MAP_H; ++i) {
        if (map.data[i] == 0) map.data[i] = 1;
    }

    map.data[(i32)player.pos.y * map.size.w + (i32)player.pos.x] = 0;

    bool random_chars = false;

    u32 start_time = (u32) time(NULL), c_time = 0;

    clock_t dt_old_time  = 0,   dt_time = 0;
    f64 dt            = 0;

    dt_old_time = now_ms();

    char time_str[64];
    //main loop
    while (input != 'q') {
        //game logic
        GET_INPUT;
        
        dt_old_time = dt_time;
        dt_time     = now_ms();
        dt          = dt_time - dt_old_time;

        if (dt > 0.05) dt = 0.05;
        if (dt < 0) dt = 0;
       
        dir_e dir;
        if (input == 'w') dir = UP;
        else if (input == 'a') dir = LEFT;
        else if (input == 's') dir = DOWN;
        else if (input == 'd') dir = RIGHT;
        else dir = NONE;

        move_player(&player, dt, dir, map);

        int last_mined = 0;
        if (input == ' ') {
            last_mined = mine(player, &map);

            if (last_mined == 0) {
                Mix_PlayChannel(-1, sounds[1], 0);
            } else if (last_mined == 1) {
                Mix_PlayChannel(-1, sounds[0], 0);
            } else if (last_mined > 1) {
                Mix_PlayChannel(-1, sounds[5], 0);
            }
        }

        if (last_mined == 2) {
            Mix_HaltChannel(-1);
            Mix_PlayChannel(-1, sounds[4], 0);
            goto win;
        }

        c_time = time(NULL) - start_time;

        if (c_time > max_time) {
            Mix_HaltChannel(-1);
            Mix_PlayChannel(-1, sounds[3], 0);
            goto loss;
        }
        //drawing / rendering
        ctx.pos   = player.pos;
        ctx.dir   = player.dir;
        ctx.plane = player.plane;

        lines = render(ctx, map);

        clear_scene(&scene);

        u32_to_str(max_time - c_time, time_str, sizeof(time_str), "s until it catches you. ");

        rectangle_str_t floor = {
            .pos = {0, SCREEN_H / 2 },
            .size = {SCREEN_W, SCREEN_H / 2},
            .color = {120, 110, 100},
            .str = time_str,
            .len = strlen(time_str),
        };

        rectangle_str_t roof = {
            .pos = {0, 0},
            .size = {SCREEN_W, SCREEN_H / 2},
            .color = {150, 120, 105},
            .str = time_str,
            .len = strlen(time_str),
        };

        random_chars = false;
        if (max_time - c_time == 20) {
            random_chars = true;
            floor.str = "hurry.. ";
            floor.len = strlen(floor.str);
            Mix_PlayChannel(-1, sounds[2], 0);
        }

        if (max_time - c_time == 15) {
            random_chars = true;
            floor.str = "it's comming.. ";
            floor.len = strlen(floor.str);
            Mix_PlayChannel(-1, sounds[2], 0);
        }

        if (max_time - c_time < 12) {
            random_chars = true;
            floor.str = "it's close.. ";
            floor.len = strlen(floor.str);
        }

        if (max_time - c_time == 11) {
            Mix_PlayChannel(-1, sounds[2], 0);
        }

        if (max_time - c_time < 10) {
            random_chars = false;
            floor.str = time_str;
            floor.len = strlen(floor.str);
        }


        if (max_time - c_time < 8) {
            random_chars = true;
            floor.str = "I CAN SEE YOU. ";
            floor.len = strlen(floor.str);
        }

        roof.str = floor.str;
        roof.len = floor.len;

        if (max_time - c_time == 7) {
            Mix_PlayChannel(-1, sounds[2], 0);
        }

        err = draw_rectangle_str(&scene, floor);
        if (err != success) goto error;

        err = draw_rectangle_str(&scene, roof);
        if (err != success) goto error;

        for (i32 i = 0; i < SCREEN_W; ++i) {
            if (lines.walls.h[i] > 0) {
                err = draw_rectangle(&scene, (rectangle_t) {
                    .pos = {i, lines.walls.y[i]},
                    .size = {1, lines.walls.h[i]},
                    .color = lines.walls.color[i],
                    .sprite = lines.walls.sprite[i]
                }, random_chars);
                if (err != success) goto error;
            }
        }

        if (c_time < 10) {
            char *str = "use W/S to move, S/D to look around and SPACE to mine rubble (r). quit with Q";
            draw_text_horizontal(&scene, (text_t){
                .pos   = {SCREEN_W / 2 - strlen(str) / 2, 11},
                .color = {255, 255, 255},
                .str   = str,
            });
        }



        draw_screen_borders(&scene, (color_t) {255,255,255});
        print_scene(&scene);
        fflush(stdout);
    }

    Mix_HaltChannel(-1);
    delay(500);

    for (int i = 0; i < SOUND_AMOUNT; ++i)
        Mix_FreeChunk(sounds[i]);

    delay(100);

    Mix_CloseAudio();
    SDL_Quit();

    free(scene.screen);
    free(scene.colors);
    free(map.data);
    END_INPUT;
    return 0;

win: 
    END_INPUT;
    clear_scene(&scene);
    clear_screen();

    draw_rectangle_str(&scene, (rectangle_str_t) {
        .pos  = {0, 0},
        .size = {SCREEN_W, SCREEN_H},
        .color = {244, 182, 24},
        .str = "you won :) ",
        .len = strlen("you won :) "),
    });

    for (i32 i = 0; i < SCREEN_W; ++i) {
        if (lines.walls.h[i] > 0) {
            err = draw_rectangle(&scene, (rectangle_t) {
                .pos = {i, lines.walls.y[i]},
                .size = {1, lines.walls.h[i]},
                .color = lines.walls.color[i],
                .sprite = ' '
            }, false);
            if (err != success) goto error;
        }
    }

    print_scene(&scene);

    delay(2000);
    Mix_HaltChannel(-1);
    delay(500);

    for (int i = 0; i < SOUND_AMOUNT; ++i)
        Mix_FreeChunk(sounds[i]);

    delay(100);

    Mix_CloseAudio();
    SDL_Quit();

    free(scene.screen);
    free(scene.colors);
    free(map.data);
    return 1;

loss:
    END_INPUT;
    clear_scene(&scene);
    clear_screen();
    
    draw_rectangle_str(&scene, (rectangle_str_t) {
        .pos  = {0, 0},
        .size = {SCREEN_W, SCREEN_H},
        .color = {170, 20, 20},
        .str = "it catched you. ",
        .len = strlen("it catched you. "),
    });

    for (i32 i = 0; i < SCREEN_W; ++i) {
        if (lines.walls.h[i] > 0) {
            err = draw_rectangle(&scene, (rectangle_t) {
                .pos = {i, lines.walls.y[i]},
                .size = {1, lines.walls.h[i]},
                .color = lines.walls.color[i],
                .sprite = ' '
            }, false);
            if (err != success) goto error;
        }
    }


    print_scene(&scene);

    delay(2000);
    Mix_HaltChannel(-1);
    delay(500);

    for (int i = 0; i < SOUND_AMOUNT; ++i)
        Mix_FreeChunk(sounds[i]);

    delay(100);

    Mix_CloseAudio();
    SDL_Quit();

    free(scene.screen);
    free(scene.colors);
    free(map.data);
    return 1;

error:
    Mix_HaltChannel(-1);
    delay(500);

    for (int i = 0; i < SOUND_AMOUNT; ++i)
        Mix_FreeChunk(sounds[i]);

    delay(100);

    Mix_CloseAudio();
    SDL_Quit();

    END_INPUT;
    free(scene.screen);
    free(scene.colors);
    free(map.data);
    return -1;
}

i32 start_screen() {
    scene_t scene = {
        .size = {SCREEN_W, SCREEN_H},
    };

    i32 ret = 0;
    error_e err;

    err = init_scene(&scene);

    if (err != success) return -1;

    rectangle_t bg_box = {
        .pos = {SCREEN_W / 2 - (u32)(SCREEN_W / 3 / 2), SCREEN_H / 2 - (u32)(SCREEN_H / 2 / 2)},
        .size = {(u32) (SCREEN_W / 3), (u32) (SCREEN_H / 2)},
        .sprite = '-',
        .color = {180, 180, 180},
    };

    rectangle_t outline = {
        .pos = {bg_box.pos.x - 1, bg_box.pos.y - 1},
        .size = {bg_box.size.w + 2, bg_box.size.h + 2},
        .sprite = '#',
        .color = {255, 255, 255},
    };

    rectangle_str_t start_game_button = {
        .pos   = {bg_box.pos.x + (u32)(bg_box.size.w / 10), bg_box.pos.y + (u32)(bg_box.size.h / 8)},
        .size  = {(u32)(bg_box.size.w * 0.8), (u32)(bg_box.size.h / 6)},
        .color = {0, 100, 160},
        .str   = " new game ",
        .len   = strlen(" new game ")
    };

    rectangle_str_t options_button = { 
        .pos   = {start_game_button.pos.x, start_game_button.pos.y + start_game_button.size.h + (u32) (start_game_button.size.h / 2)},
        .size  = start_game_button.size,
        .color = {225, 155, 25},
        .str   = "options ",
        .len   = strlen("options "),
    };

    rectangle_str_t quit_button = { 
        .pos   = {start_game_button.pos.x, options_button.pos.y + options_button.size.h + (u32) (options_button.size.h / 2)},
        .size  = start_game_button.size,
        .color = {225, 25, 25},
        .str   = "quit ",
        .len   = strlen("quit "),
    };
    INIT_INPUT;

    rectangle_str_t bg = {
        .pos = {0, 0},
        .size = {SCREEN_W, SCREEN_H},
        .color = {150, 120, 105},
        .str = "45 seconds  ",
        .len = strlen("45 seconds  "),
    };

    rectangle_t cursor = {
        .pos   = {start_game_button.pos.x - 3, start_game_button.pos.y + 1},
        .size  = {start_game_button.size.w + 6, start_game_button.size.h - 2},
        .color = {255, 255, 255},
        .sprite = 'c',
    };

    clear_screen();

    u32 start_time = (u32) time(NULL), c_time = 0;

    char time_str[64];
    while (1) {
        GET_INPUT;

        c_time = time(NULL) - start_time;

        if (input == 'w' && ret > 0) {
            ret--;
            cursor.pos.y -= 6;
        }

        if (input == 's' && ret < 2) {
            ret++;
            cursor.pos.y += 6;
        }

        if (input == ' ') break;

        if (input == '1') {
            ret = 0; 
            break;
        }
        if (input == '2') {
            ret = 1;
            break;
        }
        if (input == 'q') {
            ret = 2; 
            break;
        }

        if (c_time > max_time) {
            ret = 1;
            break;
        }

        u32_to_str(max_time - c_time, time_str, sizeof(time_str), " seconds ");

        bg.str = time_str;
        bg.len = strlen(time_str);

        draw_rectangle_str(&scene, bg);

        draw_rectangle(&scene, outline, true);
        draw_rectangle(&scene, bg_box, false);

        draw_rectangle(&scene, cursor, true);

        draw_text_horizontal(&scene, (text_t) {
            .pos   = {(u32)(scene.size.w / 2) - (u32)(strlen("use W/S to move the cursor and SPACE to confirm") / 2), bg_box.pos.y + 1},
            .str   = "use W/S to move the cursor and SPACE to confirm",
            .color = {255, 255, 255}
        });


        draw_rectangle_str(&scene, start_game_button);
        draw_rectangle_str(&scene, options_button);
        draw_rectangle_str(&scene, quit_button);

        draw_screen_borders(&scene, (color_t) {255, 255, 255});

        print_scene(&scene);
        clear_scene(&scene);
    }

    clear_screen();
    END_INPUT;
    free(scene.colors);
    free(scene.screen);

    return ret;
}

error_e options_screen() {
    scene_t scene = {
        .size = {SCREEN_W, SCREEN_H},
    };

    error_e err;

    err = init_scene(&scene);

    if (err != success) return error;

    typedef struct {
        rectangle_str_t bg;
        rectangle_str_t bar;
    } slider_t;

    rectangle_t bg_box = {
        .pos = {SCREEN_W / 2 - (u32)(SCREEN_W / 3 / 2), SCREEN_H / 2 - (u32)(SCREEN_H / 2 / 2)},
        .size = {(u32) (SCREEN_W / 3), (u32) (SCREEN_H / 2)},
        .sprite = '-',
        .color = {180, 180, 180},
    };

    rectangle_t outline = {
        .pos = {bg_box.pos.x - 1, bg_box.pos.y - 1},
        .size = {bg_box.size.w + 2, bg_box.size.h + 2},
        .sprite = '#',
        .color = {255, 255, 255},
    };

    slider_t volume_slider = {
        .bg = {
            .pos   = {bg_box.pos.x + (u32)(bg_box.size.w / 10), bg_box.pos.y + (u32)(bg_box.size.h / 6)},
            .size  = {(u32)(bg_box.size.w * 0.8), 3},
            .color = {0, 100, 160},
            .str   = "volume",
            .len   = strlen("volume"),
        },
        .bar = {
            .pos   = {bg_box.pos.x + (u32)(bg_box.size.w / 10) + 1, bg_box.pos.y + (u32)(bg_box.size.h / 6) + 1},
            .size  = {(u32)(bg_box.size.w * 0.8) - 2, 1},
            .color = {255, 255, 255},
            .str   = "volume ",
            .len   = strlen("volume "),
        }
    };

    slider_t sens_slider = {
        .bg = {
            .pos   = {volume_slider.bg.pos.x, volume_slider.bg.pos.y + volume_slider.bg.size.h + 2},
            .size  = volume_slider.bg.size,
            .color = {0, 100, 160},
            .str   = " camera sensitivity ",
            .len   = strlen(" camera sensitivity "),
        },
        .bar = {
            .pos   = {volume_slider.bg.pos.x + 1, volume_slider.bg.pos.y + volume_slider.bg.size.h + 2 + 1},
            .size  = volume_slider.bar.size,
            .color = {255, 255, 255},
            .str   = "camera sensitivity ",
            .len   = strlen("camera sensitivity "),
        }
    };

    rectangle_str_t quit_button = {
        .pos   = {bg_box.pos.x + (u32)(bg_box.size.w / 6), bg_box.pos.y + bg_box.size.h - (u32)(bg_box.size.h / 4)},
        .size  = {bg_box.size.w - (u32)(bg_box.size.w / 3), (u32)(bg_box.size.h / 8)},
        .color = {225, 25, 25},
        .str   = "exit ",
        .len   = strlen("exit ")
    };


    rectangle_str_t bg = {
        .pos = {0, 0},
        .size = {SCREEN_W, SCREEN_H},
        .color = {150, 120, 105},
        .str = "options  ",
        .len = strlen("options  "),
    };



    rectangle_t cursor = {
        .pos   = {volume_slider.bg.pos.x - 2, 0},
        .size  = {volume_slider.bg.size.w + 4, volume_slider.bar.size.h},
        .color = {255, 255, 255},
        .sprite = 'c',
    };

    i32 y_pos[3] = {
        volume_slider.bar.pos.y,
        sens_slider.bar.pos.y,
        quit_button.pos.y + 1,
    };

    i32 max_size = volume_slider.bar.size.w;

    i32 i = 0;

    INIT_INPUT;
    clear_screen();
    while (1) {
        GET_INPUT;

        if (input == 'w' && i > 0) i--;
        if (input == 's' && i < 2) i++;

        cursor.pos.y = y_pos[i];

        if (input == 'q') break;
        if (input == ' ' && i == 2) break;

        if (input == 'a') {
            if (i == 0 && volume_slider.bar.size.w > 0) {
                volume -= max_volume / 100.0;
            }
            else if (i == 1 && sens_slider.bar.size.w > 0) {
                camera_sens -= max_camera_sens / 100.0;
            }
        }

        if (input == 'd') {
            if (i == 0 && volume_slider.bar.size.w < max_size) {
                volume += max_volume / 100.0;
            }
            else if (i == 1 && sens_slider.bar.size.w < max_size) {
                camera_sens += max_camera_sens / 100.0;
            }
        }

        volume_slider.bar.size.w = (u32)(max_size * (volume / max_volume));
        sens_slider.bar.size.w = (u32)(max_size * (camera_sens / max_camera_sens));


        draw_rectangle_str(&scene, bg);

        draw_rectangle(&scene, outline, true);
        draw_rectangle(&scene, bg_box, false);

        draw_rectangle(&scene, cursor, true);
        draw_rectangle_str(&scene, volume_slider.bg);
        draw_rectangle_str(&scene, volume_slider.bar);

        draw_rectangle_str(&scene, sens_slider.bg);
        draw_rectangle_str(&scene, sens_slider.bar);

        draw_rectangle_str(&scene, quit_button);


        draw_text_horizontal(&scene, (text_t) {
            .pos   = {(u32)(scene.size.w / 2) - (u32)(strlen("use W/S to move the cursor and SPACE to confirm") / 2), bg_box.pos.y + 1},
            .str   = "use W/S to move the cursor and SPACE to confirm",
            .color = {255, 255, 255}
        });

        draw_screen_borders(&scene, (color_t) {255, 255, 255});
        print_scene(&scene);
        clear_scene(&scene);
    }

    clear_screen();
    END_INPUT;
    free(scene.colors);
    free(scene.screen);

    return success;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    (void)hInstance; (void)hPrevInstance;
    (void)lpCmdLine; (void)nCmdShow;
#else
i32 main(i32 argc, char *argv[]) {
    (void)argc; (void)argv;
#endif
    i32 start_sel = start_screen();
    while (1) {

        i32 main_game_sel = -1;

        if (start_sel == 0) {
            main_game_sel = main_level();
        }

        if (main_game_sel >= 0)
            start_sel = start_screen();

        if (start_sel == 1) {
            options_screen();
            start_sel = start_screen();
        }
        else if (start_sel == 2 || main_game_sel == 0)
            return 0;
        else if (start_sel == -1 || main_game_sel < 0)
            return 1;
    }

    return 0;
}
