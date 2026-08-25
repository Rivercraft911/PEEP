#include "stm32f0xx.h"

int main(void)
{
    SystemCoreClockUpdate();

    for (;;) {
        __WFI();
    }
}
