# Calibration

Calibration is partially manual work and can be performed with math
tool of your choice.

## Calibration formula

Given $x$ coordinate of the head, and calibration constants $a$, $b$, and $c$, we get the focus point $f$ with the following formula:

$$ f = {a \over x + b} + c $$

These constants are given to gcfocus with flags `-a`, `-b`, and `-c`.

## How to get focus information from camera for calibration

You can keep the camera in auto focus mode and use `v4l2-ctl --all -d
DEVICE` to check the current focus. Move target object in front of the
camera and collect numbers from different distances.

My webcam connected to X axis was happy with the following parameters:
`-a 4500 -b 35 -c 275`.
