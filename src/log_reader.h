#pragma once

typedef struct {
	int inotify_fd;
	int watch_fd;
} log_reader_t;

// Initializes log reader or dies with an error message
log_reader_t init_log_reader(gchar *file);

// Wait log change. Returns TRUE if we got anything, FALSE if the file
// faded away. Dies in case of inotify inconsistency.
gboolean wait_log_change(log_reader_t *reader);
