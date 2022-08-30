# gcfocus 

Focus 3D printer or CNC machine camera based on X-axis position.

## Motivation

In many cases it is difficult to use auto focus camera with 3D printer
or CNC machines since the object and machine parts are tricking the
automatic focus algorithm in cameras. Manual focus works better but
it's not optimal if the camera is attached near the head of the
machine and it's difficult to find a focus point which works for the
whole working range.

Since X position is given in the G-code it's rather straightforward to
parse the position from there, we could set the focus of the camera
manually but dynamically.

It is not just eyecandy for stream viewers but useful for checking if
the head is clean and not damaged.

## Goal

- Reads Octoprint serial log with inotify
- Feed it to very simple G-code parser
- Translate coordinate commands to absolute coordinates
- Control manual focus of given V4L2 device using given lookup table
- Truncate serial log periodically from the beginning to save space (TODO)

## Calibration

See [calibration](calibration.md).

## Requirements

- Linux
- glib
- cmake

## Building

Install dependencies. In Debian based systems for example:

```sh
sudo apt install cmake libglib2.0-dev
```

And build:

```sh
mkdir build
cd build
cmake ..
```


## Status

In production at Hacklab Jyväskylä. Calibration might need some helper tool.

## Author

Zouppen / Joel Lehtonen
