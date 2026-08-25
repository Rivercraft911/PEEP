#include "board.h"

#include "stm32f0xx.h"

int main(void)
{
    board_init();

    for (;;) {
        __WFI();
    }
}
