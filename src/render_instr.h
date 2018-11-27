#ifndef RENDER_INSTR_H
#define RENDER_INSTR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SDL.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct uo_relpoint 
{
    struct
    {
        int16_t px;
        int16_t pct_screen;
        int16_t pct_self;
    } x;
    struct
    {
        int16_t px;
        int16_t pct_screen;
        int16_t pct_self;
    } y;
} uo_relpoint;

typedef struct render_instr {
    SDL_Texture *texture;
    SDL_Rect dest;
    SDL_Surface *surface;
    uo_relpoint reldest;
    int w;
    int h;
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

void render_instr_set(
    render_instr *instr,
    SDL_Surface *surface,
    uo_relpoint reldest,
    int w,
    int h);

void render_instr_set_s(
    render_instr *instr,
    SDL_Surface *surface);

void render_instr_set_swh(
    render_instr *instr,
    SDL_Surface *surface,
    int w,
    int h);

void render_instr_set_xy(
    render_instr *instr,
    uo_relpoint reldest);

bool render_instr_is_render_required(
    render_instr *instr);

void render_instr_reposition(
    render_instr *instr);

void render_instr_update(
    render_instr *instr);

void render_instr_render(
    render_instr *instr,
    SDL_Renderer *renderer); 

#ifdef __cplusplus
}
#endif

#endif