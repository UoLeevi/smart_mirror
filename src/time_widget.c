#include "time_widget.h"

#include "SDL.h"
#include "SDL_timer.h"
#include "SDL_image.h"

#include <stdio.h>
#include <time.h>

static bool is_init;
static bool init_result;

static SDL_Renderer *renderer;

static SDL_Color white = { 255, 255, 255 };

static TTF_Font *fontXL, *fontL, *fontM, *fontS;

static int update_time(
	void *arg)
{
    time_widget *widget = arg;

    char date_buf[0x10];
    char wday_buf[0x10];
    char time_buf[0x8];

    while (!widget->is_close_requested) 
    {
        time_t t = time(NULL);
        struct tm *now = localtime(&t);

        snprintf(date_buf, sizeof date_buf, "%d.%d.%04d", now->tm_mday, now->tm_mon + 1, now->tm_year + 1900);
        strftime(wday_buf, sizeof wday_buf, "%A", now);
        strftime(time_buf, sizeof time_buf, "%H:%M", now);

        int w, h;

        SDL_Surface *surface_date = TTF_RenderUTF8_Blended(fontS, date_buf, white);
        TTF_SizeUTF8(fontS, date_buf, &w, &h);
        render_instr_set_swh(&widget->render_instrs.date, surface_date, w, h);

        SDL_Surface *surface_wday = TTF_RenderUTF8_Blended(fontS, wday_buf, white);
        TTF_SizeUTF8(fontS, wday_buf, &w, &h);
        render_instr_set_swh(&widget->render_instrs.wday, surface_wday, w, h);

        SDL_Surface *surface_time = TTF_RenderUTF8_Blended(fontXL, time_buf, white);
        TTF_SizeUTF8(fontXL, time_buf, &w, &h);
        render_instr_set_swh(&widget->render_instrs.time, surface_time, w, h);

        render_instr_update(&widget->render_instrs.date);
        render_instr_update(&widget->render_instrs.wday);
        render_instr_update(&widget->render_instrs.time);

        widget->is_ready = true;

	    SDL_Delay((60 - now->tm_sec) * 1000);
    }
}

static void time_widget_quit(void)
{
    TTF_CloseFont(fontXL);
    TTF_CloseFont(fontL);
    TTF_CloseFont(fontM);
    TTF_CloseFont(fontS);
}


bool time_widget_init(
    SDL_Renderer *renderer_)
{
    renderer = renderer_;

    fontXL = TTF_OpenFont("assets/fonts/OpenSans-Regular.ttf", 64);
    fontL  = TTF_OpenFont("assets/fonts/OpenSans-Regular.ttf", 48);
    fontM  = TTF_OpenFont("assets/fonts/OpenSans-Regular.ttf", 32);
    fontS  = TTF_OpenFont("assets/fonts/OpenSans-Regular.ttf", 24);

    if (is_init)
        return init_result;

    is_init = true;
    
    atexit(time_widget_quit);

    return init_result = true;
}

time_widget *time_widget_create(
    uo_relpoint reldest)
{
    time_widget *widget = calloc(1, sizeof(time_widget));

    render_instr_set_xy(&widget->render_instrs.time, reldest);

    reldest.y.px += 82;
    render_instr_set_xy(&widget->render_instrs.wday, reldest);

    reldest.y.px += 40;
    render_instr_set_xy(&widget->render_instrs.date, reldest);

    widget->thrd = SDL_CreateThread(update_time, "time_widget_thrd", widget);

    return widget;
}

void time_widget_destroy(
    time_widget *widget)
{
    widget->is_close_requested = true;
    SDL_DetachThread(widget->thrd);
    free(widget);
}

bool time_widget_is_render_required(
    time_widget *widget)
{
    return render_instr_is_render_required(&widget->render_instrs.date)
        || render_instr_is_render_required(&widget->render_instrs.wday)
        || render_instr_is_render_required(&widget->render_instrs.time);
}

bool time_widget_render(
    time_widget *widget)
{
    if (widget->is_ready) {
        render_instr_render(&widget->render_instrs.date, renderer);
        render_instr_render(&widget->render_instrs.wday, renderer);
        render_instr_render(&widget->render_instrs.time, renderer);
    }
}

bool time_widget_update_dest(
    time_widget *widget) 
{
    render_instr_reposition(&widget->render_instrs.date);
    render_instr_update(&widget->render_instrs.date);
    render_instr_reposition(&widget->render_instrs.wday);
    render_instr_update(&widget->render_instrs.wday);
    render_instr_reposition(&widget->render_instrs.time);
    render_instr_update(&widget->render_instrs.time);
}