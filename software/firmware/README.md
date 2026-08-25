# PEEP firmware

PEEP is an optical-triggered USB keyboard for authorized systems. Stage one targets Windows 11 with a US ANSI keyboard layout.

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

Outputs are `peep.elf`, `peep.bin`, `peep.hex`, and `peep.map`. The link fails above 24 KiB flash or 4 KiB RAM.

Run native tests with:

```sh
cmake -S software/firmware/tests -B software/firmware/build-tests -G Ninja
cmake --build software/firmware/build-tests
ctest --test-dir software/firmware/build-tests --output-on-failure
```

## Flash

The STM32 factory USB DFU bootloader is already in ROM. Install the host utility on macOS with `brew install dfu-util`.

1. Unplug PEEP.
2. Temporarily bridge TP1 (`BOOT0`) to TP2 (`3.3V`). Tweezers or pogo pins are safer than solder.
3. Plug PEEP in and confirm DFU mode:

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

Do not solder or desolder the bridge while powered. The bridge selects ROM DFU; the downloaded image is application firmware, not a replacement bootloader.

## Demo

Windows enumerates `PEEP Keyboard`. With the default browser configured and Notepad available:

1. Leave the IR receiver idle for 250 ms.
2. Send a 38 kHz IR burst; a typical 9 ms remote-control leader works.
3. After three seconds, PEEP opens the Rickroll URL.
4. PEEP opens Notepad on the right and types `Hello from PEEP.`

Triggers are ignored until the sequence finishes and the receiver has been idle for another 250 ms. USB suspend aborts the sequence and releases any held key after the host resumes.

`CAFE:5045` is a development-only USB VID/PID. Replace it with assigned values before distributing hardware.
