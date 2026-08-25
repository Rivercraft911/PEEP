#include "hid_ascii.h"

enum {
    KEY_A = 0x04,
    KEY_1 = 0x1E,
    KEY_0 = 0x27,
    KEY_SPACE = 0x2C,
    KEY_MINUS = 0x2D,
    KEY_EQUAL = 0x2E,
    KEY_BRACKET_LEFT = 0x2F,
    KEY_BRACKET_RIGHT = 0x30,
    KEY_BACKSLASH = 0x31,
    KEY_SEMICOLON = 0x33,
    KEY_APOSTROPHE = 0x34,
    KEY_GRAVE = 0x35,
    KEY_COMMA = 0x36,
    KEY_PERIOD = 0x37,
    KEY_SLASH = 0x38
};

static bool punctuation_key(char character, uint8_t *modifiers, uint8_t *keycode)
{
    static char const plain[] = " -=[]\\;'`,./";
    static char const shifted[] = " _+{}|:\"~<>?";
    static uint8_t const keys[] = {
        KEY_SPACE, KEY_MINUS, KEY_EQUAL, KEY_BRACKET_LEFT, KEY_BRACKET_RIGHT,
        KEY_BACKSLASH, KEY_SEMICOLON, KEY_APOSTROPHE, KEY_GRAVE, KEY_COMMA,
        KEY_PERIOD, KEY_SLASH
    };

    for (uint8_t index = 0; index < sizeof(keys); ++index) {
        if (character == plain[index] || character == shifted[index]) {
            *modifiers = character != ' ' && character == shifted[index]
                             ? PEEP_MOD_LEFT_SHIFT
                             : 0;
            *keycode = keys[index];
            return true;
        }
    }

    return false;
}

bool hid_ascii_key(char character, uint8_t *modifiers, uint8_t *keycode)
{
    *modifiers = 0;

    if (character >= 'a' && character <= 'z') {
        *keycode = (uint8_t)(KEY_A + character - 'a');
        return true;
    }

    if (character >= 'A' && character <= 'Z') {
        *modifiers = PEEP_MOD_LEFT_SHIFT;
        *keycode = (uint8_t)(KEY_A + character - 'A');
        return true;
    }

    if (character >= '1' && character <= '9') {
        *keycode = (uint8_t)(KEY_1 + character - '1');
        return true;
    }

    if (character == '0') {
        *keycode = KEY_0;
        return true;
    }

    if (character >= '!' && character <= ')') {
        static char const shifted_digits[] = "1234567890";

        *modifiers = PEEP_MOD_LEFT_SHIFT;
        *keycode = shifted_digits[character - '!'] == '0'
                       ? KEY_0
                       : (uint8_t)(KEY_1 + shifted_digits[character - '!'] - '1');
        return true;
    }

    return punctuation_key(character, modifiers, keycode);
}
