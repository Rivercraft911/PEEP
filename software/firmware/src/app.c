#include "app.h"

#include <stdbool.h>
#include <stddef.h>

#include "hid_ascii.h"
#include "macro.h"
#include "usb_keyboard.h"

#define STARTUP_DELAY_MS 10000U

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
    MACRO_TEXT("There comes a time in everyone's life where they are faced with a choice."),
    MACRO_KEY(0, PEEP_KEY_ENTER),
    MACRO_TEXT("To send river a rickroll, or to continue life in bliss."),
    MACRO_KEY(0, PEEP_KEY_ENTER),
    MACRO_TEXT("You made your choice."),
    MACRO_KEY(0, PEEP_KEY_ENTER),
    MACRO_TEXT("Face the consequences."),
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

    macro_init(&runner, io);
    state = APP_WAIT_USB;
}

void app_task(uint32_t now_ms)
{
    macro_task(&runner, now_ms);

    if (!usb_keyboard_mounted()) {
        macro_abort(&runner);
        state = APP_WAIT_USB;
        return;
    }

    if (state == APP_WAIT_USB) {
        if (!macro_busy(&runner)) {
            countdown_end = now_ms + STARTUP_DELAY_MS;
            state = APP_COUNTDOWN;
        }
        return;
    }

    if (state == APP_COUNTDOWN) {
        if ((int32_t)(now_ms - countdown_end) >= 0) {
            state = macro_start(&runner, demo, now_ms) ? APP_RUNNING : APP_WAIT_USB;
        }
    } else if (state == APP_RUNNING) {
        if (!macro_busy(&runner)) {
            state = APP_DONE;
        }
    }
}

void app_abort(void)
{
    macro_abort(&runner);
    state = APP_WAIT_USB;
}

app_state_t app_state(void)
{
    return state;
}
