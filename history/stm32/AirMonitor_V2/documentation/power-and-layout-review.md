# Power and layout review — V2-P1

Reviewed 2026-10-27. This is an engineering design review, not a hardware test report.

## Revised power circuit

USB4125 VBUS → F1 (1206L050YR, 0.5 A hold) → 5V_FUSED → U3 TPS25940LRVCR → 5V → U2 MCP1826S-3302E/DB → 3V3. PMS receives 5V; MCU, VOC and LCD receive 3V3. CC1 and CC2 retain separate 5.1k pull-downs. This power-only USB-C connector does not enumerate or negotiate PD. Use a verified 5V supply with adequate current.

D1 SMF5.0A is now after the PTC and before the eFuse. This corrects P0's unfused TVS branch and removes the TVS-clamped node from the LDO input. The published 9.2V TVS clamp is below the eFuse 20V absolute maximum, but this is only component coordination, not surge immunity certification. A sustained inappropriate supply must also remain within the TVS/PTC energy ratings.

TPS25940L: IN pins 9–13 and EN14 on 5V_FUSED; OUT4–8 on 5V; GND16 and exposed pad21 grounded; DEVSLP1 and PGTH3 grounded; PGOOD2, IMON19 and FLT20 unused. R10=150k sets nominal 0.58A limit (TI bounds 0.53–0.63A at nominal resistance). C12=100nF sets approximately 42ms nominal 5V output ramp. It provides reverse-current blocking and latched thermal/overload shutdown; cycling input power resets a latched fault. It does not automatically recover every fault.

R7=44.2k 0.1%, R8=100 ohm 1%, R9=10k 0.1% set the OVP divider. Nominal rising threshold is 5.3757V. Including comparator 0.97–1.01V and ±100nA leakage plus initial resistor tolerances gives **5.2540–5.4978V**, before resistor temperature drift and dynamic overshoot. Reproducible calculation: automation/power_corners.py and verification/power-corners.json.

**OVP margin is not fully closed.** Only 2.2mV remains below a 5.5V supply limit in the static upper corner; this is insufficient to promise a maximum 5.5V sensor supply over temperature and transients. A release must either adopt a tighter threshold circuit or establish the actual module's allowed transient envelope and qualify it. Do not treat the typical 5.376V threshold as a guaranteed clamp. The lower corner similarly leaves little margin above 5.25V.

## Load and thermal review

PMS7003M manual specifies ≤100mA at 5V, but the user has now selected an eight-pin module interface; confirm its real operating/startup current. User's 21VOC sheets give <40mW, equivalent to <12.2mA at 3.3V. MCU allocation is 30mA, an engineering estimate. Waveshare's maximum module/backlight current is not specified in the obtained documentation; measure at full backlight.

For an illustrative LCD allocation of 60mA, the LDO load is about 102mA and dissipation at 5.25V about 0.199W. At 150mA and 250mA it becomes 0.293W and 0.488W. The MCP1826S SOT-223 reference 62°C/W is a four-layer JEDEC condition, **not this two-layer PCB's measured thermal resistance**. The 1A headline rating is not the board's continuous-current guarantee. C5/C6 provide 4.7µF nominal input/output decoupling; verify effective capacitance and stability at bias.

The regulator ground tab and eFuse exposed ground pad use direct copper connections with adjacent thermal vias. No via-in-pad assembly is required. Measure LDO, PTC and eFuse temperatures at the intended maximum enclosure ambient and at maximum sustained load. Check undervoltage at the PMS during fan startup and cable/fuse voltage drop.

## Board and mechanical layout

Outline: 102×80mm, X=−2…100mm, Y=0…80mm. Main mounting holes Ø3.2 NPTH at (4,4), (96,4), (96,76), (4,76). USB mating face aligns with X=−2mm; USB4125-GF-A-0190 stakes are 1.90mm, suitable for the specified board thickness.

Editor physical layer manager was inspected and applied: 1.6mm total; 35µm top/bottom copper, FR4 core 1.51mm, solder masks 10µm each. These are design parameters, not a board-factory tolerance guarantee. Saved LAYER_PHYS records provide the persistent configuration; the configuration-list API returns no named presets.

Waveshare 18079 official drawing confirms 50×35mm envelope, 44×29mm hole centres and 3mm module holes. PCB has Ø3.2mm holes H5–H8 at (33,30.5), (77,30.5), (77,59.5), (33,59.5). Use M2.5 nylon hardware, proposed 5mm maximum spacer OD and 8mm height; actual height, connector access and cable bend radius need fit checking. J3 and J5 were moved to clear this mounting arrangement. Sensor airflow and enclosure remain external mechanical work.

Power nets use 0.5mm main routing with local fine-pitch neckdowns. Minimum clearance remains 0.20mm. Final route counts, copper-fill evidence and DRC are in verification/report.md. Back preview retains top-view coordinates, not mirrored assembly orientation. A DRC pass does not establish thermal, EMC or complete land-pattern acceptance.
