#include "app.h"

#include <stdbool.h>
#include <stddef.h>

#include "hid_ascii.h"
#include "macro.h"
#include "usb_keyboard.h"

#define START_DELAY_MS 15000U

static macro_action_t const demo[] = {
    MACRO_KEY(PEEP_MOD_LEFT_GUI, PEEP_KEY_NONE),
    MACRO_WAIT(600),
    MACRO_TEXT("griffinfacepalm95.exe"),
    MACRO_KEY(0, PEEP_KEY_ENTER),
    MACRO_WAIT(3000),
    MACRO_KEY(PEEP_MOD_LEFT_GUI, PEEP_KEY_NONE),
    MACRO_WAIT(600),
    MACRO_TEXT("slack"),
    MACRO_KEY(0, PEEP_KEY_ENTER),
    MACRO_WAIT(8000),
    MACRO_KEY(PEEP_MOD_LEFT_CTRL, PEEP_KEY_K),
    MACRO_WAIT(500),
    MACRO_TEXT("julian sanders"),
    MACRO_WAIT(1500),
    MACRO_KEY(0, PEEP_KEY_ENTER),
    MACRO_WAIT(1000),
    MACRO_TEXT("you know julian. I think I want to go back to using second order effects. While we are at it we should also keep geckos forever. They are super scalable."),
    MACRO_KEY(0, PEEP_KEY_ENTER),
    MACRO_WAIT(15000),
    MACRO_KEY(PEEP_MOD_LEFT_GUI, PEEP_KEY_R),
    MACRO_WAIT(500),
    MACRO_TEXT("https://www.youtube.com/watch?v=uT6mKkkvjJY"),
    MACRO_KEY(0, PEEP_KEY_ENTER),
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
        countdown_end = now_ms + START_DELAY_MS;
        state = APP_COUNTDOWN;
    } else if (state == APP_COUNTDOWN) {
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
