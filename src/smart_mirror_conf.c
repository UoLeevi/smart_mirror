#include "smart_mirror_conf.h"
#include "uo_conf.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum conf_token_t {
    NONE = 0,
    WEATHER_WIDGET,
    TIME_WIDGET,
    MSG_WIDGET
} conf_token_t;

conf_token_t conf_token_parse(
    char *token);

smart_mirror_conf_t *smart_mirror_conf_create() 
{
    uo_conf *conf = uo_conf_create("smart_mirror.conf");

    smart_mirror_conf_t *smart_mirror_conf = malloc(sizeof(smart_mirror_conf_t));
    memset(smart_mirror_conf, 0, sizeof(smart_mirror_conf_t));

    char *endptr, *val;

    if (val = uo_conf_get(conf, "msg_widget.x.px"))
        smart_mirror_conf->msg_widget_conf.reldest.x.px = strtol(val, &endptr, 10);
    if (val = uo_conf_get(conf, "msg_widget.y.px"))
        smart_mirror_conf->msg_widget_conf.reldest.y.px = strtol(val, &endptr, 10);
    if (val = uo_conf_get(conf, "msg_widget.x.pct_screen"))
        smart_mirror_conf->msg_widget_conf.reldest.x.pct_screen = strtol(val, &endptr, 10);
    if (val = uo_conf_get(conf, "msg_widget.y.pct_screen"))
        smart_mirror_conf->msg_widget_conf.reldest.y.pct_screen = strtol(val, &endptr, 10);
    if (val = uo_conf_get(conf, "msg_widget.x.pct_self"))
        smart_mirror_conf->msg_widget_conf.reldest.x.pct_self = strtol(val, &endptr, 10);
    if (val = uo_conf_get(conf, "msg_widget.y.pct_self"))
        smart_mirror_conf->msg_widget_conf.reldest.y.pct_self = strtol(val, &endptr, 10);

    if (val = uo_conf_get(conf, "time_widget.x.px"))
        smart_mirror_conf->time_widget_conf.reldest.x.px = strtol(val, &endptr, 10);
    if (val = uo_conf_get(conf, "time_widget.y.px"))
        smart_mirror_conf->time_widget_conf.reldest.y.px = strtol(val, &endptr, 10);
    if (val = uo_conf_get(conf, "time_widget.x.pct_screen"))
        smart_mirror_conf->time_widget_conf.reldest.x.pct_screen = strtol(val, &endptr, 10);
    if (val = uo_conf_get(conf, "time_widget.y.pct_screen"))
        smart_mirror_conf->time_widget_conf.reldest.y.pct_screen = strtol(val, &endptr, 10);
    if (val = uo_conf_get(conf, "time_widget.x.pct_self"))
        smart_mirror_conf->time_widget_conf.reldest.x.pct_self = strtol(val, &endptr, 10);
    if (val = uo_conf_get(conf, "time_widget.y.pct_self"))
        smart_mirror_conf->time_widget_conf.reldest.y.pct_self = strtol(val, &endptr, 10);

    if (val = uo_conf_get(conf, "weather_widget.x.px"))
        smart_mirror_conf->weather_widget_conf.reldest.x.px = strtol(val, &endptr, 10);
    if (val = uo_conf_get(conf, "weather_widget.y.px"))
        smart_mirror_conf->weather_widget_conf.reldest.y.px = strtol(val, &endptr, 10);
    if (val = uo_conf_get(conf, "weather_widget.x.pct_screen"))
        smart_mirror_conf->weather_widget_conf.reldest.x.pct_screen = strtol(val, &endptr, 10);
    if (val = uo_conf_get(conf, "weather_widget.y.pct_screen"))
        smart_mirror_conf->weather_widget_conf.reldest.y.pct_screen = strtol(val, &endptr, 10);
    if (val = uo_conf_get(conf, "weather_widget.x.pct_self"))
        smart_mirror_conf->weather_widget_conf.reldest.x.pct_self = strtol(val, &endptr, 10);
    if (val = uo_conf_get(conf, "weather_widget.y.pct_self"))
        smart_mirror_conf->weather_widget_conf.reldest.y.pct_self = strtol(val, &endptr, 10);
 
    char *weather_place = uo_conf_get(conf, "weather_widget.place");
    size_t weather_place_len = strlen(weather_place);
    smart_mirror_conf->weather_widget_conf.place = malloc(weather_place_len + 1);
    memcpy(smart_mirror_conf->weather_widget_conf.place, weather_place, weather_place_len + 1);

    uo_conf_destroy(conf);

    return smart_mirror_conf;
}

void smart_mirror_conf_delete(smart_mirror_conf_t *smart_mirror_conf) {
    free(smart_mirror_conf);
}
