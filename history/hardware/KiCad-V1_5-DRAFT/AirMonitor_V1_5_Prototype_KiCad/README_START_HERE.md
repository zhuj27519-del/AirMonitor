# Air Monitor V1.5 — editable KiCad schematic + routed PCB engineering prototype

## DELIVERED
- `AirMonitor_V1_5.kicad_sch` — native KiCad schematic with net labels and symbolic component pins.
- `AirMonitor_V1_5.kicad_pcb` — real editable KiCad board with **279 copper traces, 22 vias**, 100 × 80mm board edge, a dual-layer ground zone definition (must be filled within KiCad), and mounted connector/component pads.
- `AirMonitor.pretty/` — project-local custom footprints; `fp-lib-table` registers them.
- `BOM.csv`, `net_connections.csv` — editable part inventory and pin-to-net mapping.
- `routing_report.json`, `independent_geometric_review.json` — computer-checked routing connections and approximate copper clearances.
- `PCB_front_routed.png`, `PCB_back_routed.png` — CAD-data-derived routing previews.

## HARD STATUS: NOT YET APPROVED FOR FABRICATION
This version goes beyond the prior placement-only board by adding two-layer routing. However this development environment has NO installed KiCad executable; therefore **KiCad ERC, true DRC, and ground zone refill cannot be run here**. No production Gerbers, drill files, or fabrication pass statement are supplied. Independently computed copper shorts (not equivalent to KiCad DRC): 0; gaps <0.20mm: 23. Routing net connections (excluding GND zone): 48 / 48. If any missing routes are shown in `routing_report.json`, they MUST be finished interactively.

## Chosen real design
- MCU: STM32G0B1CBT6, 3.3 V, LQFP48; UART1 PA9/PA10 for PM sensor, UART2 PA2/PA3 for 21VOC, SPI1 PA5/PA7 with GPIO for TFT.
- **Exactly two sensing modules**: PMS7003M (case mounted via a 10-pin keyed breakout lead), and CJSLC 21VOC module (direct 4x2.0mm interface, 3.3V). 21VOC gives TVOC, HCHO-like numeric output, eCO2, temperature and RH. **eCO2 is an estimate, NOT true NDIR CO2.**
- TFT: Waveshare SKU18079 1.54in ST7789 240×240 (separate panel bolted to case) through an 8-pin 2mm-pitch cable; front PCB hosts sensing and MCU, rear display.
- Supply: USB-C 5V **power only** (CC1/CC2 each 5.1k pull-down) -> resettable fuse -> AP2112K-3.3 LDO. PM takes 5V; MCU 21VOC and LCD take 3.3V. SWD, buttons, reset, extra test header.

## MUST VERIFY before ordering a PCB
1. **USB-C footprint J1 must be replaced with a manufacturer's confirmed footprint.** The schematic reflects TYPE-C-31-M-12 nominal electrical pins, but the custom land pattern in this proof of concept is NOT production-safe: existing A/B mirrored contact pads were superposed, and the shell/body tabs are placeholders. Use the official KiCad `Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12` for your exact bought connector, re-run routing.
2. Confirm **PMS7003M variant & 10-pin harness**. J2 is a 2x5 2.54mm *adapter/header*, not the actual thin sensor connector. The PM module in housing has to be physically mounted with airflow unobstructed. Conventional PMS7003 uses 5 V power + UART 3.3V signals, but validate this specific suffix / cable pin1.
3. J3 4×2.0mm 21VOC is electrically grounded in vendor sheet, but connector/board mating positions and top/bottom are mechanically provisional. Confirm with paper template and calipers.
4. J4 8×2.0mm is an electrical pin-header placeholder for Waveshare SKU18079 PH2.0-8P; replace it with keyed PH2.0 mating PCB receptacle after confirming its footprint. Verify pin1 at the panel end before powering. LCD is NOT directly attached to rear copper pads.
5. AP2112K may dissipate significant heat at large backlight loads. Verify max module backlight draw, thermal margin, and set 3V3 reservoir decoupling close to LCD connector.
6. Verify all LQFP48 pin numbers and GPIO AF mapping against ST datasheet, solder pin-1 identification, BOOT options, and SWD NRST wiring.
7. Use a real KiCad installation: Tools > Electrical Rules Checker in schematic; PCB Editor > Update PCB from schematic, refill zones (`B`), inspect ratlines, Run DRC, inspect dimensions/silkscreen and 1:1 PDF print, then generate fab files only after a 0-error DRC.
8. Mechanically check case screw holes, particulate exhaust, gas sensor breathability (no siloxanes, glues, corrosive vapors), display standoffs and button access.

## Libraries and datasheets (verified externally)
- ST G0B1 pin table: https://www.st.com/resource/en/datasheet/stm32g0b1cc.pdf
- Plantower conventional PMS7003 pinout: https://www.espruino.com/PMS7003
- LCD SKU18079: https://docs.waveshare.net/1.54inch_LCD_Module/
- LDO AP2112K: https://www.diodes.com/datasheet/download/AP2112.pdf
- 21VOC pins, specs & UART protocol: vendor screenshots supplied by the user. 12B UART 9600 8N1 frame prefix 0x2C; eCO2 not direct CO2.
