// Log file reader module for gcfocus
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

#include <stdio.h>
#include <err.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <glib.h>
#include "log_reader.h"

log_reader_t init_log_reader(gchar *file)
{
	log_reader_t ret;

	// Opening the file first to avoid triggering event for it
	ret.log = fopen(file, "rb");
	if (ret.log == NULL) {
		err(2, "Unable to open log file %s", file);
	}

	ret.inotify_fd = inotify_init();
	if (ret.inotify_fd == -1) {
		err(2, "Unable to initialize inotify");
	}

	ret.watch_fd = inotify_add_watch(ret.inotify_fd, file,
					 IN_MODIFY |
					 IN_DELETE_SELF |
					 IN_MOVE_SELF);
	if (ret.watch_fd == -1) {
		err(2, "Unable to watch %s", file);
	}

	// Buffer is empty initially
	ret.line_pos = 0;

	return ret;
}

gboolean wait_log_change(log_reader_t *reader)
{
	// Fetch the event without file name
	struct inotify_event event;
	int got = read(reader->inotify_fd, &event, sizeof(event));
	if (got == -1) {
		err(2, "Log file watch failed");
	}
	if (got != sizeof(event)) {
		errx(2, "Log file watch failed, inotify gave %d bytes instead of %lud", got, sizeof(event));
	}

	// Getting a file name if it's there. Creates extra syscall
	// but those events are rare in this application so not a
	// bottleneck. File names are allocated from stack since
	// they're rather small anyway.
	char filename[event.len];
	if (event.len != 0) {
		if (read(reader->inotify_fd, filename, sizeof(filename)) != sizeof(filename)) {
			errx(2, "Filename data from inotify is not consistent");
		}
	}

	switch (event.mask) {
	case IN_MODIFY:
		// This is what we've been waiting for
		return TRUE;
	case IN_MOVE_SELF:
	case IN_DELETE_SELF:
		// File was probably rotated
		return FALSE;
	default:
		err(2, "Inotify gave inconsistent event %d", event.mask);
	}
}

gchar *try_getline(log_reader_t *reader)
{
 omstart:
	while (reader->line_pos < sizeof(reader->line_buf)) {
		int c = fgetc(reader->log);
		switch (c) {
		case EOF:
			// Gotta continue later
			clearerr(reader->log);
			return NULL;
		case '\n':
			// Terminate and return the string and rollback buffer
			reader->line_buf[reader->line_pos] = '\0';
			reader->line_pos = 0;
			return reader->line_buf;
		default:
			// Store char
			reader->line_buf[reader->line_pos++] = c;
		}
	}

	// OK, we have overflown, ignore garbage until we get a new line
	while (TRUE) {
		int c = fgetc(reader->log);
		switch (c) {
		case EOF:
			// Gotta continue later
			clearerr(reader->log);
			return NULL;
		case '\n':
			// Finally! Rollbacking the buffer.
			if (TRUE) {
				printf("Ignored a too long input line\n");
			}
			reader->line_pos = 0;

			// Maybe tail recursion detection is working on gcc but
			// doing still old fashioned way
			goto omstart;
		default:
			// No action
		}
	}
}
