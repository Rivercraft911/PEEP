#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app.h"
#include "hid_ascii.h"
#include "ir_trigger.h"
#include "macro.h"
#include "trigger_gate.h"

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition);  \
            return false;                                                      \
        }                                                                      \
    } while (0)

typedef struct {
    uint32_t at_ms;
    uint8_t modifiers;
    uint8_t keycode;
    bool release;
} report_event_t;

typedef struct {
    report_event_t events[32];
    uint32_t now_ms;
    uint8_t count;
    bool available;
    bool ready;
} fake_usb_t;

static bool app_usb_mounted;
static bool app_usb_ready;
static trigger_gate_t app_ir_gate;
static bool app_ir_line_high;
static bool app_ir_edge;
static bool app_ir_allowed;
static uint16_t app_ir_init_count;
static uint16_t app_report_count;
static uint16_t app_release_count;

void ir_trigger_init(void)
{
    app_ir_gate = (trigger_gate_t){0};
    app_ir_edge = false;
    app_ir_allowed = false;
    ++app_ir_init_count;
}

void ir_trigger_task(uint32_t now_ms, bool allowed)
{
    trigger_gate_update(&app_ir_gate, now_ms, allowed, app_ir_line_high, app_ir_edge);
    app_ir_edge = false;
    app_ir_allowed = allowed;
}

bool ir_trigger_take(void)
{
    return trigger_gate_take(&app_ir_gate);
}

bool ir_trigger_armed(void)
{
    return trigger_gate_armed(&app_ir_gate);
}

bool usb_keyboard_mounted(void)
{
    return app_usb_mounted;
}

bool usb_keyboard_ready(void)
{
    return app_usb_mounted && app_usb_ready;
}

bool usb_keyboard_send(uint8_t modifiers, uint8_t keycode)
{
    (void)modifiers;
    (void)keycode;
    ++app_report_count;
    return true;
}

bool usb_keyboard_release(void)
{
    ++app_report_count;
    ++app_release_count;
    return true;
}

static void reset_app(void)
{
    app_usb_mounted = false;
    app_usb_ready = true;
    app_ir_line_high = true;
    app_ir_init_count = 0;
    app_report_count = 0;
    app_release_count = 0;
    app_init();
}

static bool arm_app(uint32_t now_ms)
{
    reset_app();
    app_usb_mounted = true;
    app_task(now_ms);
    CHECK(app_state() == APP_REARM);
    app_task(now_ms + 250U);
    CHECK(app_state() == APP_ARMED);
    return true;
}

static bool fire_app(uint32_t now_ms)
{
    app_ir_line_high = false;
    app_ir_edge = true;
    app_task(now_ms);
    CHECK(app_state() == APP_RUNNING);
    CHECK(!app_ir_allowed);
    return true;
}

static bool fake_available(void *context)
{
    return ((fake_usb_t *)context)->available;
}

static bool fake_ready(void *context)
{
    return ((fake_usb_t *)context)->ready;
}

static bool fake_send(void *context, uint8_t modifiers, uint8_t keycode)
{
    fake_usb_t *usb = context;

    usb->events[usb->count++] = (report_event_t){
        .at_ms = usb->now_ms,
        .modifiers = modifiers,
        .keycode = keycode
    };
    return true;
}

static bool fake_release(void *context)
{
    fake_usb_t *usb = context;

    usb->events[usb->count++] = (report_event_t){
        .at_ms = usb->now_ms,
        .release = true
    };
    return true;
}

static macro_io_t fake_io(fake_usb_t *usb)
{
    return (macro_io_t){
        .context = usb,
        .available = fake_available,
        .ready = fake_ready,
        .send = fake_send,
        .release = fake_release
    };
}

static bool check_key(char character, uint8_t modifiers, uint8_t keycode)
{
    uint8_t actual_modifiers;
    uint8_t actual_keycode;

    CHECK(hid_ascii_key(character, &actual_modifiers, &actual_keycode));
    CHECK(actual_modifiers == modifiers);
    CHECK(actual_keycode == keycode);
    return true;
}

