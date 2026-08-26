#include "dfu_runtime.h"

#include "board.h"
#include "tusb.h"
#include "usb_keyboard.h"

#define RELEASE_TIMEOUT_MS 250U
#define DISCONNECT_MS 20U

static bool requested;
static bool release_sent;
static bool disconnected;
static uint32_t request_ms;
static uint32_t disconnect_ms;

void tud_dfu_runtime_reboot_to_dfu_cb(void)
{
    if (!requested) {
        request_ms = board_millis();
        requested = true;
    }
}

bool dfu_runtime_pending(void)
{
    return requested;
}

void dfu_runtime_task(uint32_t now_ms)
{
    if (!requested) {
        return;
    }

    if (!release_sent && usb_keyboard_ready()) {
        release_sent = usb_keyboard_release();
        return;
    }

    if (!disconnected &&
        ((release_sent && usb_keyboard_ready()) ||
         (uint32_t)(now_ms - request_ms) >= RELEASE_TIMEOUT_MS)) {
        tud_disconnect();
        disconnect_ms = now_ms;
        disconnected = true;
        return;
    }

    if (disconnected && (uint32_t)(now_ms - disconnect_ms) >= DISCONNECT_MS) {
        board_enter_bootloader();
    }
}
