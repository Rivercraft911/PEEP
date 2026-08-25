#ifndef PEEP_MACRO_H
#define PEEP_MACRO_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    MACRO_ACTION_WAIT_MS,
    MACRO_ACTION_KEY,
    MACRO_ACTION_TEXT,
    MACRO_ACTION_END
} macro_action_kind_t;

typedef struct {
    macro_action_kind_t kind;
    union {
        uint32_t wait_ms;
        struct {
            uint8_t modifiers;
            uint8_t keycode;
        } key;
        char const *text;
    } value;
} macro_action_t;

#define MACRO_WAIT(ms) \
    { .kind = MACRO_ACTION_WAIT_MS, .value.wait_ms = (ms) }
#define MACRO_KEY(mods, usage) \
    { .kind = MACRO_ACTION_KEY, .value.key = { (mods), (usage) } }
#define MACRO_TEXT(string) \
    { .kind = MACRO_ACTION_TEXT, .value.text = (string) }
#define MACRO_END() \
    { .kind = MACRO_ACTION_END }

typedef struct {
    void *context;
    bool (*available)(void *context);
    bool (*ready)(void *context);
    bool (*send)(void *context, uint8_t modifiers, uint8_t keycode);
    bool (*release)(void *context);
} macro_io_t;

typedef struct {
    macro_io_t io;
    macro_action_t const *actions;
    char const *text;
    uint32_t deadline;
    uint16_t action_index;
    uint8_t modifiers;
    uint8_t keycode;
    uint8_t phase;
    bool active;
    bool key_down;
    bool aborting;
} macro_runner_t;

void macro_init(macro_runner_t *runner, macro_io_t io);
bool macro_start(macro_runner_t *runner, macro_action_t const *actions,
                 uint32_t now_ms);
void macro_task(macro_runner_t *runner, uint32_t now_ms);
void macro_abort(macro_runner_t *runner);
bool macro_busy(macro_runner_t const *runner);

#endif
