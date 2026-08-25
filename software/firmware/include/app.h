#ifndef PEEP_APP_H
#define PEEP_APP_H

#include <stdint.h>

typedef enum {
    APP_WAIT_USB,
    APP_REARM,
    APP_ARMED,
    APP_COUNTDOWN,
    APP_RUNNING
} app_state_t;

void app_init(void);
void app_task(uint32_t now_ms);
app_state_t app_state(void);

#endif