static bool test_ascii(void)
{
    uint8_t modifiers;
    uint8_t keycode;

    for (char character = ' '; character <= '~'; ++character) {
        CHECK(hid_ascii_key(character, &modifiers, &keycode));
    }

    CHECK(check_key('a', 0, 0x04));
    CHECK(check_key('A', PEEP_MOD_LEFT_SHIFT, 0x04));
    CHECK(check_key('0', 0, 0x27));
    CHECK(check_key('!', PEEP_MOD_LEFT_SHIFT, 0x1E));
    CHECK(check_key('@', PEEP_MOD_LEFT_SHIFT, 0x1F));
    CHECK(check_key('\'', 0, 0x34));
    CHECK(check_key(':', PEEP_MOD_LEFT_SHIFT, 0x33));
    CHECK(check_key('?', PEEP_MOD_LEFT_SHIFT, 0x38));
    CHECK(check_key(' ', 0, 0x2C));
    CHECK(!hid_ascii_key('\n', &modifiers, &keycode));
    return true;
}

static bool test_trigger(void)
{
    trigger_gate_t gate = {0};

    trigger_gate_update(&gate, 100, true, true, false);
    trigger_gate_update(&gate, 349, true, true, false);
    CHECK(!trigger_gate_armed(&gate));
    trigger_gate_update(&gate, 350, true, true, false);
    CHECK(trigger_gate_armed(&gate));

    trigger_gate_update(&gate, 351, true, false, true);
    CHECK(!trigger_gate_armed(&gate));
    CHECK(trigger_gate_take(&gate));
    CHECK(!trigger_gate_take(&gate));

    trigger_gate_update(&gate, 400, false, true, false);
    trigger_gate_update(&gate, 700, false, true, false);
    CHECK(!trigger_gate_armed(&gate));
    trigger_gate_update(&gate, 701, true, true, false);
    trigger_gate_update(&gate, 951, true, true, false);
    CHECK(trigger_gate_armed(&gate));
    return true;
}

static bool test_trigger_wrap(void)
{
    trigger_gate_t gate = {0};
    uint32_t start = UINT32_MAX - 100U;

    trigger_gate_update(&gate, start, true, true, false);
    trigger_gate_update(&gate, start + 249U, true, true, false);
    CHECK(!trigger_gate_armed(&gate));
    trigger_gate_update(&gate, start + 250U, true, true, false);
    CHECK(trigger_gate_armed(&gate));
    return true;
}

static bool test_trigger_quiet_time(void)
{
    trigger_gate_t gate = {0};

    trigger_gate_update(&gate, 0, true, true, false);
    trigger_gate_update(&gate, 250, true, true, true);
    CHECK(!trigger_gate_armed(&gate));
    trigger_gate_update(&gate, 251, true, true, false);
    trigger_gate_update(&gate, 500, true, true, false);
    CHECK(!trigger_gate_armed(&gate));
    trigger_gate_update(&gate, 501, true, true, false);
    CHECK(trigger_gate_armed(&gate));

    trigger_gate_update(&gate, 502, true, false, true);
    trigger_gate_update(&gate, 503, true, true, true);
    CHECK(trigger_gate_take(&gate));
    CHECK(!trigger_gate_take(&gate));
    trigger_gate_update(&gate, 504, true, true, false);
    trigger_gate_update(&gate, 753, true, true, false);
    CHECK(!trigger_gate_armed(&gate));
    trigger_gate_update(&gate, 754, true, true, false);
    CHECK(trigger_gate_armed(&gate));
    trigger_gate_update(&gate, 755, false, false, true);
    CHECK(!trigger_gate_armed(&gate));
    CHECK(!trigger_gate_take(&gate));
    return true;
}

