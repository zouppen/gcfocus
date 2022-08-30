# Calibration

Calibration is partially manual work and can be performed with math
tool of your choice.

## Calibration formula

Given $x$ coordinate of the head, and calibration constants $a$, $b$, and $c$, we get the focus point $f$ with the following formula:

$$ f = {a \over x + b} + c $$

These constants are given to gcfocus with flags `-a`, `-b`, and `-c`.
