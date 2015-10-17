#ifndef LEDS_H
#define LEDS_H


#include <stdint.h>
#include <stdio.h>
#include <xc.h>
#include "HardwareDefs.h"

// sets TRISE and LATE to zero
#define LEDS_INIT(void) do { \
        TRISE = 0; \
        LATE = 0; \
} while (0)

// returns value of LATE register (don't add semicolon within macro)
#define LEDS_GET() (LATE)


// sets LATE register value to x (don't add semicolon within macro)
#define LEDS_SET(x) (LATE = (x))


#endif // LEDS_H