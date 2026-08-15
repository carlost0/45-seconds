#ifndef _STATE_H
#define _STATE_H

#include "../aschii/utils.h"
#include <stdint.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#define SCREEN_W 211
#define SCREEN_H 49

#define MAP_H 12
#define MAP_W 12

#define SOUND_AMOUNT 6

typedef float    f32;
typedef double   f64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef struct {
    box_t size;
    u8 *data;
} map_t;

static const color_t fog_color = {20, 20, 10};
static const f64 fog_density   = 2.5;
static const i32 max_fog_steps = 10;

static f64 volume      = 32.0;
static f64 camera_sens = 2.0;

static const f64 max_volume = 128;
static const f64 max_camera_sens = 5;

static inline uint64_t now_ms(void) {
#ifndef _WIN32
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#else
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER uli;
    uli.LowPart  = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    // FILETIME is 100-ns ticks -> milliseconds
    uint64_t ms = uli.QuadPart / 10000ULL;
    return ms;
#endif
}

#endif // _STATE_H
