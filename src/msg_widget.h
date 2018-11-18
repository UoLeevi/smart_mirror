#ifndef MSG_WIDGET_H
#define MSG_WIDGET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "render_instr.h"

#include "SDL.h"
#include "SDL_ttf.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct msg_widget 
{
    SDL_Thread *thrd;
    void *conf;
    void *ipcs;
    int x, y;
    render_instr_dest_origin dest_origin;
    bool is_ready;
    bool is_close_requested;
    struct {
        render_instr msg;
    } render_instrs;
} msg_widget;

bool msg_widget_init(
    SDL_Renderer *renderer);

msg_widget *msg_widget_create(
    int x, 
    int y,
    render_instr_dest_origin dest_origin);

void msg_widget_destroy(
    msg_widget *widget);

bool msg_widget_is_render_required(
    msg_widget *widget);

bool msg_widget_render(
    msg_widget *widget);

bool msg_widget_update_dest(
    msg_widget *widget);

#ifdef __cplusplus
}
#endif

#endif