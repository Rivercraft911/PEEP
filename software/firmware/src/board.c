#include "board.h"

#include "stm32f0xx.h"

static volatile uint32_t ticks;

static void clock_init(void)
{
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;

    RCC->CR2 |= RCC_CR2_HSI48ON;
    while ((RCC->CR2 & RCC_CR2_HSI48RDY) == 0U) {
    }

    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE)) |
                RCC_CFGR_SW_HSI48;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI48) {
    }

    SystemCoreClock = 48000000U;
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    RCC->APB1ENR |= RCC_APB1ENR_CRSEN | RCC_APB1ENR_USBEN;
    RCC->CFGR3 &= ~RCC_CFGR3_USBSW;

    SYSCFG->CFGR1 |= SYSCFG_CFGR1_PA11_PA12_RMP;
    CRS->CFGR = 47999U | (34U << CRS_CFGR_FELIM_Pos) | CRS_CFGR_SYNCSRC_1;
    CRS->CR |= CRS_CR_AUTOTRIMEN | CRS_CR_CEN;
}

void board_init(void)
{
    clock_init();
    SysTick_Config(SystemCoreClock / 1000U);
}

uint32_t board_millis(void)
{
    return ticks;
}

uint32_t tusb_time_millis_api(void)
{
    return board_millis();
}

void SysTick_Handler(void)
{
    ++ticks;
}
