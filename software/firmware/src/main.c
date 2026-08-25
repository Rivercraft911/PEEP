#include "board.h"
#include "usb_keyboard.h"

int main(void)
{
    board_init();
    usb_keyboard_init();

    for (;;) {
        usb_keyboard_task();
    }
}
