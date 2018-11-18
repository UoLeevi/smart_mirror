#include "msg_widget.h"

#include "uo_conf.h"
#include "uo_ipc.h"
#include "uo_ipcs.h"

#include "SDL.h"
#include "SDL_timer.h"
#include "SDL_image.h"

#include <stdio.h>
#include <string.h>

static bool is_init;
static bool init_result;

static SDL_Renderer *renderer;

static SDL_Color white = { 255, 255, 255 };

static TTF_Font *fontXL, *fontL, *fontM, *fontS;

static msg_widget *active_widget;

static void *msg_widget_handle_cmd(
    uo_ipcmsg *cmd,
    uo_cb *uo_ipcmsg_cb)
{    
    bool has_msg = cmd->data && cmd->data_len;

    if (has_msg)
    {
        int w, h;

        SDL_Surface *surface_msg = TTF_RenderUTF8_Blended(fontM, cmd->data, white);
        TTF_SizeUTF8(fontM, cmd->data, &w, &h);
        render_instr_set_swh(&active_widget->render_instrs.msg, surface_msg, w, h);

        render_instr_update(&active_widget->render_instrs.msg);

        active_widget->is_ready = true;
    }

    uo_cb_invoke_async(uo_ipcmsg_cb, NULL, NULL);

    if (has_msg)
    {
        SDL_Delay(3000);

        int w, h;

        SDL_Surface *surface_msg = TTF_RenderUTF8_Blended(fontM, "", white);
        render_instr_set_swh(&active_widget->render_instrs.msg, surface_msg, 0, 0);

        render_instr_update(&active_widget->render_instrs.msg);

        active_widget->is_ready = true;
    }
}

static int init_update_msg(
	void *arg)
{
    msg_widget *widget = active_widget = arg;

    int x = widget->x;
    int y = widget->y;
    render_instr_query_output_xy(renderer, widget->dest_origin, &x, &y);
    widget->render_instrs.msg.dest_origin = MIDDLE_CENTER;
    render_instr_set_xy(&widget->render_instrs.msg, x, y);

    uo_conf *conf = widget->conf = uo_conf_create("msg_widget.conf");

    char *port = uo_conf_get(conf, "port");
    widget->ipcs = uo_ipcs_create(port, strlen(port), msg_widget_handle_cmd);

}

static void msg_widget_quit(void)
{
    TTF_CloseFont(fontXL);
    TTF_CloseFont(fontL);
    TTF_CloseFont(fontM);
    TTF_CloseFont(fontS);
}


bool msg_widget_init(
    SDL_Renderer *renderer_)
{
    if (is_init)
        return init_result;
    
    renderer = renderer_;

    fontXL = TTF_OpenFont("assets/fonts/OpenSans-Regular.ttf", 64);
    fontL  = TTF_OpenFont("assets/fonts/OpenSans-Regular.ttf", 48);
    fontM  = TTF_OpenFont("assets/fonts/OpenSans-Regular.ttf", 32);
    fontS  = TTF_OpenFont("assets/fonts/OpenSans-Regular.ttf", 24);

    is_init = true;

    is_init &= uo_cb_init();
    is_init &= uo_ipc_init();
    
    atexit(msg_widget_quit);

    return init_result = true;
}

msg_widget *msg_widget_create(
    int x, 
    int y,
    render_instr_dest_origin dest_origin)
{
    msg_widget *widget = calloc(1, sizeof(msg_widget));
    widget->x = x;
    widget->y = y;
    widget->dest_origin = dest_origin;
    widget->thrd = SDL_CreateThread(init_update_msg, "msg_widget_thread", widget);

    return widget;
}

void msg_widget_destroy(
    msg_widget *widget)
{
    widget->is_close_requested = true;
    SDL_DetachThread(widget->thrd);
    free(widget);
}

bool msg_widget_is_render_required(
    msg_widget *widget)
{
    return !!widget->render_instrs.msg.flags;
}

bool msg_widget_render(
    msg_widget *widget)
{
    if (widget->is_ready) {
        render_instr_render(&widget->render_instrs.msg, renderer);
    }
}

bool msg_widget_update_dest(
    msg_widget *widget) 
{
    int x = widget->x;
    int y = widget->y;
    render_instr_query_output_xy(renderer, widget->dest_origin, &x, &y);
    render_instr_set_xy(&widget->render_instrs.msg, x, y);
    render_instr_update(&widget->render_instrs.msg);
}