#ifndef SMART_MIRROR_CONF_H
#define SMART_MIRROR_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "render_instr.h"

#include "SDL.h"

#include <stddef.h>

#define SMART_MIRROR_CONF_FILENAME "smart_mirror.conf"

typedef struct smart_mirror_conf_t {
	struct {
		int x, y;
		render_instr_dest_origin dest_origin;
		char *place;
	} weather_widget_conf;
	struct {
		int x, y;
		render_instr_dest_origin dest_origin;
	} time_widget_conf;
	struct {
		int x, y;
		render_instr_dest_origin dest_origin;
	} msg_widget_conf;
} smart_mirror_conf_t;

smart_mirror_conf_t *smart_mirror_conf_create(void);

void smart_mirror_conf_delete(
	smart_mirror_conf_t *);

#ifdef __cplusplus
}
#endif

#endif