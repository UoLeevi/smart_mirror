#include "smart_mirror_conf.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

static const char path_separator =
#ifdef WIN32
    '\\';
#else
    '/';
#endif

typedef enum conf_token_t {
    NONE = 0,
    WEATHER_WIDGET,
    TIME_WIDGET
} conf_token_t;

conf_token_t conf_token_parse(
    char *token);

smart_mirror_conf_t *smart_mirror_conf_create() 
{
    struct stat sb;
    if (stat(SMART_MIRROR_CONF_FILENAME, &sb) == -1) {
        // error getting file stats
        return NULL;
    }

    FILE *fp = fopen(SMART_MIRROR_CONF_FILENAME, "rb");
    if (!fp)
        // error opening the conf file
        return NULL;

    char *bytes = malloc(sb.st_size + 1);
    bytes[sb.st_size] = '\0'; // strtok expects null terminated string

	size_t t = fread(bytes, sizeof(char), sb.st_size, fp);

    if (t != sb.st_size || ferror(fp)) {
        // error reading the file
        fclose(fp);
        return NULL;
    }

    fclose(fp);

    smart_mirror_conf_t *smart_mirror_conf = malloc(sizeof(smart_mirror_conf_t));
    memset(smart_mirror_conf, 0, sizeof(smart_mirror_conf_t));

    const char *delim = "\r\n\t ";
    char *token = strtok(bytes, delim);
        
    while (token) {
        conf_token_t conf_token = conf_token_parse(token);

        if (!conf_token) {
            free(smart_mirror_conf);
            return NULL;
        }

        switch (conf_token) {

			case TIME_WIDGET: {
                char* endptr;
                smart_mirror_conf->time_widget_conf.x = strtol(strtok(NULL, delim), &endptr, 10);
                smart_mirror_conf->time_widget_conf.y = strtol(strtok(NULL, delim), &endptr, 10);
                RENDER_INSTR_DEST_ORIGIN_FROM_STR(smart_mirror_conf->time_widget_conf.dest_origin, strtok(NULL, delim));
                break;
			}

            case WEATHER_WIDGET: {
                char* endptr;
                smart_mirror_conf->weather_widget_conf.x = strtol(strtok(NULL, delim), &endptr, 10);
                smart_mirror_conf->weather_widget_conf.y = strtol(strtok(NULL, delim), &endptr, 10);
                RENDER_INSTR_DEST_ORIGIN_FROM_STR(smart_mirror_conf->time_widget_conf.dest_origin, strtok(NULL, delim));
                char *weather_place = strtok(NULL, delim);
                size_t weather_place_len = strlen(weather_place);
				smart_mirror_conf->weather_widget_conf.place = malloc(weather_place_len + 1);
                memcpy(smart_mirror_conf->weather_widget_conf.place, weather_place, weather_place_len + 1);
                break;
			}

            default:

                break;
            
        }

        token = strtok(NULL, delim);
    }

	free(bytes);

    return smart_mirror_conf;
}

void smart_mirror_conf_delete(smart_mirror_conf_t *smart_mirror_conf) {
    free(smart_mirror_conf);
}

conf_token_t conf_token_parse(char *token) {

    if (!token)
        return NONE;

    if (strcmp(token, "weather_widget") == 0)
        return WEATHER_WIDGET;

    if (strcmp(token, "time_widget") == 0)
        return TIME_WIDGET;

    else
        // unrecognized token
        return NONE;
}