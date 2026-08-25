#include "app.h"

#include <stdbool.h>
#include <stddef.h>

#include "hid_ascii.h"
#include "ir_trigger.h"
#include "macro.h"
#include "usb_keyboard.h"

#define COUNTDOWN_MS 3000U

static macro_action_t const demo[] = {
    MACRO_KEY(PEEP_MOD_LEFT_GUI, PEEP_KEY_R),
    MACRO_WAIT(500),
    MACRO_TEXT("https://www.youtube.com/watch?v=dQw4w9WgXcQ"),
    MACRO_KEY(0, PEEP_KEY_ENTER),
    MACRO_WAIT(5000),
    MACRO_KEY(PEEP_MOD_LEFT_GUI, PEEP_KEY_R),
    MACRO_WAIT(500),
    MACRO_TEXT("notepad"),
    MACRO_KEY(0, PEEP_KEY_ENTER),
    MACRO_WAIT(1000),
    MACRO_KEY(PEEP_MOD_LEFT_GUI, PEEP_KEY_ARROW_RIGHT),
    MACRO_WAIT(500),
    MACRO_KEY(0, PEEP_KEY_ESCAPE),
    MACRO_WAIT(250),
    MACRO_TEXT("Hello from PEEP."),
    MACRO_END()
};

static macro_runner_t runner;
static app_state_t state;
static uint32_t countdown_end;

static bool usb_available(void *context)
{
    (void)context;
    return usb_keyboard_mounted();
}

static bool usb_ready(void *context)
{
    (void)context;
    return usb_keyboard_ready();
}

static bool usb_send(void *context, uint8_t modifiers, uint8_t keycode)
{
    (void)context;
    return usb_keyboard_send(modifiers, keycode);
}

static bool usb_release(void *context)
{
    (void)context;
    return usb_keyboard_release();
}

void app_init(void)
{
    macro_io_t io = {
        .context = NULL,
        .available = usb_available,
        .ready = usb_ready,
        .send = usb_send,
        .release = usb_release
    };

    ir_trigger_init();
    macro_init(&runner, io);
    state = APP_WAIT_USB;
}

void app_task(uint32_t now_ms)
{
    macro_task(&runner, now_ms);

    if (!usb_keyboard_mounted()) {
        macro_abort(&runner);
        ir_trigger_task(now_ms, false);
        state = APP_WAIT_USB;
        return;
    }

    if (state == APP_WAIT_USB) {
        if (!macro_busy(&runner)) {
            state = APP_REARM;
        }
        return;
    }

    if (state == APP_REARM) {
        ir_trigger_task(now_ms, true);
        if (ir_trigger_armed()) {
            state = APP_ARMED;
        }
    } else if (state == APP_ARMED) {
        ir_trigger_task(now_ms, true);
        if (ir_trigger_take()) {
            countdown_end = now_ms + COUNTDOWN_MS;
            state = APP_COUNTDOWN;
        }
    } else if (state == APP_COUNTDOWN) {
        ir_trigger_task(now_ms, false);
        if ((int32_t)(now_ms - countdown_end) >= 0) {
            state = macro_start(&runner, demo, now_ms) ? APP_RUNNING : APP_WAIT_USB;
        }
    } else {
        ir_trigger_task(now_ms, false);
        if (!macro_busy(&runner)) {
            state = APP_REARM;
        }
    }
}

app_state_t app_state(void)
{
    return state;
}
