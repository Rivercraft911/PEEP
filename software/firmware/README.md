# PEEP firmware

STM32F042G6 firmware using TinyUSB. The demo targets Windows 11 with a US ANSI keyboard layout.

## Demo

PEEP appears as `PEEP Keyboard`. An IR burst starts the demo:

1. Opens `griffinfacepalm95.exe` through Windows Search.
2. Opens Slack, selects Julian Sanders, and sends the configured message.
3. Waits 15 seconds, then opens the configured URL in the default browser.

Edit the executable, recipient, message, and URL in [src/app.c](src/app.c) before flashing. Use only on systems you own or have permission to test.

There is no auto-start timer. Once USB HID is ready and the receiver has been idle for 250 ms, PEEP arms. A falling edge on PA1 starts the macro. IR is ignored during playback; afterward, another 250 ms of idle time is required before a new trigger.

USB loss, suspend, or a DFU request aborts playback and disarms IR. Held keys are released when USB is available again.

This is a burst trigger, not a packet decoder. There are no command addresses or authentication; another IR remote can trigger it. The IR path still needs verification on the board.

## Build

Requirements: CMake 3.23+, Ninja, and the Arm GNU bare-metal toolchain. On macOS:

```sh
brew install --cask gcc-arm-embedded
```

Initialize dependencies and build from the repository root:

```sh
git submodule update --init --recursive
cmake -S software/firmware -B software/firmware/build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build software/firmware/build
```

Outputs: `peep.elf`, `peep.bin`, `peep.hex`, and `peep.map`. The linker enforces a 24 KiB flash / 4 KiB RAM budget.

Run native tests with:

```sh
cmake -S software/firmware/tests -B software/firmware/build-tests -G Ninja
cmake --build software/firmware/build-tests
ctest --test-dir software/firmware/build-tests --output-on-failure
```

Tests cover IR arming, held-low input, duplicate bursts, repeated runs, USB readiness/loss, aborts, and timer wraparound, along with ASCII mapping and macro playback.

## Flash

The factory USB DFU bootloader is in ROM; only the application is flashed. On macOS, install `dfu-util` with `brew install dfu-util`.

For the first install or recovery:

1. Unplug PEEP.
2. Bridge TP1 (`BOOT0`) to TP2 (`3.3V`).
3. Plug PEEP in and confirm ROM DFU mode:

   ```sh
   dfu-util -l
   ```

   The ROM bootloader should appear as `0483:df11`.

4. Flash the application:

   ```sh
   dfu-util -a 0 -s 0x08000000:leave \
     -D software/firmware/build/peep.bin
   ```

5. Unplug PEEP, remove the bridge while unpowered, and reconnect it.

For later updates, rebuild and run:

```sh
software/firmware/tools/flash.sh
```

The script requests DFU mode. The firmware releases keys, erases the first application page from RAM, and resets. The STM32 empty-flash check then selects ROM DFU. If the application won't enumerate, use the physical bridge.

Never solder or desolder while powered. Bootloader details: [ST AN2606](https://www.st.com/resource/en/application_note/an2606-stm32-microcontroller-system-memory-boot-mode-stmicroelectronics.pdf) and [ST RM0091](https://www.st.com/resource/en/reference_manual/dm00031936-stm32f0x1stm32f0x2stm32f0x8-advanced-armbased-32bit-mcus-stmicroelectronics.pdf).

`CAFE:5045` is a development-only USB VID/PID. Replace it with assigned values before distributing hardware.
