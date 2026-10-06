#ifndef _BOARDS_SPELL_TOME_H
#define _BOARDS_SPELL_TOME_H

pico_board_cmake_set(PICO_PLATFORM, rp2040)

#ifndef PICO_AUDIO_I2S_DATA_PIN
#define PICO_AUDIO_I2S_DATA_PIN 10
#endif

#ifndef PICO_AUDIO_I2S_CLOCK_PIN_BASE
#define PICO_AUDIO_I2S_CLOCK_PIN_BASE 11
#endif

#include "boards/pico.h"

#endif
