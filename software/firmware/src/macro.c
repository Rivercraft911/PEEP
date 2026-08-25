#include "macro.h"

#include <stddef.h>

#include "hid_ascii.h"

#define REPORT_MS 10U

enum {
    PHASE_ACTION,
    PHASE_WAIT,
    PHASE_TEXT,
    PHASE_PRESS,
    PHASE_RELEASE,
    PHASE_GAP
};

static bool time_reached(uint32_t now_ms, uint32_t deadline)
{
    return (int32_t)(now_ms - deadline) >= 0;
}

void macro_init(macro_runner_t *runner, macro_io_t io)
{
    runner->io = io;
    runner->actions = NULL;
    runner->text = NULL;
    runner->active = false;
    runner->key_down = false;
    runner->aborting = false;
}

bool macro_start(macro_runner_t *runner, macro_action_t const *actions,
                 uint32_t now_ms)
{
    if (macro_busy(runner) || actions == NULL || !runner->io.available(runner->io.context)) {
        return false;
    }

    runner->actions = actions;
    runner->text = NULL;
    runner->deadline = now_ms;
    runner->action_index = 0;
    runner->phase = PHASE_ACTION;
    runner->active = true;
    return true;
}

void macro_abort(macro_runner_t *runner)
{
    runner->active = false;
    runner->actions = NULL;
    runner->text = NULL;
    runner->aborting = runner->key_down;
}

bool macro_busy(macro_runner_t const *runner)
{
    return runner->active || runner->aborting;
}

void macro_task(macro_runner_t *runner, uint32_t now_ms)
{
    if (runner->aborting) {
        if (runner->io.available(runner->io.context) &&
            runner->io.ready(runner->io.context) &&
            runner->io.release(runner->io.context)) {
            runner->key_down = false;
            runner->aborting = false;
        }
        return;
    }

    if (!runner->active) {
        return;
    }

    if (!runner->io.available(runner->io.context)) {
        macro_abort(runner);
        return;
    }

    switch (runner->phase) {
    case PHASE_ACTION: {
        macro_action_t const *action = &runner->actions[runner->action_index++];

        if (action->kind == MACRO_ACTION_WAIT_MS) {
            runner->deadline = now_ms + action->value.wait_ms;
            runner->phase = PHASE_WAIT;
        } else if (action->kind == MACRO_ACTION_KEY) {
            runner->modifiers = action->value.key.modifiers;
            runner->keycode = action->value.key.keycode;
            runner->phase = PHASE_PRESS;
        } else if (action->kind == MACRO_ACTION_TEXT) {
            runner->text = action->value.text;
            runner->phase = PHASE_TEXT;
        } else {
            runner->active = false;
            runner->actions = NULL;
        }
        break;
    }
    case PHASE_WAIT:
        if (time_reached(now_ms, runner->deadline)) {
            runner->phase = PHASE_ACTION;
        }
        break;
    case PHASE_TEXT:
        if (*runner->text == '\0') {
            runner->text = NULL;
            runner->phase = PHASE_ACTION;
        } else if (hid_ascii_key(*runner->text++, &runner->modifiers, &runner->keycode)) {
            runner->phase = PHASE_PRESS;
        } else {
            macro_abort(runner);
        }
        break;
    case PHASE_PRESS:
        if (runner->io.ready(runner->io.context) &&
            runner->io.send(runner->io.context, runner->modifiers, runner->keycode)) {
            runner->key_down = true;
            runner->deadline = now_ms + REPORT_MS;
            runner->phase = PHASE_RELEASE;
        }
        break;
    case PHASE_RELEASE:
        if (time_reached(now_ms, runner->deadline) && runner->io.ready(runner->io.context) &&
            runner->io.release(runner->io.context)) {
            runner->key_down = false;
            runner->deadline = now_ms + REPORT_MS;
            runner->phase = PHASE_GAP;
        }
        break;
    case PHASE_GAP:
        if (time_reached(now_ms, runner->deadline)) {
            runner->phase = runner->text == NULL ? PHASE_ACTION : PHASE_TEXT;
        }
        break;
    default:
        macro_abort(runner);
        break;
    }
}
