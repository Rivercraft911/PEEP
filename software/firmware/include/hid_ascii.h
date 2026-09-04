#ifndef PEEP_HID_ASCII_H
#define PEEP_HID_ASCII_H

#include <stdbool.h>
#include <stdint.h>

enum {
    PEEP_MOD_LEFT_CTRL = 0x01,
    PEEP_MOD_LEFT_SHIFT = 0x02,
    PEEP_MOD_LEFT_GUI = 0x08,
    PEEP_KEY_NONE = 0x00,
    PEEP_KEY_K = 0x0E,
    PEEP_KEY_R = 0x15,
    PEEP_KEY_ENTER = 0x28,
    PEEP_KEY_ESCAPE = 0x29,
    PEEP_KEY_ARROW_RIGHT = 0x4F
};

bool hid_ascii_key(char character, uint8_t *modifiers, uint8_t *keycode);

#endif
