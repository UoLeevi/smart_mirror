#include "smart_mirror_conf.h"
#include "weather_widget.h"
#include "time_widget.h"

#include "SDL.h"
#include "SDL_timer.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

static bool is_close_requested;

int main(int argc, char **argv) {

	const uint32_t SCREEN_FPS = 60;
	const uint32_t SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
	uint32_t frame_start_ticks, frame_end_ticks = ~0, frame_ticks;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		printf("Unable to initialize SDL: %s", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	if (TTF_Init() != 0) {
		printf("TTF_Init: %s\n", TTF_GetError());
		return 1;
	}
	atexit(TTF_Quit);

	smart_mirror_conf_t *conf = smart_mirror_conf_create();
	if (!conf) {
		printf("Error occurred while reading the configuration./r/n");
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow(
		"Smart mirror",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640,
        480,
        SDL_WINDOW_OPENGL
    );

    if (!window) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

	SDL_Renderer *renderer = SDL_CreateRenderer(
		window, 
		-1, 
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (!renderer) {
		printf("Could not create renderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		return 1;
	}

	if (!weather_widget_init(renderer)) {
		printf("Could not create initialize weather widget: %s\n", SDL_GetError());
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		return 1;
	};

	if (!time_widget_init(renderer)) {
		printf("Could not create initialize time widget: %s\n", SDL_GetError());
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		return 1;
	};

	weather_widget *weather_w = weather_widget_create(
		conf->weather_widget_conf.place, 
		conf->weather_widget_conf.x, 
		conf->weather_widget_conf.y,
		conf->weather_widget_conf.dest_origin);

	time_widget *time_w = time_widget_create(
		conf->time_widget_conf.x, 
		conf->time_widget_conf.y,
		conf->time_widget_conf.dest_origin);

	while (!is_close_requested) 
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
			switch (event.type) 
			{
				case SDL_QUIT:
					is_close_requested = true;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.scancode) 
					{
						case SDL_SCANCODE_ESCAPE:
							is_close_requested = true;
							break;

						case SDL_SCANCODE_F11: {
							bool is_fullscreen = SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP;
							SDL_ShowCursor(is_fullscreen ? SDL_ENABLE : SDL_DISABLE);
							SDL_SetWindowFullscreen(window, is_fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
							weather_widget_update_dest(weather_w);
							time_widget_update_dest(time_w);
							break;
						}
					}
			}

		if (time_widget_is_render_required(time_w) ||
			weather_widget_is_render_required(weather_w))
		{
			SDL_RenderClear(renderer);

			weather_widget_render(weather_w);
			time_widget_render(time_w);

			SDL_RenderPresent(renderer);
		}

		frame_start_ticks = frame_end_ticks;
		frame_end_ticks = SDL_GetTicks();
		frame_ticks = frame_end_ticks - frame_start_ticks;

		if (frame_ticks < SCREEN_TICKS_PER_FRAME)
			SDL_Delay(SCREEN_TICKS_PER_FRAME - frame_ticks);
	}

	weather_widget_destroy(weather_w);
	time_widget_destroy(time_w);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	return 0;
}