#pragma once

#include <glib.h>

typedef struct {
	gboolean is_absolute;
	gdouble x_pos;
} gcode_t;

// Initialize gcode structure
gcode_t gcode_init(void);

// Parses Octoprint serial log line. Alters the line content because
// it is processed with strsep(3).
gboolean gcode_parse_octoprint_line(gcode_t *state, gchar *buf);
