"""Apply reviewed local footprint edits to embedded native resources while EDA is closed.

TPcbPad and EPadConnect official format responses are preserved in verification/.
Does not change pins, net names, placement, holes or library references.
"""
from pathlib import Path
import json
r=Path(__file__).resolve().parents[1]
changes=[]
for p in [r/'hardware/EasyEDA/pcb/PCB1.epcb2',r/'hardware/EasyEDA/sch/Schematic1/P1.esch2']:
 lines=p.read_text(encoding='utf-8').splitlines();doc={};count=0
 for i,line in enumerate(lines):
  if '||' not in line: continue
  h,b=line.split('||',1)
  if not b.startswith('{'): continue
  head=json.loads(h);body=json.JSONDecoder().raw_decode(b)[0]
  if head['type']=='DOCHEAD':doc=body
  if head['type']!='PAD' or doc.get('docType')!='FOOTPRINT':continue
  uid=doc.get('uuid');num=body.get('num');old=None
  if uid=='62f5990f8876e5a1' and num in ['A9','B9']:
   old={'width':body['defaultPad']['width']};body['defaultPad']['width']=.76/.0254
  elif (uid,num) in [('a3dfe8fc6158c620','4'),('96b152974707cdac','21')]:
   old={'connectMode':body.get('connectMode')};body['connectMode']='DIRECT'
  if old is not None:
   head['ticket']+=200000
   lines[i]=json.dumps(head,separators=(',',':'))+'||'+json.dumps(body,ensure_ascii=False,separators=(',',':'))+'|'
   changes.append({'file':str(p.relative_to(r)),'footprint':uid,'pad':num,'before':old,'width':body['defaultPad']['width'],'connectMode':body.get('connectMode')});count+=1
 assert count==4,(p,count)
 p.write_text('\n'.join(lines)+'\n',encoding='utf-8')
(r/'verification/p1-native-footprint-edits.json').write_text(json.dumps(changes,indent=2),encoding='utf-8')
print('Persisted',len(changes),'reviewed pad changes')
