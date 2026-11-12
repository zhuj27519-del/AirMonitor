"""Read-back of persisted native engineering fields, not in-memory API setters."""
from pathlib import Path
import json
r=Path(__file__).resolve().parents[1]
doc={};pads={};phys=[];holes=[]
for line in (r/'hardware/EasyEDA/pcb/PCB1.epcb2').read_text(encoding='utf-8').splitlines():
 if '||' not in line:continue
 h,b=line.split('||',1)
 if not b.startswith('{'):continue
 head=json.loads(h);body=json.JSONDecoder().raw_decode(b)[0]
 if head['type']=='DOCHEAD':doc=body
 if head['type']=='PAD' and doc.get('docType')=='FOOTPRINT':
  pads[(doc['uuid'],body.get('num'))]=body
 if doc.get('docType')=='PCB':
  if head['type']=='LAYER_PHYS':phys.append({'id':head['id'],**body})
  if head['type']=='PAD' and body.get('plated') is False:holes.append(body)
usb=[pads[('62f5990f8876e5a1',n)]['defaultPad']['width']*.0254 for n in ['A9','B9']]
thermal=[pads[k].get('connectMode') for k in [('a3dfe8fc6158c620','4'),('96b152974707cdac','21')]]
result={'usb_power_land_width_mm':usb,'thermal_pad_connect_modes':thermal,'physical_layers':phys,'npth_holes':holes}
result['persisted_field_checks_passed']=all(abs(v-.76)<.0001 for v in usb) and thermal==['DIRECT','DIRECT'] and len(holes)==8 and any(x.get('material')=='FR4' for x in phys)
(r/'verification/p1-native-readback.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
print({k:v for k,v in result.items() if k not in ['physical_layers','npth_holes']})
assert result['persisted_field_checks_passed']
