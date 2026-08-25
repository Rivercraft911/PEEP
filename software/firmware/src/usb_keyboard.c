#include "usb_keyboard.h"

#include "tusb.h"

void usb_keyboard_init(void)
{
    tusb_init();
}

void usb_keyboard_task(void)
{
    tud_task();
}

bool usb_keyboard_mounted(void)
{
    return tud_mounted() && !tud_suspended();
}

bool usb_keyboard_ready(void)
{
    return usb_keyboard_mounted() && tud_hid_ready();
}

bool usb_keyboard_send(uint8_t modifiers, uint8_t keycode)
{
    uint8_t keys[6] = {keycode, 0, 0, 0, 0, 0};

    return usb_keyboard_ready() && tud_hid_keyboard_report(0, modifiers, keys);
}

bool usb_keyboard_release(void)
{
    return usb_keyboard_ready() && tud_hid_keyboard_report(0, 0, NULL);
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t requested_length)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)requested_length;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t buffer_size)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)buffer_size;
}

void USB_IRQHandler(void)
{
    tud_int_handler(0);
}