static bool test_macro_sequence(void)
{
    static macro_action_t const actions[] = {
        MACRO_WAIT(5),
        MACRO_KEY(PEEP_MOD_LEFT_GUI, PEEP_KEY_R),
        MACRO_TEXT("A!"),
        MACRO_END()
    };
    fake_usb_t usb = {.available = true, .ready = true};
    macro_runner_t runner;

    macro_init(&runner, fake_io(&usb));
    CHECK(macro_start(&runner, actions, 100));
    CHECK(!macro_start(&runner, actions, 100));
    for (uint32_t now = 100; now < 250; ++now) {
        usb.now_ms = now;
        macro_task(&runner, now);
    }

    CHECK(!macro_busy(&runner));
    CHECK(usb.count == 6);
    CHECK(!usb.events[0].release);
    CHECK(usb.events[0].at_ms >= 105);
    CHECK(usb.events[0].modifiers == PEEP_MOD_LEFT_GUI);
    CHECK(usb.events[0].keycode == PEEP_KEY_R);
    CHECK(usb.events[1].release);
    CHECK(usb.events[2].modifiers == PEEP_MOD_LEFT_SHIFT);
    CHECK(usb.events[2].keycode == 0x04);
    CHECK(usb.events[3].release);
    CHECK(usb.events[4].modifiers == PEEP_MOD_LEFT_SHIFT);
    CHECK(usb.events[4].keycode == 0x1E);
    CHECK(usb.events[5].release);

    for (uint8_t index = 0; index < usb.count; index += 2) {
        CHECK(usb.events[index + 1].at_ms - usb.events[index].at_ms >= 10U);
    }
    return true;
}

static bool test_macro_wrap(void)
{
    static macro_action_t const actions[] = {
        MACRO_WAIT(20),
        MACRO_KEY(0, PEEP_KEY_ENTER),
        MACRO_END()
    };
    fake_usb_t usb = {.available = true, .ready = true};
    macro_runner_t runner;
    uint32_t start = UINT32_MAX - 10U;

    macro_init(&runner, fake_io(&usb));
    CHECK(macro_start(&runner, actions, start));
    for (uint32_t elapsed = 0; elapsed < 80U; ++elapsed) {
        usb.now_ms = start + elapsed;
        macro_task(&runner, usb.now_ms);
    }

    CHECK(!macro_busy(&runner));
    CHECK(usb.count == 2);
    CHECK((uint32_t)(usb.events[0].at_ms - start) >= 20U);
    CHECK(usb.events[1].release);
    return true;
}

static bool test_usb_loss(void)
{
    static macro_action_t const actions[] = {
        MACRO_KEY(PEEP_MOD_LEFT_SHIFT, 0x04),
        MACRO_WAIT(100),
        MACRO_END()
    };
    fake_usb_t usb = {.available = true, .ready = true};
    macro_runner_t runner;

    macro_init(&runner, fake_io(&usb));
    CHECK(macro_start(&runner, actions, 0));
    usb.now_ms = 0;
    macro_task(&runner, 0);
    usb.now_ms = 1;
    macro_task(&runner, 1);
    CHECK(usb.count == 1);

    usb.available = false;
    usb.now_ms = 2;
    macro_task(&runner, 2);
    CHECK(macro_busy(&runner));
    CHECK(usb.count == 1);

    usb.available = true;
    usb.now_ms = 3;
    macro_task(&runner, 3);
    CHECK(!macro_busy(&runner));
    CHECK(usb.count == 2);
    CHECK(usb.events[1].release);
    return true;
}

static bool test_ir_launch(void)
{
    uint32_t now = 180001U;
    uint16_t completed_reports;

    CHECK(arm_app(0));
    CHECK(app_ir_init_count == 1);
    app_task(180000);
    CHECK(app_state() == APP_ARMED);
    CHECK(app_report_count == 0);
    CHECK(fire_app(now));

    app_ir_line_high = true;
    while (app_state() == APP_RUNNING && now < 240000U) {
        app_ir_edge = true;
        app_task(++now);
        CHECK(!app_ir_allowed);
    }
    CHECK(app_state() == APP_REARM);
    CHECK(app_report_count > 0);

    completed_reports = app_report_count;
    app_task(++now);
    CHECK(app_state() == APP_REARM);
    app_task(now + 249U);
    CHECK(app_state() == APP_REARM);
    app_task(now + 250U);
    CHECK(app_state() == APP_ARMED);
    CHECK(app_report_count == completed_reports);

    CHECK(fire_app(now + 251U));
    return true;
}

