/* fmi WeatherSymbol3
1 selkeää
2 puolipilvistä
21 heikkoja sadekuuroja
22 sadekuuroja
23 voimakkaita sadekuuroja
3 pilvistä
31 heikkoa vesisadetta
32 vesisadetta
33 voimakasta vesisadetta
41 heikkoja lumikuuroja
42 lumikuuroja
43 voimakkaita lumikuuroja
51 heikkoa lumisadetta
52 lumisadetta
53 voimakasta lumisadetta
61 ukkoskuuroja
62 voimakkaita ukkoskuuroja
63 ukkosta
64 voimakasta ukkosta
71 heikkoja räntäkuuroja
72 räntäkuuroja
73 voimakkaita räntäkuuroja
81 heikkoa räntäsadetta
82 räntäsadetta
83 voimakasta räntäsadetta
91 utua
92 sumua
*/

#include "weather_widget.h"

#include "uo_sock.h"

#include "SDL.h"
#include "SDL_timer.h"
#include "SDL_image.h"

#include <stdio.h>

#include <unistd.h>

#define U8_DEG u8"°"
#define U8_DEG_SIZE sizeof(U8_DEG)

static bool is_init;
static bool init_result;

static SDL_Renderer *renderer;

static SDL_Color white = { 255, 255, 255 };

static TTF_Font *fontXL, *fontL, *fontM, *fontS;

static long tz_offset_second(void) 
{
    time_t t = time(NULL);

    struct tm local = *localtime(&t);
    struct tm utc = *gmtime(&t);

    long diff = ((local.tm_hour - utc.tm_hour) 
        * 60 + (local.tm_min - utc.tm_min))
        * 60L + (local.tm_sec - utc.tm_sec);
    int delta_day = local.tm_mday - utc.tm_mday;

    if ((delta_day == 1) || (delta_day < -1))
        diff += 24L * 60 * 60;
    else if ((delta_day == -1) || (delta_day > 1))
        diff -= 24L * 60 * 60;
    
    return diff;
}

static void weather_widget_quit(void)
{
    TTF_CloseFont(fontXL);
    TTF_CloseFont(fontL);
    TTF_CloseFont(fontM);
    TTF_CloseFont(fontS);
}

