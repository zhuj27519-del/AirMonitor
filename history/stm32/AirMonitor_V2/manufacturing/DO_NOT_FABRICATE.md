# V2-P0 — NOT RELEASED FOR FABRICATION

These are real native-editor exports for review, not approved manufacturing data.

Four USB-C footprint clearance violations remain. GND pour outlines have no filled copper. Display mounting, exact PMS7003M cable, complete footprint overlays, supply/fault/thermal checks and CPL rotations remain open. See ../verification/report.md.

The Gerber ZIP contains both copper layers, masks, paste, silk, outline, drill data and flying-probe data. The separate DRL files are extracted byte-for-byte from that ZIP. BOM-native.csv is tab-delimited despite its extension; CPL-native.csv retains the editor's original format. Do not silently alter CPL angles or mirror coordinates. J5 is through-hole, not an ordinary SMT placement.

Intended stack: two-layer FR-4, nominal 1.6mm, 1oz copper. Final board size: 102×80mm. No fabrication order or purchase has been submitted.
