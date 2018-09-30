#ifndef RENDER_INSTR_H
#define RENDER_INSTR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SDL.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define RENDER_INSTR_DEST_ORIGIN_FROM_STR(dest_origin, str) { \
    char *s = str; \
    switch (s[0]) \
    { \
        case 'T': \
            switch (s[4]) \
            { \
                case 'L': dest_origin = TOP_LEFT; break; \
                case 'C': dest_origin = TOP_CENTER; break; \
                case 'R': dest_origin = TOP_RIGHT; break; \
            } \
            break; \
    \
        case 'M': \
            switch (s[7]) \
            { \
                case 'L': dest_origin = MIDDLE_LEFT; break; \
                case 'C': dest_origin = MIDDLE_CENTER; break; \
                case 'R': dest_origin = MIDDLE_RIGHT; break; \
            } \
            break; \
    \
        case 'B': \
            switch (s[7]) \
            { \
                case 'L': dest_origin = BOTTOM_LEFT; break; \
                case 'C': dest_origin = BOTTOM_CENTER; break; \
                case 'R': dest_origin = BOTTOM_RIGHT; break; \
            } \
            break;\
    } \
}

typedef enum render_instr_dest_origin {
    TOP_LEFT,       TOP_CENTER,     TOP_RIGHT,
    MIDDLE_LEFT,    MIDDLE_CENTER,  MIDDLE_RIGHT,
    BOTTOM_LEFT,    BOTTOM_CENTER,  BOTTOM_RIGHT
} render_instr_dest_origin;

typedef struct render_instr {
    SDL_Texture *texture;
    SDL_Rect dest[2];
    SDL_Surface *surface;
    render_instr_dest_origin dest_origin;
    uint8_t alpha;
    uint8_t flags;
    /* flags:
        0b0000_0000 no changes
        0b0000_0001 surface changed
        0b0000_0010 xy changed
        0b0000_0100 wh changed
        0b1000_0000 ready
    */
} render_instr;

bool render_instr_query_output_xy(
    SDL_Renderer *renderer,
    render_instr_dest_origin dest_origin,
    int *x,
    int *y);

bool render_instr_set_s(
    render_instr *instr,
    SDL_Surface *surface);

bool render_instr_set_swh(
    render_instr *instr,
    SDL_Surface *surface,
    int w,
    int h);

bool render_instr_set_xy(
    render_instr *instr,
    int x,
    int y);

bool render_instr_update(
    render_instr *instr);

bool render_instr_render(
    render_instr *instr,
    SDL_Renderer *renderer); 

#ifdef __cplusplus
}
#endif

#endif