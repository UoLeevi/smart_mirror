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
#include "uo_ipcc.h"
#include "uo_mem.h"

#include "SDL.h"
#include "SDL_timer.h"
#include "SDL_image.h"

#include <stdio.h>
#include <math.h>

#include <unistd.h>

#define U8_DEG u8"°"
#define U8_DEG_SIZE sizeof(U8_DEG)

static bool is_init;
static bool init_result;

static SDL_Renderer *renderer;

static SDL_Color white = { 255, 255, 255 };

static TTF_Font *fontXL, *fontL, *fontM, *fontS;

// http://edwilliams.org/sunrise_sunset_algorithm.htm
static double get_sunrise_or_sunset_hour_utc(
    int year,
    int month,
    int day,
    double latitude,
    double longitude,
    bool is_sunrise)
{
    const double zenith = 90.0 + 50.0 / 60.0;
    const double pi = 3.1415926;

    // 1. first calculate the day of the year
    double N1 = floor(275 * month / 9);
    double N2 = floor((month + 9) / 12);
    double N3 = (1 + floor((year - 4 * floor(year / 4) + 2) / 3));
    double N = N1 - (N2 * N3) + day - 30;

    // 2. convert the longitude to hour value and calculate an approximate time
	double lngHour = longitude / 15;
    double t = is_sunrise
        ? N + ((6 - lngHour) / 24)
	    : N + ((18 - lngHour) / 24);

    // 3. calculate the Sun's mean anomaly
	double M = (0.9856 * t) - 3.289;

    // 4. calculate the Sun's true longitude
    double L = M + (1.916 * sin((pi / 180) * M)) + (0.020 * sin((pi / 180) * 2 * M)) + 282.634;

    if (L >= 360.0)
        L -= 360.0;
    else if (L < 0)
        L += 360.0;

    // 5a. calculate the Sun's right ascension
	double RA = (180 / pi ) * atan(0.91764 * tan((pi / 180) * L));

    if (RA >= 360.0)
        RA -= 360.0;
    else if (L < 0)
        RA += 360.0;

    // 5b. right ascension value needs to be in the same quadrant as L
	double Lquadrant  = (floor( L/90)) * 90;
	double RAquadrant = (floor(RA/90)) * 90;
	RA = RA + (Lquadrant - RAquadrant);

    // 5c. right ascension value needs to be converted into hours
	RA = RA / 15;

    // 6. calculate the Sun's declination
	double sinDec = 0.39782 * sin((pi / 180) * L);
	double cosDec = cos(asin(sinDec));

    // 7a. calculate the Sun's local hour angle
	double cosH = (cos((pi / 180) * zenith) - (sinDec * sin((pi / 180) * latitude))) / (cosDec * cos((pi / 180) * latitude));
	
    if (is_sunrise ? cosH > 1.0 : cosH < -1.0)
        return -1.0;

    // 7b. finish calculating H and convert into hours
    double H = is_sunrise
	    ? 360 - (180 / pi ) * acos(cosH)
        : (180 / pi ) * acos(cosH);
	H = H / 15;

    // 8. calculate local mean time of rising/setting
	double T = H + RA - (0.06571 * t) - 6.622;

    double UT = T - lngHour;

    if (UT >= 24.0)
        UT -= 24.0;
    else if (UT < 0)
        UT += 24.0;

    return UT;
}

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

    uo_ipcc *ipcc = uo_ipcc_create("localhost", 9, "12001", 5);

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

    char buf[0x100] = { 0 };
    uo_ipcmsg msg = { 0 };

    while (!widget->is_close_requested)
    {
        size_t cmd_len = snprintf(buf, sizeof buf, "WEATHER %s", widget->place);
        msg = uo_ipcc_send_msg(ipcc, (uo_ipcmsg) { .data = buf, .data_len = cmd_len }, true);

        char *weather = msg.data;

		if (weather)
        {
            double latitude;
            double longitude;
            double celsius;
            int symbol;
            int year, month, day, hour;
            bool is_daytime;

            char *line = strtok(weather, "\r\n");
            sscanf(line, "%lf,%lf,%d-%d-%dT%d:00:00Z,%lf,%d", &latitude, &longitude, &year, &month, &day, &hour);
            line = strtok(NULL, "\r\n");
            sscanf(line, "%lf,%d", &celsius, &symbol);

            long tz_offset_hours = tz_offset_second() / 3600;

            hour += tz_offset_hours;

            if (hour >= 24)
                hour -= 24;
            else if (hour < 0)
                hour += 24;

            // TODO:
            // Get latitude and longitude based on place
            // Get next sunrise and next sunset, not today's sunrise and sundown
            double sunrise = get_sunrise_or_sunset_hour_utc(year, month, day, latitude, longitude, true);
            double sunset = get_sunrise_or_sunset_hour_utc(year, month, day, latitude, longitude, false);

            if (sunrise != -1.0) 
            {
                sunrise += tz_offset_hours;
                if (sunrise >= 24.0)
                    sunrise -= 24.0;
                else if (sunrise < 0.0)
                    sunrise += 24.0;
            }

            if (sunset != -1.0) 
            {
                sunset += tz_offset_hours;
                if (sunset >= 24.0)
                    sunset -= 24.0;
                else if (sunset < 0.0)
                    sunset += 24.0;
            }

            is_daytime = sunset == -1.0 || hour < sunset && hour > sunrise && sunrise != -1.0;

            int w, h;
            SDL_Surface *surface_celsius, *surface_symbol;

            snprintf(buf, sizeof buf, "%.0lf" U8_DEG, celsius);
            surface_celsius = TTF_RenderUTF8_Blended(fontXL, buf, white);
            TTF_SizeUTF8(fontXL, buf, &w, &h);
            render_instr_set_swh(&widget->render_instrs[0].celsius, surface_celsius, w, h);

            if (is_daytime)
                switch (symbol) 
                {
                    case 1:  surface_symbol = IMG_Load("assets/fmi/white/1-128.png");  break; // selkeää
                    case 2:  surface_symbol = IMG_Load("assets/fmi/white/4-128.png");  break; // puolipilvistä
                    case 21: surface_symbol = IMG_Load("assets/fmi/white/21-128.png"); break; // heikkoja sadekuuroja
                    case 22: surface_symbol = IMG_Load("assets/fmi/white/24-128.png"); break; // sadekuuroja
                    case 23: surface_symbol = IMG_Load("assets/fmi/white/27-128.png"); break; // voimakkaita sadekuuroja
                    case 3:  surface_symbol = IMG_Load("assets/fmi/white/7-128.png");  break; // pilvistä
                    case 31: surface_symbol = IMG_Load("assets/fmi/white/37-128.png"); break; // heikkoa vesisadetta
                    case 32: surface_symbol = IMG_Load("assets/fmi/white/38-128.png"); break; // vesisadetta
                    case 33: surface_symbol = IMG_Load("assets/fmi/white/39-128.png"); break; // voimakasta vesisadetta
                    case 41: surface_symbol = IMG_Load("assets/fmi/white/54-128.png"); break; // heikkoja lumikuuroja
                    case 42: surface_symbol = IMG_Load("assets/fmi/white/55-128.png"); break; // lumikuuroja
                    case 43: surface_symbol = IMG_Load("assets/fmi/white/56-128.png"); break; // voimakkaita lumikuuroja
                    case 51: surface_symbol = IMG_Load("assets/fmi/white/57-128.png"); break; // heikkoa lumisadetta
                    case 52: surface_symbol = IMG_Load("assets/fmi/white/58-128.png"); break; // lumisadetta
                    case 53: surface_symbol = IMG_Load("assets/fmi/white/59-128.png"); break; // voimakasta lumisadetta
                    case 61: surface_symbol = IMG_Load("assets/fmi/white/71-128.png"); break; // ukkoskuuroja
                    case 62: surface_symbol = IMG_Load("assets/fmi/white/74-128.png"); break; // voimakkaita ukkoskuuroja
                    case 63: surface_symbol = IMG_Load("assets/fmi/white/77-128.png"); break; // ukkosta
                    case 64: surface_symbol = IMG_Load("assets/fmi/white/67-128.png"); break; // voimakasta ukkosta
                    case 71: surface_symbol = IMG_Load("assets/fmi/white/44-128.png"); break; // heikkoja räntäkuuroja
                    case 72: surface_symbol = IMG_Load("assets/fmi/white/45-128.png"); break; // räntäkuuroja
                    case 73: surface_symbol = IMG_Load("assets/fmi/white/46-128.png"); break; // voimakkaita räntäkuuroja
                    case 81: surface_symbol = IMG_Load("assets/fmi/white/47-128.png"); break; // heikkoa räntäsadetta
                    case 82: surface_symbol = IMG_Load("assets/fmi/white/48-128.png"); break; // räntäsadetta
                    case 83: surface_symbol = IMG_Load("assets/fmi/white/49-128.png"); break; // voimakasta räntäsadetta
                    case 91: surface_symbol = IMG_Load("assets/fmi/white/9-128.png");  break; // utua
                    case 92: surface_symbol = IMG_Load("assets/fmi/white/9-128.png");  break; // sumua
                }
            else
                switch (symbol) 
                {
                    case 1:  surface_symbol = IMG_Load("assets/fmi/white/101-128.png"); break; // selkeää
                    case 2:  surface_symbol = IMG_Load("assets/fmi/white/104-128.png"); break; // puolipilvistä
                    case 21: surface_symbol = IMG_Load("assets/fmi/white/121-128.png"); break; // heikkoja sadekuuroja
                    case 22: surface_symbol = IMG_Load("assets/fmi/white/124-128.png"); break; // sadekuuroja
                    case 23: surface_symbol = IMG_Load("assets/fmi/white/127-128.png"); break; // voimakkaita sadekuuroja
                    case 3:  surface_symbol = IMG_Load("assets/fmi/white/107-128.png"); break; // pilvistä
                    case 31: surface_symbol = IMG_Load("assets/fmi/white/137-128.png"); break; // heikkoa vesisadetta
                    case 32: surface_symbol = IMG_Load("assets/fmi/white/138-128.png"); break; // vesisadetta
                    case 33: surface_symbol = IMG_Load("assets/fmi/white/139-128.png"); break; // voimakasta vesisadetta
                    case 41: surface_symbol = IMG_Load("assets/fmi/white/154-128.png"); break; // heikkoja lumikuuroja
                    case 42: surface_symbol = IMG_Load("assets/fmi/white/155-128.png"); break; // lumikuuroja
                    case 43: surface_symbol = IMG_Load("assets/fmi/white/156-128.png"); break; // voimakkaita lumikuuroja
                    case 51: surface_symbol = IMG_Load("assets/fmi/white/157-128.png"); break; // heikkoa lumisadetta
                    case 52: surface_symbol = IMG_Load("assets/fmi/white/158-128.png"); break; // lumisadetta
                    case 53: surface_symbol = IMG_Load("assets/fmi/white/159-128.png"); break; // voimakasta lumisadetta
                    case 61: surface_symbol = IMG_Load("assets/fmi/white/171-128.png"); break; // ukkoskuuroja
                    case 62: surface_symbol = IMG_Load("assets/fmi/white/174-128.png"); break; // voimakkaita ukkoskuuroja
                    case 63: surface_symbol = IMG_Load("assets/fmi/white/177-128.png"); break; // ukkosta
                    case 64: surface_symbol = IMG_Load("assets/fmi/white/167-128.png"); break; // voimakasta ukkosta
                    case 71: surface_symbol = IMG_Load("assets/fmi/white/144-128.png"); break; // heikkoja räntäkuuroja
                    case 72: surface_symbol = IMG_Load("assets/fmi/white/145-128.png"); break; // räntäkuuroja
                    case 73: surface_symbol = IMG_Load("assets/fmi/white/146-128.png"); break; // voimakkaita räntäkuuroja
                    case 81: surface_symbol = IMG_Load("assets/fmi/white/147-128.png"); break; // heikkoa räntäsadetta
                    case 82: surface_symbol = IMG_Load("assets/fmi/white/148-128.png"); break; // räntäsadetta
                    case 83: surface_symbol = IMG_Load("assets/fmi/white/149-128.png"); break; // voimakasta räntäsadetta
                    case 91: surface_symbol = IMG_Load("assets/fmi/white/109-128.png"); break; // utua
                    case 92: surface_symbol = IMG_Load("assets/fmi/white/109-128.png"); break; // sumua
                }

            render_instr_set_swh(&widget->render_instrs[0].symbol, surface_symbol, 0, 0);
            
            render_instr_update(&widget->render_instrs[0].celsius);
            render_instr_update(&widget->render_instrs[0].symbol);

            for (size_t i = 1; i < sizeof widget->render_instrs / sizeof widget->render_instrs[0]; ++i)
            {
                is_daytime = sunset == -1.0 || (hour + i) % 24 < sunset && (hour + i) % 24 > sunrise && sunrise != -1.0;

                line = strtok(NULL, "\r\n");
                sscanf(line, "%lf,%d", &celsius, &symbol);

                snprintf(buf, sizeof buf, "%.0lf" U8_DEG, celsius);
                surface_celsius = TTF_RenderUTF8_Blended(fontS, buf, white);
                TTF_SizeUTF8(fontS, buf, &w, &h);
                render_instr_set_swh(&widget->render_instrs[i].celsius, surface_celsius, w, h);

                snprintf(buf, sizeof buf, "%d", (hour + i) % 24 );
                surface_celsius = TTF_RenderUTF8_Blended(fontS, buf, white);
                TTF_SizeUTF8(fontS, buf, &w, &h);
                render_instr_set_swh(&widget->render_instrs[i].time, surface_celsius, w, h);

                if (is_daytime)
                    switch (symbol) 
                    {
                        case 1:  surface_symbol = IMG_Load("assets/fmi/white/1-48.png");  break; // selkeää
                        case 2:  surface_symbol = IMG_Load("assets/fmi/white/4-48.png");  break; // puolipilvistä
                        case 21: surface_symbol = IMG_Load("assets/fmi/white/21-48.png"); break; // heikkoja sadekuuroja
                        case 22: surface_symbol = IMG_Load("assets/fmi/white/24-48.png"); break; // sadekuuroja
                        case 23: surface_symbol = IMG_Load("assets/fmi/white/27-48.png"); break; // voimakkaita sadekuuroja
                        case 3:  surface_symbol = IMG_Load("assets/fmi/white/7-48.png");  break; // pilvistä
                        case 31: surface_symbol = IMG_Load("assets/fmi/white/37-48.png"); break; // heikkoa vesisadetta
                        case 32: surface_symbol = IMG_Load("assets/fmi/white/38-48.png"); break; // vesisadetta
                        case 33: surface_symbol = IMG_Load("assets/fmi/white/39-48.png"); break; // voimakasta vesisadetta
                        case 41: surface_symbol = IMG_Load("assets/fmi/white/54-48.png"); break; // heikkoja lumikuuroja
                        case 42: surface_symbol = IMG_Load("assets/fmi/white/55-48.png"); break; // lumikuuroja
                        case 43: surface_symbol = IMG_Load("assets/fmi/white/56-48.png"); break; // voimakkaita lumikuuroja
                        case 51: surface_symbol = IMG_Load("assets/fmi/white/57-48.png"); break; // heikkoa lumisadetta
                        case 52: surface_symbol = IMG_Load("assets/fmi/white/58-48.png"); break; // lumisadetta
                        case 53: surface_symbol = IMG_Load("assets/fmi/white/59-48.png"); break; // voimakasta lumisadetta
                        case 61: surface_symbol = IMG_Load("assets/fmi/white/71-48.png"); break; // ukkoskuuroja
                        case 62: surface_symbol = IMG_Load("assets/fmi/white/74-48.png"); break; // voimakkaita ukkoskuuroja
                        case 63: surface_symbol = IMG_Load("assets/fmi/white/77-48.png"); break; // ukkosta
                        case 64: surface_symbol = IMG_Load("assets/fmi/white/67-48.png"); break; // voimakasta ukkosta
                        case 71: surface_symbol = IMG_Load("assets/fmi/white/44-48.png"); break; // heikkoja räntäkuuroja
                        case 72: surface_symbol = IMG_Load("assets/fmi/white/45-48.png"); break; // räntäkuuroja
                        case 73: surface_symbol = IMG_Load("assets/fmi/white/46-48.png"); break; // voimakkaita räntäkuuroja
                        case 81: surface_symbol = IMG_Load("assets/fmi/white/47-48.png");  break; // heikkoa räntäsadetta
                        case 82: surface_symbol = IMG_Load("assets/fmi/white/48-48.png");  break; // räntäsadetta
                        case 83: surface_symbol = IMG_Load("assets/fmi/white/49-48.png");  break; // voimakasta räntäsadetta
                        case 91: surface_symbol = IMG_Load("assets/fmi/white/9-48.png");  break; // utua
                        case 92: surface_symbol = IMG_Load("assets/fmi/white/9-48.png");  break; // sumua
                    }
                else
                    switch (symbol) 
                    {
                        case 1:  surface_symbol = IMG_Load("assets/fmi/white/101-48.png");  break; // selkeää
                        case 2:  surface_symbol = IMG_Load("assets/fmi/white/104-48.png");  break; // puolipilvistä
                        case 21: surface_symbol = IMG_Load("assets/fmi/white/121-48.png"); break; // heikkoja sadekuuroja
                        case 22: surface_symbol = IMG_Load("assets/fmi/white/124-48.png"); break; // sadekuuroja
                        case 23: surface_symbol = IMG_Load("assets/fmi/white/127-48.png"); break; // voimakkaita sadekuuroja
                        case 3:  surface_symbol = IMG_Load("assets/fmi/white/107-48.png");  break; // pilvistä
                        case 31: surface_symbol = IMG_Load("assets/fmi/white/137-48.png"); break; // heikkoa vesisadetta
                        case 32: surface_symbol = IMG_Load("assets/fmi/white/138-48.png"); break; // vesisadetta
                        case 33: surface_symbol = IMG_Load("assets/fmi/white/139-48.png"); break; // voimakasta vesisadetta
                        case 41: surface_symbol = IMG_Load("assets/fmi/white/154-48.png"); break; // heikkoja lumikuuroja
                        case 42: surface_symbol = IMG_Load("assets/fmi/white/155-48.png"); break; // lumikuuroja
                        case 43: surface_symbol = IMG_Load("assets/fmi/white/156-48.png"); break; // voimakkaita lumikuuroja
                        case 51: surface_symbol = IMG_Load("assets/fmi/white/157-48.png"); break; // heikkoa lumisadetta
                        case 52: surface_symbol = IMG_Load("assets/fmi/white/158-48.png"); break; // lumisadetta
                        case 53: surface_symbol = IMG_Load("assets/fmi/white/159-48.png"); break; // voimakasta lumisadetta
                        case 61: surface_symbol = IMG_Load("assets/fmi/white/171-48.png"); break; // ukkoskuuroja
                        case 62: surface_symbol = IMG_Load("assets/fmi/white/174-48.png"); break; // voimakkaita ukkoskuuroja
                        case 63: surface_symbol = IMG_Load("assets/fmi/white/177-48.png"); break; // ukkosta
                        case 64: surface_symbol = IMG_Load("assets/fmi/white/167-48.png"); break; // voimakasta ukkosta
                        case 71: surface_symbol = IMG_Load("assets/fmi/white/144-48.png"); break; // heikkoja räntäkuuroja
                        case 72: surface_symbol = IMG_Load("assets/fmi/white/145-48.png"); break; // räntäkuuroja
                        case 73: surface_symbol = IMG_Load("assets/fmi/white/146-48.png"); break; // voimakkaita räntäkuuroja
                        case 81: surface_symbol = IMG_Load("assets/fmi/white/147-48.png");  break; // heikkoa räntäsadetta
                        case 82: surface_symbol = IMG_Load("assets/fmi/white/148-48.png");  break; // räntäsadetta
                        case 83: surface_symbol = IMG_Load("assets/fmi/white/149-48.png");  break; // voimakasta räntäsadetta
                        case 91: surface_symbol = IMG_Load("assets/fmi/white/109-48.png");  break; // utua
                        case 92: surface_symbol = IMG_Load("assets/fmi/white/109-48.png");  break; // sumua
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
        else
            SDL_Delay(10 * 60000);
    }
}

bool weather_widget_init(
    SDL_Renderer *renderer_)
{
    uo_sock_init();
    uo_ipc_init();

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
