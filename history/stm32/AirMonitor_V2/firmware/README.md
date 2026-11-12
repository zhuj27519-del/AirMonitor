# AirMonitor V2 firmware

STM32G0B1CBT6, Cortex-M0+, CMSIS register implementation. Import this folder with **File → Import → Existing Projects into Workspace** in STM32CubeIDE. The `Debug` configuration uses `build.py` as an external build command. No CubeMX `.ioc` or generated HAL is implied.

Verified with STM32CubeIDE 2.2.0 / GNU Arm GCC 14.3.1. See `../verification/cubeide-build-final.txt`: **Build Finished, 0 errors, 0 warnings**. Output: `build/AirMonitor_V2.elf`, `.hex`, `.bin`, `.map`. Flash allocation is 128 KiB; linker uses the first 128 KiB SRAM conservatively. HSI16 and SysTick 1 ms; no external crystal.

Command-line build: `python build.py`. `ARM_GCC` can override the compiler path. The script locates the installed Windows CubeIDE compiler under `C:/ST`; on another machine update `.cproject`'s Python builder path and set `ARM_GCC`. `Build.launch` is a retained alternative external builder, not the active configuration.

UART1: PMS, 9600 8N1. UART2: VOC, 9600 8N1. Interrupt-fed 1024-byte rings keep receiving during blocking SPI draws. Overflow and UART error counters are shown on the diagnostic page. A bad packet never replaces the last valid reading. Measurements older than 10 s are hidden behind a stale/missing message. PMS readings are withheld during the first 30 s after boot. No automatic active/passive-mode commands are sent because the user-selected eight-pin module has not yet been tested with real serial frames.

Display: Waveshare 18079, ST7789, portrait 240×240 RGB565, Mode 0, 2 MHz, CS=PA4. Initialization follows the vendor example with an explicit 120 ms sleep-out delay. Text uses an original compact bitmap font. KEY1=previous page; KEY2=next page; KEY3=diagnostics, 30 ms debounce. Basic pages show PM1/PM2.5/PM10, TVOC/HCHO estimate/eCO2/temperature/RH. The screen's `ECO2` label is the ASCII rendering of eCO₂; it is explicitly marked estimated.

Protocol conventions:

- VOC: `2C` + five big-endian 16-bit fields + 8-bit two's-complement checksum; sum of all 12 bytes is zero modulo 256. Fields: TVOC µg/m³, HCHO µg/m³, eCO2 ppm, temperature 0.1°C, RH 0.1%. Negative temperature uses the supplied sheet's **one's-complement magnitude**, not C's two's-complement casting. Out-of-range VOC, temperature or humidity frames are rejected.
- PMS32 parser (checked against PMS7003M V2.5): `42 4D`, length 28, total 32 bytes, big-endian 16-bit checksum over bytes 0–29. Uses atmospheric PM fields at offsets 10/12/14, not CF=1 values. Nonzero sensor error byte is rejected. The user confirmed the eight-pin connector shown in the supplied image; verify actual captured frames before claiming compatibility with that module. Connector pin count alone does not establish the protocol.

Host tests execute actual compiled ARM parser code with Unicorn. Install `unicorn==2.1.4` in a local Python environment, then run `python tests/test_protocol.py`; optionally pass the package directory. Tests cover field order, units, one's-complement negative temperature, range checks, every single-byte corruption, wrong lengths, sensor errors, truncated/corrupt stream recovery and random noise. 107 assertions passed in the recorded run.

These are software build and host protocol tests, **not real-hardware validation**. UART electrical timing, LCD orientation/colors, boot option bytes, button operation, power and temperature remain to be tested on a board.

Vendor sources: ST's `cmsis-device-g0` official repository (device header/startup/system file) and the installed STM32Cube H7 package's generic CMSIS core headers. Their licenses are preserved under `vendor/`. Main application, parsers and compact font were authored for this project.
