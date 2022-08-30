// Very simple gcode parser for gcfocus
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
#include <string.h>
#include <math.h>
#include "gcode.h"

// Helper for extracting the number out of a token
static void extract_x_coord(gcode_t *state, gchar *buf);

gcode_t gcode_init(void) {
	gcode_t ret;
	ret.is_absolute = TRUE;
	ret.x_pos = NAN;
	return ret;
}

gboolean gcode_parse_octoprint_line(gcode_t *state, gchar *buf)
{
	// Process only outgoing lines
	char *gcode = strstr(buf, " Send: ");
	if (gcode == NULL) {
		// Report error only if it is not receive
		char *recv = strstr(buf, " Recv: ");
		return recv != NULL;
	}
	gcode += 7; // Length of the needle in strstr()

	// First, check the command
	char *command = strsep(&gcode, " ");
	if (command != NULL && *command == 'N') {
		// Skip command number if it's there
		command = strsep(&gcode, " ");
	}

	if (command == NULL) {
		return FALSE;
	} else if (strcmp(command, "G90") == 0) {
		// Absulute positioning
		state->is_absolute = TRUE;
	} else if (strcmp(command, "G91") == 0) {
		// Relative positioning
		state->is_absolute = FALSE;
 	} else if (strcmp(command, "G91") == 0) {
		// Relative positioning
		state->is_absolute = FALSE;
 	} else if (strcmp(command, "G0") == 0 ||
		   strcmp(command, "G00") == 0 ||
		   strcmp(command, "G1") == 0 ||
		   strcmp(command, "G01") == 0) {
		// Linear move. We want to get the X coordinate out
		extract_x_coord(state, gcode);
	} else {
		// It is probably Octoprint line but not interesting
		// for us.
		return TRUE;
	}
	return TRUE;
}

static void extract_x_coord(gcode_t *state, gchar *buf)
{
	while (TRUE) {
		char *token = strsep(&buf, " ");

		// Stop if we reached the end
		if (token == NULL) break;

		// Skip if not X coordinate
		if (*token != 'X') continue;

		// Extracting the number
		char *endptr;
		if (token[1] == '\0') {
			// Nothing after X, safest to stop
			break;
		}
		double move = strtod(token+1, &endptr);
		if (*endptr != '\0') {
			// Parse error, safest to stop
			continue;
		}

		// Now recording the actual move
		if (state->is_absolute) {
			state->x_pos = move;
		} else {
			state->x_pos += move;
		}
	}
}
