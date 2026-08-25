#ifndef PEEP_USB_KEYBOARD_H
#define PEEP_USB_KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

void usb_keyboard_init(void);
void usb_keyboard_task(void);
bool usb_keyboard_mounted(void);
bool usb_keyboard_ready(void);
bool usb_keyboard_send(uint8_t modifiers, uint8_t keycode);
bool usb_keyboard_release(void);

#endif
