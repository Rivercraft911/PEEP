#ifndef PEEP_BOARD_H
#define PEEP_BOARD_H

#include <stdint.h>

void board_init(void);
uint32_t board_millis(void);
__attribute__((noreturn)) void board_enter_bootloader(void);

#endif