static int update_weather(
	void *arg)
{
    weather_widget *widget = arg;

	struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP
    }, *weatherserv_addrinfo;

    int s = getaddrinfo("localhost", "12001", &hints, &weatherserv_addrinfo);
    
    if (s != 0) {
        printf("getaddrinfo: %s\n", gai_strerror(s));
        return 0;
    }

	char buf[0x1000];

    int x = widget->x;
    int y = widget->y;
    render_instr_query_output_xy(renderer, widget->dest_origin, &x, &y);

    widget->render_instrs[0].celsius.dest_origin = MIDDLE_LEFT;
    widget->render_instrs[0].symbol.dest_origin = MIDDLE_RIGHT;
    render_instr_set_xy(&widget->render_instrs[0].celsius, x + 36, y);
    render_instr_set_xy(&widget->render_instrs[0].symbol,  x + 20, y);

    for (size_t i = 1; i < sizeof widget->render_instrs / sizeof widget->render_instrs[0]; ++i)
    {
        uint8_t alpha = (255 * (sizeof widget->render_instrs / sizeof widget->render_instrs[0])) * i;
        widget->render_instrs[i].celsius.dest_origin = MIDDLE_LEFT;
        widget->render_instrs[i].symbol.dest_origin = MIDDLE_CENTER;
        widget->render_instrs[i].time.dest_origin = MIDDLE_RIGHT;
        widget->render_instrs[i].celsius.alpha = alpha;
        widget->render_instrs[i].symbol.alpha = alpha;
        widget->render_instrs[i].time.alpha = alpha;
        render_instr_set_xy(&widget->render_instrs[i].celsius, x + 32, y + i * 48 + 28);
        render_instr_set_xy(&widget->render_instrs[i].symbol,  x - 16, y + i * 48 + 28);
        render_instr_set_xy(&widget->render_instrs[i].time,    x - 64, y + i * 48 + 28);
    }

    while (!widget->is_close_requested) {

		int sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

		if (connect(sockfd, weatherserv_addrinfo->ai_addr, weatherserv_addrinfo->ai_addrlen) == -1)
        	return 0;

		ssize_t len = 0;
        char *end = NULL;
        char *p = buf;
        while (!end) {
            p += len = recv(sockfd, p, sizeof buf - (p - buf), 0);
            if (len == -1)
                break; //error TODO: handle
            *p = '\0';
            end = strstr(buf, "READY\r\n");
        }

		size_t place_len = strlen(widget->place);
		p = memcpy(buf, widget->place, place_len) + place_len;
		p = memcpy(p, "\r\n", 2) + 2;

        char *weather = p;

		if (send(sockfd, buf, p - buf, 0) == -1)
			return 0;

		end = NULL;
		while (!end) {
            p += len = recv(sockfd, p, sizeof buf - (p - buf), 0);
            if (len == -1)
                break; //error TODO: handle
            *p = '\0';
            end = strstr(buf, "CLOSING\r\n");
        }

		send(sockfd, "\0", 1, 0);
		close(sockfd);

        double celsius;
        int symbol;
        int hour;

        sscanf(strtok(weather, "\r\n"), "%*d-%*d-%*dT%d:00:00Z,%lf,%d", &hour, &celsius, &symbol);

        hour += tz_offset_second() / 3600;

        int w, h;
        SDL_Surface *surface_celsius, *surface_symbol;

        snprintf(buf, sizeof buf, "%.0lf" U8_DEG, celsius);
        surface_celsius = TTF_RenderUTF8_Blended(fontXL, buf, white);
        TTF_SizeUTF8(fontXL, buf, &w, &h);
        render_instr_set_swh(&widget->render_instrs[0].celsius, surface_celsius, w, h);

        switch (symbol) {
            case 1:  surface_symbol = IMG_Load("assets/fmi/white/1-128.png");  break; // selkeää
            case 2:  surface_symbol = IMG_Load("assets/fmi/white/2-128.png");  break; // puolipilvistä
            case 21: surface_symbol = IMG_Load("assets/fmi/white/21-128.png"); break; // heikkoja sadekuuroja
            case 22: surface_symbol = IMG_Load("assets/fmi/white/24-128.png"); break; // sadekuuroja
            case 23: surface_symbol = IMG_Load("assets/fmi/white/27-128.png"); break; // voimakkaita sadekuuroja
            case 3:  surface_symbol = IMG_Load("assets/fmi/white/7-128.png");  break; // pilvistä
            case 31: surface_symbol = IMG_Load("assets/fmi/white/31-128.png"); break; // heikkoa vesisadetta
            case 32: surface_symbol = IMG_Load("assets/fmi/white/32-128.png"); break; // vesisadetta
            case 33: surface_symbol = IMG_Load("assets/fmi/white/33-128.png"); break; // voimakasta vesisadetta
            case 41: surface_symbol = IMG_Load("assets/fmi/white/41-128.png"); break; // heikkoja lumikuuroja
            case 42: surface_symbol = IMG_Load("assets/fmi/white/42-128.png"); break; // lumikuuroja
            case 43: surface_symbol = IMG_Load("assets/fmi/white/43-128.png"); break; // voimakkaita lumikuuroja
            case 51: surface_symbol = IMG_Load("assets/fmi/white/51-128.png"); break; // heikkoa lumisadetta
            case 52: surface_symbol = IMG_Load("assets/fmi/white/52-128.png"); break; // lumisadetta
            case 53: surface_symbol = IMG_Load("assets/fmi/white/53-128.png"); break; // voimakasta lumisadetta
            case 61: surface_symbol = IMG_Load("assets/fmi/white/71-128.png"); break; // ukkoskuuroja
            case 62: surface_symbol = IMG_Load("assets/fmi/white/74-128.png"); break; // voimakkaita ukkoskuuroja
            case 63: surface_symbol = IMG_Load("assets/fmi/white/77-128.png"); break; // ukkosta
            case 64: surface_symbol = IMG_Load("assets/fmi/white/67-128.png"); break; // voimakasta ukkosta
            case 71: surface_symbol = IMG_Load("assets/fmi/white/71-128.png"); break; // heikkoja räntäkuuroja
            case 72: surface_symbol = IMG_Load("assets/fmi/white/74-128.png"); break; // räntäkuuroja
            case 73: surface_symbol = IMG_Load("assets/fmi/white/77-128.png"); break; // voimakkaita räntäkuuroja
            case 81: surface_symbol = IMG_Load("assets/fmi/white/1-128.png");  break; // heikkoa räntäsadetta
            case 82: surface_symbol = IMG_Load("assets/fmi/white/1-128.png");  break; // räntäsadetta
            case 83: surface_symbol = IMG_Load("assets/fmi/white/1-128.png");  break; // voimakasta räntäsadetta
            case 91: surface_symbol = IMG_Load("assets/fmi/white/1-128.png");  break; // utua
            case 92: surface_symbol = IMG_Load("assets/fmi/white/1-128.png");  break; // sumua
        }

        render_instr_set_swh(&widget->render_instrs[0].symbol, surface_symbol, 0, 0);
        
        render_instr_update(&widget->render_instrs[0].celsius);
        render_instr_update(&widget->render_instrs[0].symbol);

        for (size_t i = 1; i < sizeof widget->render_instrs / sizeof widget->render_instrs[0]; ++i)
        {
            sscanf(strtok(NULL, "\r\n"), "%*d-%*d-%*dT%*d:00:00Z,%lf,%d", &celsius, &symbol);

            snprintf(buf, sizeof buf, "%.0lf" U8_DEG, celsius);
            surface_celsius = TTF_RenderUTF8_Blended(fontS, buf, white);
            TTF_SizeUTF8(fontS, buf, &w, &h);
            render_instr_set_swh(&widget->render_instrs[i].celsius, surface_celsius, w, h);

            snprintf(buf, sizeof buf, "%d", (hour + i) % 24 );
            surface_celsius = TTF_RenderUTF8_Blended(fontS, buf, white);
            TTF_SizeUTF8(fontS, buf, &w, &h);
            render_instr_set_swh(&widget->render_instrs[i].time, surface_celsius, w, h);

            switch (symbol) {
                case 1:  surface_symbol = IMG_Load("assets/fmi/white/1-48.png");  break; // selkeää
                case 2:  surface_symbol = IMG_Load("assets/fmi/white/2-48.png");  break; // puolipilvistä
                case 21: surface_symbol = IMG_Load("assets/fmi/white/21-48.png"); break; // heikkoja sadekuuroja
                case 22: surface_symbol = IMG_Load("assets/fmi/white/24-48.png"); break; // sadekuuroja
                case 23: surface_symbol = IMG_Load("assets/fmi/white/27-48.png"); break; // voimakkaita sadekuuroja
                case 3:  surface_symbol = IMG_Load("assets/fmi/white/7-48.png");  break; // pilvistä
                case 31: surface_symbol = IMG_Load("assets/fmi/white/31-48.png"); break; // heikkoa vesisadetta
                case 32: surface_symbol = IMG_Load("assets/fmi/white/32-48.png"); break; // vesisadetta
                case 33: surface_symbol = IMG_Load("assets/fmi/white/33-48.png"); break; // voimakasta vesisadetta
                case 41: surface_symbol = IMG_Load("assets/fmi/white/41-48.png"); break; // heikkoja lumikuuroja
                case 42: surface_symbol = IMG_Load("assets/fmi/white/42-48.png"); break; // lumikuuroja
                case 43: surface_symbol = IMG_Load("assets/fmi/white/43-48.png"); break; // voimakkaita lumikuuroja
                case 51: surface_symbol = IMG_Load("assets/fmi/white/51-48.png"); break; // heikkoa lumisadetta
                case 52: surface_symbol = IMG_Load("assets/fmi/white/52-48.png"); break; // lumisadetta
                case 53: surface_symbol = IMG_Load("assets/fmi/white/53-48.png"); break; // voimakasta lumisadetta
                case 61: surface_symbol = IMG_Load("assets/fmi/white/71-48.png"); break; // ukkoskuuroja
                case 62: surface_symbol = IMG_Load("assets/fmi/white/74-48.png"); break; // voimakkaita ukkoskuuroja
                case 63: surface_symbol = IMG_Load("assets/fmi/white/77-48.png"); break; // ukkosta
                case 64: surface_symbol = IMG_Load("assets/fmi/white/67-48.png"); break; // voimakasta ukkosta
                case 71: surface_symbol = IMG_Load("assets/fmi/white/71-48.png"); break; // heikkoja räntäkuuroja
                case 72: surface_symbol = IMG_Load("assets/fmi/white/74-48.png"); break; // räntäkuuroja
                case 73: surface_symbol = IMG_Load("assets/fmi/white/77-48.png"); break; // voimakkaita räntäkuuroja
                case 81: surface_symbol = IMG_Load("assets/fmi/white/1-48.png");  break; // heikkoa räntäsadetta
                case 82: surface_symbol = IMG_Load("assets/fmi/white/1-48.png");  break; // räntäsadetta
                case 83: surface_symbol = IMG_Load("assets/fmi/white/1-48.png");  break; // voimakasta räntäsadetta
                case 91: surface_symbol = IMG_Load("assets/fmi/white/1-48.png");  break; // utua
                case 92: surface_symbol = IMG_Load("assets/fmi/white/1-48.png");  break; // sumua
            }

            render_instr_set_swh(&widget->render_instrs[i].symbol, surface_symbol, 0, 0);
            
            render_instr_update(&widget->render_instrs[i].celsius);
            render_instr_update(&widget->render_instrs[i].symbol);
            render_instr_update(&widget->render_instrs[i].time);
        }

        widget->is_ready = true;

        time_t t = time(NULL);
        struct tm *now = localtime(&t);

        SDL_Delay((60 - now->tm_min) * 60000 - now->tm_sec * 1000);
    }
}

