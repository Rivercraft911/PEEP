#ifndef PEEP_DFU_RUNTIME_H
#define PEEP_DFU_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>

bool dfu_runtime_pending(void);
void dfu_runtime_task(uint32_t now_ms);

#endif
