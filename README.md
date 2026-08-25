<div align="center">

# PEEP

### A tiny, open-source, optically controlled USB HID automation device

**how small can a useful USB automation device get?**


## How it works

```text
Optical remote
      ↓ 38 kHz infrared
Photodiode + VSOP38338 receiver
      ↓ decoded pulse timing
STM32F042 macro engine
      ↓ USB HID reports
Computer sees a standard keyboard
```

PEEP is powered directly by USB. The receiver removes the 38 kHz carrier and sends the pulse envelope to the microcontroller, where firmware can decode a command, select a stored macro, and emit ordinary USB HID keyboard reports.

## Hardware

- 12.06 × 8 mm, four-layer, 0.8 mm PCB
- Direct USB-A edge connection
- STM32F042G6 Cortex-M0 MCU
- Crystal-less USB Full Speed
- VEMD10940F photodiode
- VSOP38338 38 kHz IR receiver frontend
- USB DFU recovery using the onboard BOOT0 flash points
- Open KiCad 10 design and production files

<!--
BOARD PHOTOS
-->

## Why open source?

PEEP is partly a useful tool and partly an experiment in extreme hardware miniaturization. The design files, production outputs, firmware, protocol, and host tools are intended to remain inspectable and reproducible.

Pretty please use PEEP only with systems you own or are explicitly authorized to test.

