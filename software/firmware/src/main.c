#include "app.h"
#include "board.h"
#include "usb_keyboard.h"

int main(void)
{
    board_init();
    usb_keyboard_init();
    app_init();

    for (;;) {
        usb_keyboard_task();
        app_task(board_millis());
    }
}