bool weather_widget_init(
    SDL_Renderer *renderer_)
{
    uo_sock_init();

    renderer = renderer_;

    fontXL  = TTF_OpenFont("assets/fonts/OpenSans-Regular.ttf", 64);
    fontL   = TTF_OpenFont("assets/fonts/OpenSans-Regular.ttf", 48);
    fontM   = TTF_OpenFont("assets/fonts/OpenSans-Regular.ttf", 32);
    fontS   = TTF_OpenFont("assets/fonts/OpenSans-Regular.ttf", 24);

    if (is_init)
        return init_result;

    is_init = true;

    atexit(weather_widget_quit);

    return init_result = true;
}

weather_widget *weather_widget_create(
    char *place,
    int x, 
    int y,
    render_instr_dest_origin dest_origin)
{
    weather_widget *widget = calloc(1, sizeof(weather_widget));
    size_t place_len = strlen(place);
    widget->place = malloc(place_len + 1);
	memcpy(widget->place, place, place_len + 1);
    widget->x = x;
    widget->y = y;
    widget->dest_origin = dest_origin;

    widget->thrd = SDL_CreateThread(update_weather, "weather_widget_thrd", widget);

    return widget;
}

void weather_widget_destroy(
    weather_widget *widget)
{
    widget->is_close_requested = true;
    SDL_DetachThread(widget->thrd);
    free(widget->place);
    free(widget);
}

