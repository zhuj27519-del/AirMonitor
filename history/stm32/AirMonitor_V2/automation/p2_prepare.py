from pathlib import Path
import json
r=Path(__file__).resolve().parents[1]
p=r/'hardware/pin_plan.json';plan=json.loads(p.read_text(encoding='utf-8'))
plan['U3']['14']='EF_EN'
plan.update({'U4':{'2':'GND','3':'GND','4':'OVP_PRECISE','5':'5V_FUSED','6':'EF_EN'},'R11':{'1':'5V_FUSED','2':'EF_EN'},'R12':{'1':'5V_FUSED','2':'OVP_PRECISE'},'R13':{'1':'OVP_PRECISE','2':'GND'},'C14':{'1':'5V_FUSED','2':'GND'}})
p.write_text(json.dumps(plan,indent=2)+'\n',encoding='utf-8')
print('Components',len(plan),'planned connections',sum(map(len,plan.values())))
# Worst-case independent initial tolerance and temperature coefficient, -40..125C.
e=.001+25e-6*100
lo=124000*(1-e);hi=124000*(1+e)
blo=10000*(1-e);bhi=10000*(1+e)
d={'revision':'P2','temperature_c':[-40,125],'reference_temperature_c':25,
 'resistor_fractional_bound':e,'precision_ovp_nominal_v':.4*(1+124000/10000),
 'precision_ovp_rising_corners_v':[.396*(1+lo/bhi)-15e-9*lo,.404*(1+hi/blo)+15e-9*hi],
 'precision_ovp_falling_corners_v':[.387*(1+lo/bhi)-15e-9*lo,.400*(1+hi/blo)+15e-9*hi],
 'scope':'Static rising/falling threshold bounds using TI full-temperature comparator limits, +/-15nA INB leakage, independent 0.1% and 25ppm/C resistor tolerances. No aging, transient or PCB leakage allowance.',
 'backup_ovp_nominal_v':.99*(1+(44200+1000)/10000),
 'startup_max_us':450,'nominal_5v_ramp_ms':5*100e-9/(1e-6*11.9)*1000,
 'transient_scope':'TLV6700 propagation delay is typical only. Internal TPS25940 OVP remains active as a faster backup; no guaranteed instantaneous 5.5V clamp is claimed. Verify hot-plug and startup on hardware.',
 'sources':['https://www.ti.com/lit/ds/symlink/tlv6700.pdf','https://www.ti.com/lit/ds/symlink/tps25940.pdf']}
(r/'verification/p2-power-corners.json').write_text(json.dumps(d,indent=2),encoding='utf-8');print(json.dumps(d,indent=2))
