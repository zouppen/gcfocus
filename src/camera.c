// Camera focus tools gcfocus
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <err.h>
#include <linux/videodev2.h>
#include "camera.h"

camera_t camera_init(gchar *dev)
{
	camera_t ret;
	ret.fd = open(dev, 3); // ioctl only magic number, see open(2)
	if (ret.fd == -1) {
		err(2, "Unable to open camera %s", dev);
	}

	// I don't have good clue about these ioctls. I just straced v4l2-ctl
	// and got it running.
	
	struct v4l2_ext_control ctrl = {
		.id = V4L2_CID_FOCUS_AUTO,
		.size = 0,
		.value = 0,
		.value64 = 0
	};
	
	struct v4l2_ext_controls ctrls = {
 		.ctrl_class = V4L2_CTRL_CLASS_CAMERA,
		.count = 1,
		.controls = &ctrl
	};
	
	if (ioctl(ret.fd, VIDIOC_S_EXT_CTRLS, &ctrls) == -1) {
		err(2, "Unable to set camera to manual focus mode. Is it supported?");
	}

	return ret;
}

void camera_set_focus(camera_t *cam, int focus)
{
	// I don't have good clue about these ioctls. I just straced v4l2-ctl
	// and got it running.
	
	struct v4l2_ext_control ctrl = {
		.id = V4L2_CID_FOCUS_ABSOLUTE,
		.size = 0,
		.value = focus,
		.value64 = focus
	};
	
	struct v4l2_ext_controls ctrls = {
 		.ctrl_class = V4L2_CTRL_CLASS_CAMERA,
		.count = 1,
		.controls = &ctrl
	};
	
	if (ioctl(cam->fd, VIDIOC_S_EXT_CTRLS, &ctrls) == -1) {
		err(2, "Unable to set focus to the camera");
	}
}
