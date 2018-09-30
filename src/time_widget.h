#ifndef TIME_WIDGET_H
#define TIME_WIDGET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "render_instr.h"

#include "SDL.h"
#include "SDL_ttf.h"

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

typedef struct time_widget {
    SDL_Thread *thrd;
    int x, y;
    render_instr_dest_origin dest_origin;
    bool is_ready;
    bool is_close_requested;
    struct {
        render_instr time;
        render_instr wday;
        render_instr date;
    } render_instrs;
} time_widget;

bool time_widget_init(
    SDL_Renderer *renderer);

time_widget *time_widget_create(
    int x, 
    int y,
    render_instr_dest_origin dest_origin);

void time_widget_destroy(
    time_widget *widget);

bool time_widget_is_render_required(
    time_widget *widget);

bool time_widget_render(
    time_widget *widget);

bool time_widget_update_dest(
    time_widget *widget);

#ifdef __cplusplus
}
#endif

#endif