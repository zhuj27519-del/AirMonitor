"""Correct J4 anchor centres to JST PH drawing: 1.6 mm to inner edge + 0.75 mm half-width.
Run with EDA closed. Preserve the signal pads and their orientation.
"""
from pathlib import Path
import json
r=Path(__file__).resolve().parents[1];changes=[]
for p in [r/'hardware/EasyEDA/pcb/PCB1.epcb2',r/'hardware/EasyEDA/sch/Schematic1/P1.esch2']:
 lines=p.read_text(encoding='utf-8').splitlines();doc={};count=0
 for i,line in enumerate(lines):
  if '||' not in line:continue
  h,b=line.split('||',1)
  if not b.startswith('{'):continue
  head=json.loads(h);body=json.JSONDecoder().raw_decode(b)[0]
  if head['type']=='DOCHEAD':doc=body
  if doc.get('uuid')=='0c67bbc8fc49b79f' and head['type']=='PAD' and body['num'] in ['9','10']:
   old=body['centerX'];body['centerX']=(1 if old>0 else -1)*9.35/.0254
   head['ticket']+=400000;count+=1
   lines[i]=json.dumps(head,separators=(',',':'))+'||'+json.dumps(body,ensure_ascii=False,separators=(',',':'))+'|'
   changes.append({'file':str(p.relative_to(r)),'pin':body['num'],'old_x_mm':old*.0254,'new_x_mm':body['centerX']*.0254})
 assert count==2,(p,count)
 p.write_text('\n'.join(lines)+'\n',encoding='utf-8')
(r/'verification/v1-j4-anchor-edits.json').write_text(json.dumps(changes,indent=2),encoding='utf-8');print(json.dumps(changes,indent=2))
