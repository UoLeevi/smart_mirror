#ifndef WEATHER_WIDGET_H
#define WEATHER_WIDGET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "render_instr.h"

#include "SDL.h"
#include "SDL_ttf.h"

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

typedef struct weather_widget {
    SDL_Thread *thrd;
    char *place;
    bool is_ready;
    bool is_close_requested;
    struct {
        render_instr celsius;
        render_instr symbol;
        render_instr time;
    } render_instrs[16];

} weather_widget;

bool weather_widget_init(
    SDL_Renderer *renderer);

weather_widget *weather_widget_create(
    char *place,
    uo_relpoint reldest);

void weather_widget_destroy(
    weather_widget *widget);

bool weather_widget_is_render_required(
    weather_widget *widget);

bool weather_widget_render(
    weather_widget *widget);

bool weather_widget_update_dest(
    weather_widget *widget); 

#ifdef __cplusplus
}
#endif

#endif