#include "stm32f0xx.h"
#include "tusb.h"

enum {
    STRING_LANGID,
    STRING_MANUFACTURER,
    STRING_PRODUCT,
    STRING_SERIAL,
    STRING_DFU
};

static tusb_desc_device_t const device_descriptor = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0xCAFE,
    .idProduct = 0x5045,
    .bcdDevice = 0x0101,
    .iManufacturer = STRING_MANUFACTURER,
    .iProduct = STRING_PRODUCT,
    .iSerialNumber = STRING_SERIAL,
    .bNumConfigurations = 1
};

static uint8_t const hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};

static uint8_t const configuration_descriptor[] = {
    TUD_CONFIG_DESCRIPTOR(1, 2, 0,
                          TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_DFU_RT_DESC_LEN,
                          0, 100),
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_KEYBOARD,
                       sizeof(hid_report_descriptor), 0x81, 8, 10),
    TUD_DFU_RT_DESCRIPTOR(1, STRING_DFU,
                          DFU_ATTR_CAN_DOWNLOAD | DFU_ATTR_CAN_UPLOAD |
                              DFU_ATTR_WILL_DETACH,
                          1000, 1024)
};

static char const *const strings[STRING_DFU + 1] = {
    [STRING_MANUFACTURER] = "PEEP",
    [STRING_PRODUCT] = "PEEP Keyboard",
    [STRING_DFU] = "PEEP DFU Runtime"
};

static uint16_t string_descriptor[32];

uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&device_descriptor;
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void)instance;
    return hid_report_descriptor;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return configuration_descriptor;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    uint8_t count = 0;

    (void)langid;
    if (index == STRING_LANGID) {
        string_descriptor[1] = 0x0409;
        count = 1;
    } else if (index == STRING_SERIAL) {
        uint32_t const *uid = (uint32_t const *)UID_BASE;
        static char const hex[] = "0123456789ABCDEF";

        for (uint8_t word = 0; word < 3; ++word) {
            for (uint8_t nibble = 0; nibble < 8; ++nibble) {
                uint8_t shift = (uint8_t)(28U - nibble * 4U);
                string_descriptor[1 + count++] = hex[(uid[word] >> shift) & 0xFU];
            }
        }
    } else {
        if (index >= sizeof(strings) / sizeof(strings[0])) {
            return NULL;
        }

        char const *value = strings[index];
        while (value[count] != '\0' && count < 31U) {
            string_descriptor[1 + count] = (uint8_t)value[count];
            ++count;
        }
    }

    string_descriptor[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2U * count + 2U));
    return string_descriptor;
}
