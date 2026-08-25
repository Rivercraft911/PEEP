#include "board.h"
#include "ir_trigger.h"
#include "usb_keyboard.h"

int main(void)
{
    board_init();
    ir_trigger_init();
    usb_keyboard_init();

    for (;;) {
        usb_keyboard_task();
        ir_trigger_task(board_millis(), usb_keyboard_mounted());
        (void)ir_trigger_take();
    }
}
