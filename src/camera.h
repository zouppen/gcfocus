#pragma once

#include <glib.h>

typedef struct {
	int fd;
} camera_t;

// Initialize camera and set it to manual focus mode
camera_t camera_init(gchar *dev);

// Set focus to given value
void camera_set_focus(camera_t *cam, int focus);
