"""Update only the documented native sheet Version attribute (API rejects sheet edits)."""
from pathlib import Path
import json
p=Path(__file__).resolve().parents[1]/'hardware/EasyEDA/sch/Schematic1/P1.esch2'
lines=p.read_text(encoding='utf-8').splitlines()
changed=0
for i,line in enumerate(lines):
 if '||' not in line: continue
 h,b=line.split('||',1)
 if not b.startswith('{'): continue
 head=json.loads(h); body=json.JSONDecoder().raw_decode(b)[0]
 if head['type']=='ATTR' and body.get('parentId')=='e55694400726d0e2' and body.get('key')=='Version':
  body['value']='V2-P1';head['ticket']+=100000
  lines[i]=json.dumps(head,separators=(',',':'))+'||'+json.dumps(body,ensure_ascii=False,separators=(',',':'))+'|'
  changed+=1
assert changed==1,changed
p.write_text('\n'.join(lines)+'\n',encoding='utf-8')
print('Updated one sheet revision attribute')
