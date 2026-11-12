# Sources and part verification

Reviewed 2026-10-27. Library availability is not a stock or assembly-availability guarantee. No parts were ordered.

| Source | Evidence and scope |
|---|---|
| [ST STM32G0B1 datasheet, DS13560](https://www.st.com/resource/en/datasheet/stm32g0b1vb.pdf) | Covers the selected STM32G0B1CBT6; reviewed LQFP48 non-N pin assignment and AF mapping. Web retrieval worked; direct local PDF download was rejected by the server. |
| [ST CMSIS device repository](https://github.com/STMicroelectronics/cmsis-device-g0) | Official startup, device headers and system code saved in firmware/vendor, with license. Exact downloaded file hashes are recorded by the delivery manifest. Source was fetched from master, not a pinned release. |
| User's 21VOC sheets | Five original JPGs copied into datasheets/21VOC. Identifies 广州传苣电子有限公司, 3.3 V, <40 mW, UART 9600/8N1, 12-byte frame and one's-complement negative values. Missing sheets and actual connector mating part remain unresolved. |
| [Waveshare 18079 documentation](https://docs.waveshare.net/1.54inch_LCD_Module/) | Selected 1.54-inch, 240×240 ST7789 module; 50×35 mm envelope; 3.3 V operation and SPI Mode 0. |
| [Waveshare resources](https://docs.waveshare.net/1.54inch_LCD_Module/Resources-And-Documents/) | Manufacturer schematic saved as datasheets/Waveshare_18079_schematic.pdf. Initialization checked against the vendor LCD_1in54.c example. PH connector numbering must follow the schematic, not an assumed straight-through cable. |
| [GCT USB4125 drawing](https://gct.co/files/drawings/usb4125.pdf) | Current A6 drawing saved as GCT_USB4125.pdf. Six power/CC lands, four shell slots, mating-edge datum and 1.90mm stake option reviewed. USB4105 file retained only as P0 history. |
| [TI TPS25940 datasheet](https://www.ti.com/lit/ds/symlink/tps25940.pdf) | Saved locally. Pinout, OVP min/max and leakage, 150k current limit, dVdT, reverse blocking and latch behavior reviewed. |
| [Microchip MCP1826S datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/22057B.pdf) | Saved locally. SOT-223 pinout, input limits, minimum capacitance and reference thermal data reviewed. |
| [Waveshare size drawing](https://www.waveshare.com/img/devkit/LCD/1.54inch-LCD-Module/1.54inch-LCD-Module-details-size.jpg?v=26051401) | Saved as Waveshare_18079_dimensions.jpg: 50×35mm, 44×29mm mounting centres, Ø3mm module holes. |
| User eight-pin particulate-module image | Saved locally; user explicitly confirmed this interface on 2026-10-27. Defines electrical cable mapping but not a connector manufacturer part number or complete UART protocol. |
| [JST PH manufacturer drawing](https://www.jst-mfg.com/product/pdf/eng/ePH.pdf) | Reviewed online: 2 mm pitch, side-entry SMT family and PHR/SPH mating parts. Local download returned 403, so no local PDF is claimed. Library signal lands are about 1.0×3.5 mm; mounting-land details differ across the selected library entries and require final manufacturer-pattern overlay. |
| [Plantower PMS7003M manual mirror](https://pdf.elecfans.com/p/7222689.html) | Manufacturer-authored V2.5, 2016-07-02, saved as Plantower_PMS7003M_V2_5.pdf. Confirms existing firmware's 32-byte protocol and ten-pin alternative; user-selected eight-pin module still requires real-frame qualification. |
| Installed EasyEDA Pro docs | C:/Program Files/lceda-pro/docs/cli-for-ai.md and easyeda-format.md. Individual API/format queries are preserved under verification/. |

## PCB BOM

The native BOM includes all **37 PCB components** with manufacturer and LCSC part numbers. Its `.csv` extension is the editor's convention; the file content is tab-delimited. Import with UTF-8 and tab separator if required.

| Designators | Exact selected part | LCSC | Verification status |
|---|---|---|---|
| U1 | STM32G0B1CBT6 | C2847904 | Pin map checked; complete land-pattern overlay pending |
| U2 | MCP1826S-3302E/DB | C638624 | SOT-223, IN1/GND2/OUT3/tab4 GND; thermal qualification pending |
| U3 | TPS25940LRVCR | C2867756 | WQFN20+EP21, eFuse; pin mapping and EP size checked, stencil/thermal acceptance pending |
| J1 | USB4125-GF-A-0190 | C5246813 | GCT A6 drawing checked; 1.90mm stakes; A9/B9 lands corrected to 0.76×1.20mm |
| J2 | S6B-PH-SM4-TB(LF)(SN) | C265405 | 2 mm side-entry SMT family; harness provisional |
| J3 | S4B-PH-SM4-TB(LF)(SN) | C265102 | 2 mm side-entry SMT family; sensor mating connector unconfirmed |
| J4 | S8B-PH-SM4-TB(LF)(SN) | C265121 | Back side; custom signal-to-signal LCD cable required |
| J5 | B-2200S06P-A120 | C124415 | Through-hole 1×6 socket; manual assembly unless explicitly supported |
| F1 | 1206L050YR | C163512 | Nominal 0.5 A hold PTC; temperature derating pending |
| D1 | SMF5.0A-T13 | C1977665 | Cathode pin1 now on 5V_FUSED, anode2 GND; energy qualification pending |
| R1,R2 | 0603WAF5101T5E, 5.1k | C23186 | Independent CC pull-downs |
| R3–R6 | 0603WAF1002T5E, 10k | C25804 | Reset and key pull-ups |
| C1–C4,C9,C11,C12 | CC0603KRX7R9BB104, 100nF | C14663 | 0603 ceramic |
| C5,C6,C8,C10,C13 | CL10A475KO8NNNC, 4.7µF | C19666 | 0603 ceramic; effective capacitance under bias to be confirmed |
| R7 | RT0603BRD0744K2L, 44.2k 0.1% | C861410 | OVP upper resistor |
| R8 | 0603WAF1000T5E, 100Ω 1% | C22775 | OVP upper series trim |
| R9 | RT0603BRD0710KL, 10k 0.1% | C95204 | OVP lower resistor |
| R10 | 0603WAF1503T5E, 150k 1% | C22807 | eFuse current limit |
| C7 | CL10A105KB8NNNC, 1µF | C15849 | 0603 ceramic |
| SW1–SW4 | TS-1088-AR02026 | C52175904 | Library SMD switch; mechanical actuation/height review pending |

## External assembly BOM — not included in SMT CPL

| Item | Qty | Part / procurement status |
|---|---:|---|
| Particulate module | 1 | User-confirmed eight-pin interface from supplied photograph; use the matching module and supplied eight-pin harness. Do not substitute the PMS7003M ten-pin harness. |
| VOC module | 1 | User's 21VOC from supplied documentation; no verified supplier SKU supplied. |
| Display | 1 | Waveshare 18079 / 1.54inch LCD Module |
| Mainboard PMS cable housing | 1 | JST PHR-6 with suitable SPH-002T-P0.5S contacts; sensor end unresolved |
| Mainboard VOC cable housing | 1 | JST PHR-4 with suitable contacts; actual sensor end unresolved |
| LCD cable | 1 | Two compatible 8-way PH ends, wired according to interfaces.md; do not infer pin order from housing orientation |
| USB-C cable and supply | 1 each | 5 V source with sufficient confirmed current; no PD required |
| SWD adapter cable | 1 | Custom 1×6 mapping in interfaces.md |
| Display supports / fasteners | 4 sets | Proposed M2.5 nylon spacers, 8mm high, ≤5mm OD; matching screws/nuts, fit verification required |
| Enclosure | TBD | Sensor inlet/outlet clearance and cable routing not yet designed |

The external assembly BOM remains incomplete until mating connectors, cable lengths and fasteners are confirmed. The project is not a turnkey JLCPCB SMT order: both PCB sides contain SMT parts, J5 is through-hole, and the sensors/display are external modules.
