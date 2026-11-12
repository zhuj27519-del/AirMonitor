"""Analytical bounds, not physical measurements or a transient simulation."""
from pathlib import Path
import json
top_lo=44200*.999+100*.99
top_hi=44200*1.001+100*1.01
data={
 'ovp_nominal_v':.99*(1+44300/10000),
 'ovp_initial_resistor_corners_v':[.97*(1+top_lo/(10000*1.001))-100e-9*top_lo,1.01*(1+top_hi/(10000*.999))+100e-9*top_hi],
 'ovp_scope':'Comparator full-temperature bounds and leakage; resistor initial tolerances only. Excludes resistor drift, switching delay and input overshoot.',
 'current_limit_a_at_nominal_150k':[.53,.58,.63],
 'nominal_5v_ramp_ms':5*100e-9/(1e-6*11.9)*1000,
 'ldo_dissipation_w':{str(i):(5.25-3.3)*i for i in [.102,.15,.25]},
 'thermal_scope':'SOT-223 62 C/W is a four-layer JEDEC reference, not measured two-layer thermal resistance.',
 'design_status':'Insufficient margin to guarantee 5.5 V at PMS over all temperatures and transients; characterize or revise before release.'
}
p=Path(__file__).resolve().parents[1]/'verification/power-corners.json'
p.write_text(json.dumps(data,indent=2),encoding='utf-8')
print(json.dumps(data,indent=2))