bool weather_widget_is_render_required(
    weather_widget *widget)
{
    for (size_t i = 0; i < sizeof widget->render_instrs / sizeof widget->render_instrs[0]; ++i)
        if (widget->render_instrs[i].celsius.flags |
            widget->render_instrs[i].symbol.flags |
            widget->render_instrs[i].time.flags)
            return true;

    return false;
}

bool weather_widget_render(
    weather_widget *widget)
{
    if (widget->is_ready) 
    {
        render_instr_render(&widget->render_instrs[0].celsius, renderer);
        render_instr_render(&widget->render_instrs[0].symbol, renderer);

        for (size_t i = 1; i < sizeof widget->render_instrs / sizeof widget->render_instrs[0]; ++i)
        {
		    render_instr_render(&widget->render_instrs[i].celsius, renderer);
            render_instr_render(&widget->render_instrs[i].symbol, renderer);
            render_instr_render(&widget->render_instrs[i].time, renderer);
        }
    }
}

bool weather_widget_update_dest(
    weather_widget *widget) 
{
    int x = widget->x;
    int y = widget->y;
    render_instr_query_output_xy(renderer, widget->dest_origin, &x, &y);
    render_instr_set_xy(&widget->render_instrs[0].celsius, x + 36, y);
    render_instr_set_xy(&widget->render_instrs[0].symbol,  x + 20, y);
    render_instr_update(&widget->render_instrs[0].celsius);
    render_instr_update(&widget->render_instrs[0].symbol);
    for (size_t i = 1; i < sizeof widget->render_instrs / sizeof widget->render_instrs[0]; ++i)
    {
        render_instr_set_xy(&widget->render_instrs[i].celsius, x + 32, y + i * 48 + 28);
        render_instr_set_xy(&widget->render_instrs[i].symbol,  x - 16, y + i * 48 + 28);
        render_instr_set_xy(&widget->render_instrs[i].time,    x - 64, y + i * 48 + 28);
        render_instr_update(&widget->render_instrs[i].celsius);
        render_instr_update(&widget->render_instrs[i].symbol);
        render_instr_update(&widget->render_instrs[i].time);
    }
}
