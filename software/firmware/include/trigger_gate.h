#ifndef PEEP_TRIGGER_GATE_H
#define PEEP_TRIGGER_GATE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t high_since;
    bool tracking_high;
    bool armed;
    bool fired;
} trigger_gate_t;

void trigger_gate_update(trigger_gate_t *gate, uint32_t now_ms, bool allowed,
                         bool line_high, bool falling_edge);
bool trigger_gate_take(trigger_gate_t *gate);

#endif
