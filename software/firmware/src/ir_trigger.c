#include "ir_trigger.h"

#include "stm32f0xx.h"
#include "trigger_gate.h"

#define IR_PIN (1U << 1)

static volatile bool falling_edge;
static trigger_gate_t gate;

void ir_trigger_init(void)
{
    GPIOA->MODER &= ~(3U << GPIO_MODER_MODER1_Pos);
    GPIOA->PUPDR = (GPIOA->PUPDR & ~(3U << GPIO_PUPDR_PUPDR1_Pos)) |
                   (1U << GPIO_PUPDR_PUPDR1_Pos);

    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI1;
    EXTI->RTSR &= ~IR_PIN;
    EXTI->FTSR |= IR_PIN;
    EXTI->PR = IR_PIN;
    EXTI->IMR |= IR_PIN;
    NVIC_SetPriority(EXTI0_1_IRQn, 2);
    NVIC_EnableIRQ(EXTI0_1_IRQn);
}

void ir_trigger_task(uint32_t now_ms, bool allowed)
{
    uint32_t primask = __get_PRIMASK();
    bool edge;

    __disable_irq();
    edge = falling_edge;
    falling_edge = false;
    if (primask == 0U) {
        __enable_irq();
    }

    trigger_gate_update(&gate, now_ms, allowed, (GPIOA->IDR & IR_PIN) != 0U, edge);
}

bool ir_trigger_take(void)
{
    return trigger_gate_take(&gate);
}

void EXTI0_1_IRQHandler(void)
{
    if ((EXTI->PR & IR_PIN) != 0U) {
        EXTI->PR = IR_PIN;
        falling_edge = true;
    }
}
