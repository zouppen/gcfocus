// Main module for gcfocus
// SPDX-License-Identifier:   GPL-3.0-or-later
// Copyright (C) 2022 Joel Lehtonen
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <err.h>
#include <errno.h>
#include <glib.h>
#include "camera.h"
#include "log_reader.h"
#include "gcode.h"

static gchar *from_bool(gboolean x);
static gdouble convert_simple(gdouble x);

static gchar *camera_dev = NULL;
static gchar *log_file = NULL;
static gboolean log_truncate = FALSE;
static gdouble cal_a = NAN;
static gdouble cal_b = NAN;
static gdouble cal_c = NAN;
gboolean verbose = FALSE;

static GOptionEntry entries[] =
{
	{ "device", 'd', 0, G_OPTION_ARG_FILENAME, &camera_dev, "Device path to V4L2 compliant camera. Required.", "PATH"},
	{ "log", 'f', 0, G_OPTION_ARG_FILENAME, &log_file, "Octoprint serial log file to read. Required.", "PATH"},
	{ "truncate", 't', 0, G_OPTION_ARG_NONE, &log_truncate, "Truncate input log periodically to save space. Default: no truncate", NULL},
	{ "a", 'a', 0, G_OPTION_ARG_DOUBLE, &cal_a, "Calibration constant a. Required.", "NUMBER"},
	{ "b", 'b', 0, G_OPTION_ARG_DOUBLE, &cal_b, "Calibration constant b. Required.", "NUMBER"},
	{ "c", 'c', 0, G_OPTION_ARG_DOUBLE, &cal_c, "Calibration constant c. Required.", "NUMBER"},
	{ "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output. Report conversions and camera operations.", NULL},
	{ NULL }
};

int main(int argc, char **argv)
{
	g_autoptr(GError) error = NULL;
	g_autoptr(GOptionContext) context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, entries, NULL);
	g_option_context_set_summary(context, "Focus 3D printer or CNC machine camera based on X-axis position.");
	g_option_context_set_description(context,
					 "See calibration.md for instructions how to find constants a, b, and c.\n"
					 "");

	if (!g_option_context_parse(context, &argc, &argv, &error))
	{
		errx(1, "Option parsing failed: %s", error->message);
	}

	// Options validation
	if (argc != 1) {
		errx(1, "This command only expects flags, not arguments. See %s --help", argv[0]);
	}

	if (isnan(cal_a) || isnan(cal_b) || isnan(cal_c)) {
		errx(1, "All calibration constants must be given. See %s --help", argv[0]);
	}

	if (camera_dev == NULL) {
		errx(1, "Camera device path must be given. See %s --help", argv[0]);
	}

	if (log_file == NULL) {
		errx(1, "Octoprint log file location must be given. See %s --help", argv[0]);
	}

	// Report startup variables
	if (verbose) {
		printf("Camera device path: %s\n"
		       "Octoprint log path: %s\n"
		       "Truncating logs: %s\n"
		       "Calibration formula: f = %f/(x%+f)%+f\n",
		       camera_dev,
		       log_file,
		       from_bool(log_truncate),
		       cal_a,
		       cal_b,
		       cal_c);
	}

	camera_t camera = camera_init(camera_dev);
	log_reader_t reader = log_reader_init(log_file);
	gcode_t gcode_state = gcode_init();
	int last_focus = -1;

	// Run this loop forever, once per incoming line chunk.
	while (TRUE) {
		gchar *line;
		// Get lines
		while ((line = log_reader_try_getline(&reader)) != NULL) {
			if (!gcode_parse_octoprint_line(&gcode_state, line)) {
				if (verbose) {
					printf("Skipping garbage: %s...\n", line);
				}
			}
		}

		// Now we have got all input for this time, now it's
		// time for lens adjustment.
		if (verbose) {
			printf("Coord: %f, absolute: %s\n", gcode_state.x_pos, from_bool(gcode_state.is_absolute));
		}

		// Convert x position to lens focus value
		double focus = convert_simple(gcode_state.x_pos);
		if (isnan(focus)) {
			if (verbose) {
				printf("Still confused about current coordinates\n");
			}
		} else {
			int new_focus = focus;
			if (new_focus != last_focus) {
				// Adjust focus now
				if (verbose) {
					printf("Adjusting focus to: %d\n", new_focus);
				}

				camera_set_focus(&camera, new_focus);
				last_focus = new_focus;
			}
		}

		// Wait for more
		if (!log_reader_wait(&reader)) {
			warnx("Log file was rotated or deleted, reopening");
			// Give some magic seconds time to create a new file
			sleep(3);
			log_reader_reopen(&reader);
		}
	}
	
	return 0;
}

// Returns just string constants, no dynamic strings are allocated.
static gchar *from_bool(gboolean x)
{
	return x ? "on" : "off";
}

// Does the conversion described in calibration.md 
static gdouble convert_simple(gdouble x) {
	return cal_a / (x + cal_b) + cal_c;
}
