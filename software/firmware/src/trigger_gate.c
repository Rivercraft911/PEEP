#include "trigger_gate.h"

#define REARM_MS 250U

void trigger_gate_update(trigger_gate_t *gate, uint32_t now_ms, bool allowed,
                         bool line_high, bool falling_edge)
{
    if (!allowed) {
        gate->tracking_high = false;
        gate->armed = false;
        gate->fired = false;
        return;
    }

    if (gate->armed) {
        if (falling_edge) {
            gate->tracking_high = false;
            gate->armed = false;
            gate->fired = true;
        }
        return;
    }

    if (gate->fired) {
        return;
    }

    if (!line_high || falling_edge) {
        gate->tracking_high = false;
    } else if (!gate->tracking_high) {
        gate->tracking_high = true;
        gate->high_since = now_ms;
    } else if ((uint32_t)(now_ms - gate->high_since) >= REARM_MS) {
        gate->armed = true;
    }
}

bool trigger_gate_take(trigger_gate_t *gate)
{
    bool fired = gate->fired;

    gate->fired = false;
    return fired;
}

bool trigger_gate_armed(trigger_gate_t const *gate)
{
    return gate->armed;
}
