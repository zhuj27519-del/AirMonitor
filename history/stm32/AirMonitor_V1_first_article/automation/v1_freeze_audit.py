"""Read-only final native-resource checks; run after saving final EDA documents."""
from pathlib import Path
import json,hashlib
r=Path(__file__).resolve().parents[1]
def records(p):
 doc={}
 for line in p.read_text(encoding='utf-8').splitlines():
  if '||' not in line:continue
  h,b=line.split('||',1)
  if not b.startswith('{'):continue
  hd=json.loads(h);v=json.JSONDecoder().raw_decode(b)[0]
  if hd['type']=='DOCHEAD':doc=v
  yield doc,hd,v
files=[r/'hardware/EasyEDA/pcb/PCB1.epcb2',r/'hardware/EasyEDA/sch/Schematic1/P1.esch2'];checks={};resources=[]
for f in files:
 pads={};paste=[]
 for d,h,v in records(f):
  if d.get('docType')=='FOOTPRINT':
   if h['type']=='PAD':pads[(d['uuid'],v['num'])]=v
   if d['uuid']=='96b152974707cdac' and h['type']=='FILL' and v['layerId']==7:paste+=v['path']
 near=lambda a,b:abs(a-b)<.001
 u3=[pads[('96b152974707cdac',str(n))] for n in range(1,21)]
 u4=[pads[('077fd60b0473b66b',str(n))] for n in range(1,7)]
 c={'usb_power_lands_0p76mm':all(near(pads[('62f5990f8876e5a1',n)]['defaultPad']['width']*.0254,.76) for n in ['A9','B9']),
 'u3_twenty_rect_lands_0p2_by_0p6':all(p['defaultPad']['padType']=='RECT' and all(near(a,b) for a,b in zip(sorted([p['defaultPad']['width']*.0254,p['defaultPad']['height']*.0254]),[.2,.6])) for p in u3),
 'u3_paste_apertures_22':len(paste)==22,
 'thermal_pads_direct_ground':all(pads[k]['connectMode']=='DIRECT' for k in [('96b152974707cdac','21'),('a3dfe8fc6158c620','4')]),
 'u4_lands_1p1_by_0p6':all(near(p['defaultPad']['width']*.0254,1.1) and near(p['defaultPad']['height']*.0254,.6) for p in u4),
 'j4_anchor_x_9p35mm':all(near(abs(pads[('0c67bbc8fc49b79f',n)]['centerX'])*.0254,9.35) for n in ['9','10'])}
 checks[str(f.relative_to(r))]=c
 resources.append({k:v for k,v in pads.items() if k[0] in ['96b152974707cdac','077fd60b0473b66b','62f5990f8876e5a1','0c67bbc8fc49b79f']})
assert resources[0]==resources[1],'Schematic and PCB embedded pad resources differ'
checks['embedded_pad_resources_identical']=True
checks['all_passed']=all(all(v.values()) for v in checks.values() if isinstance(v,dict))
assert checks['all_passed'],checks
fw={str(p.relative_to(r/'firmware')):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted((r/'firmware').rglob('*')) if p.is_file() and p.suffix in ['.c','.h','.s','.ld','.elf','.hex','.bin']}
(r/'verification/v1-firmware-freeze.json').write_text(json.dumps(fw,indent=2),encoding='utf-8')
(r/'verification/v1-native-final-checks.json').write_text(json.dumps(checks,indent=2),encoding='utf-8');print(json.dumps(checks,indent=2))
