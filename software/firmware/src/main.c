#include "app.h"
#include "board.h"
#include "dfu_runtime.h"
#include "usb_keyboard.h"

int main(void)
{
    board_init();
    usb_keyboard_init();
    app_init();

    for (;;) {
        uint32_t now_ms;

        usb_keyboard_task();
        now_ms = board_millis();
        if (dfu_runtime_pending()) {
            app_abort();
        } else {
            app_task(now_ms);
        }
        dfu_runtime_task(now_ms);
    }
}