static bool test_ir_usb_ready(void)
{
    reset_app();
    app_ir_edge = true;
    app_task(0);
    CHECK(app_state() == APP_WAIT_USB);
    CHECK(!app_ir_allowed);

    app_usb_mounted = true;
    app_usb_ready = false;
    app_ir_edge = true;
    app_task(1000);
    CHECK(app_state() == APP_WAIT_USB);
    CHECK(!app_ir_allowed);
    CHECK(!ir_trigger_take());

    app_usb_ready = true;
    app_task(2000);
    CHECK(app_state() == APP_REARM);
    app_task(2249);
    CHECK(app_state() == APP_REARM);
    app_task(2250);
    CHECK(app_state() == APP_ARMED);

    app_usb_ready = false;
    app_ir_edge = true;
    app_task(2251);
    CHECK(app_state() == APP_WAIT_USB);
    CHECK(!ir_trigger_take());
    CHECK(app_report_count == 0);
    return true;
}

static bool test_ir_low_input(void)
{
    reset_app();
    app_usb_mounted = true;
    app_ir_line_high = false;
    app_task(0);
    app_task(10000);
    CHECK(app_state() == APP_REARM);
    CHECK(app_report_count == 0);

    app_ir_line_high = true;
    app_task(10001);
    app_task(10250);
    CHECK(app_state() == APP_REARM);
    app_task(10251);
    CHECK(app_state() == APP_ARMED);
    return true;
}

static bool test_ir_usb_loss(void)
{
    CHECK(arm_app(100));
    CHECK(fire_app(351));
    app_task(352);
    app_task(353);
    CHECK(app_report_count == 1);
    CHECK(app_release_count == 0);

    app_usb_mounted = false;
    app_ir_edge = true;
    app_task(354);
    CHECK(app_state() == APP_WAIT_USB);
    CHECK(!app_ir_allowed);
    CHECK(!ir_trigger_take());
    CHECK(app_release_count == 0);

    app_usb_mounted = true;
    app_task(400);
    CHECK(app_state() == APP_REARM);
    CHECK(app_release_count == 1);
    CHECK(app_report_count == 2);

    app_ir_line_high = true;
    app_task(401);
    app_task(650);
    CHECK(app_state() == APP_REARM);
    app_task(651);
    CHECK(app_state() == APP_ARMED);
    CHECK(app_report_count == 2);
    return true;
}

static bool test_ir_abort(void)
{
    CHECK(arm_app(0));
    app_ir_edge = true;
    app_abort();
    CHECK(app_state() == APP_WAIT_USB);
    CHECK(!app_ir_allowed);
    CHECK(!ir_trigger_armed());
    CHECK(!ir_trigger_take());
    CHECK(app_report_count == 0);

    CHECK(arm_app(0));
    CHECK(fire_app(251));
    app_task(252);
    app_task(253);
    CHECK(app_report_count == 1);
    app_ir_edge = true;
    app_abort();
    CHECK(app_state() == APP_WAIT_USB);
    CHECK(!app_ir_allowed);
    CHECK(!ir_trigger_take());
    app_task(254);
    CHECK(app_release_count == 1);
    CHECK(app_state() == APP_REARM);
    CHECK(app_report_count == 2);
    return true;
}

static bool test_ir_wrap(void)
{
    uint32_t start = UINT32_MAX - 100U;

    CHECK(arm_app(start));
    CHECK(fire_app(start + 251U));
    app_task(start + 252U);
    app_task(start + 253U);
    CHECK(app_report_count == 1);
    return true;
}

int main(void)
{
    static bool (*const tests[])(void) = {
        test_ascii,
        test_trigger,
        test_trigger_wrap,
        test_trigger_quiet_time,
        test_macro_sequence,
        test_macro_wrap,
        test_usb_loss,
        test_ir_launch,
        test_ir_usb_ready,
        test_ir_low_input,
        test_ir_usb_loss,
        test_ir_abort,
        test_ir_wrap
    };
    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    for (size_t index = 0; index < test_count; ++index) {
        if (!tests[index]()) {
            return 1;
        }
    }

    printf("%zu tests passed\n", test_count);
    return 0;
}
