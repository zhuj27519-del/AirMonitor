"""Export a STEP larger than the CLI 8 MiB result limit in bounded chunks."""
from eda_cli import call,ROOT
import sys,base64,json
session=sys.argv[1]
code="""await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
const f=await eda.pcb_ManufactureData.get3DFile('AirMonitor_V1','step',['Component Model'],'Outfit',true);
if(!f)return {available:false};
globalThis.__airmonitorStep=await new Promise(r=>{const a=new FileReader();a.onload=()=>r(String(a.result).split(',')[1]);a.readAsDataURL(f);});
return {available:true,name:f.name,length:globalThis.__airmonitorStep.length};"""
v=call(['invoke','--session',session,'--ext-uuid','eda','--timeout','180000','--code',code],'v1-step-size',timeout=210)['value']
if not v.get('available'):raise RuntimeError('No STEP returned')
print(v,flush=True)
chunks=[]
for start in range(0,v['length'],1000000):
    chunk=call(['invoke','--session',session,'--ext-uuid','eda','--code',f'return globalThis.__airmonitorStep.slice({start},{start+1000000});'],'v1-step-transfer',timeout=30)['value']
    chunks.append(chunk);print('transferred',min(start+1000000,v['length']),flush=True)
p=ROOT/'hardware/AirMonitor_V1.step';p.write_bytes(base64.b64decode(''.join(chunks)))
(ROOT/'verification/step-transfer.txt').write_text(json.dumps({'file':p.name,'bytes':p.stat().st_size,'chunks':len(chunks)}),encoding='utf-8')
call(['invoke','--session',session,'--ext-uuid','eda','--code','delete globalThis.__airmonitorStep;return true;'],'v1-step-cache-released')
