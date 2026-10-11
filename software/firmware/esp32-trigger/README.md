# PEEP remote

The XIAO ESP32-C6 serves a trigger button on the local Wi-Fi network and sends three 9 ms, 38 kHz IR bursts.

PEEP must have USB HID ready and its receiver idle for 250 ms before it accepts a trigger. Bursts during macro playback are ignored.

## Wiring

For the Adafruit High Power IR LED Emitter (5639):

| XIAO ESP32-C6 | Emitter |
| --- | --- |
| `3V3` | `V+` |
| `GND` | `GND` |
| `D1` / GPIO1 | `In` |

For the Adafruit IR Transceiver (5990), connect D1 to `IRout` and leave `IRin` open. Both boards use an active-high input and include the LED driver. Do not drive a bare high-current IR LED from a GPIO.

Aim the emitter at PEEP's side-facing detector; the USB receptacle can block the signal.

## Build

1. Copy `secrets.example.h` to `secrets.h` and enter the Wi-Fi credentials. `secrets.h` is ignored by Git.
2. Install the Espressif ESP32 Arduino core 3.3.11.
3. Select `XIAO_ESP32C6`, open `esp32-trigger.ino`, and upload.
4. Open `http://peep-trigger.local`. If mDNS is unavailable, use the IP printed at 115200 baud.

Anyone on the network can press the trigger; there is no authentication. Keep it on a trusted LAN and don't forward its port.

Hardware references: [Adafruit 5639](https://www.adafruit.com/product/5639), [Adafruit 5990](https://learn.adafruit.com/adafruit-infrared-ir-remote-transceiver/pinouts), [XIAO ESP32-C6](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/), [VEMD10940F](https://www.vishay.com/docs/84171/vemd10940f.pdf), and [VSOP38338](https://www.vishay.com/docs/82748/vsop383.pdf).
