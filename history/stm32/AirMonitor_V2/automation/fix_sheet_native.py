"""Update documented sheet ATTR records, preserving all native document blocks.
Official references: verification/format-sch-attr.txt and format-sch-component.txt.
Only the presentation frame is changed; no circuit/net data or ERC rules change.
"""
import json,pathlib,shutil
root=pathlib.Path(__file__).resolve().parents[1]
p=root/'hardware/EasyEDA/sch/Schematic1/P1.esch2'
backup=root/'verification/schematic-before-frame.esch2'
if not backup.exists():shutil.copy2(p,backup)
dec=json.JSONDecoder();lines=p.read_text(encoding='utf-8').splitlines();changed=[]
for i,line in enumerate(lines):
    if '||' not in line:continue
    head,body=line.split('||',1)
    if not body.startswith('{'):continue
    h=json.loads(head);b=dec.raw_decode(body)[0]
    if h['type']=='ATTR' and b.get('parentId')=='e55694400726d0e2':
        values={'Size':'A1','Page Size':'A1','Width':3311,'Height':2339,'Version':'V2-P0','Title Block':'1','Border':'1','Company':'AirMonitor'}
        if b.get('key') in values:
            b['value']=values[b['key']];h['ticket']+=100000
            lines[i]=json.dumps(h,ensure_ascii=False,separators=(',',':'))+'||'+json.dumps(b,ensure_ascii=False,separators=(',',':'))+'|'
            changed.append(b['key'])
p.write_text('\n'.join(lines)+'\n',encoding='utf-8')
print({'changed':changed})
