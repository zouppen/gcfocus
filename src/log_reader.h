#pragma once

typedef struct {
	char *filename; // Allocated buffer
	int inotify_fd;
	int watch_fd;
	FILE *log;
	char line_buf[256]; // Longer lines than this are garbage anyway
	int line_pos;
} log_reader_t;

// Initializes log reader or dies with an error message
log_reader_t log_reader_init(gchar *file);

// Wait log change. Returns TRUE if we got anything, FALSE if the file
// faded away. Dies in case of inotify inconsistency.
gboolean log_reader_wait(log_reader_t *reader);

// Try to get a line or NULL if we need to wait for more data
gchar *log_reader_try_getline(log_reader_t *reader);
