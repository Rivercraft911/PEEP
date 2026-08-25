#ifndef PEEP_IR_TRIGGER_H
#define PEEP_IR_TRIGGER_H

#include <stdbool.h>
#include <stdint.h>

void ir_trigger_init(void);
void ir_trigger_task(uint32_t now_ms, bool allowed);
bool ir_trigger_take(void);

#endif
