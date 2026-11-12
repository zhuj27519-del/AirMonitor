"""Run with editor closed. Match TI RVC0020A and DDC0006A reference lands.
Native TPcbPad/TPcbFill documentation is preserved in verification/.
"""
from pathlib import Path
import json,math
r=Path(__file__).resolve().parents[1]
def mil(v):return v/.0254
def rectangle(x,y,w,h):
 x0,y0,x1,y1=map(mil,(x-w/2,y-h/2,x+w/2,y+h/2))
 return [x0,y0,'L',x1,y0,x1,y1,x0,y1,x0,y0]
def u3loc(n):
 if 1<=n<=6:return (-1.25+.5*(n-1),-1.4,.2,.6)
 if 11<=n<=16:return (1.25-.5*(n-11),1.4,.2,.6)
 if 7<=n<=10:return (1.9,-.75+.5*(n-7),.6,.2)
 return (-1.9,.75-.5*(n-17),.6,.2)
changes=[]
for p in [r/'hardware/EasyEDA/pcb/PCB1.epcb2',r/'hardware/EasyEDA/sch/Schematic1/P1.esch2']:
 lines=p.read_text(encoding='utf-8').splitlines();doc={};pads=0;paste=0;u4=0
 for i,line in enumerate(lines):
  if '||' not in line:continue
  h,b=line.split('||',1)
  if not b.startswith('{'):continue
  head=json.loads(h);body=json.JSONDecoder().raw_decode(b)[0];changed=False
  if head['type']=='DOCHEAD':doc=body
  uid=doc.get('uuid')
  if doc.get('docType')=='FOOTPRINT' and uid=='96b152974707cdac':
   if head['type']=='PAD':
    n=int(body['num']);pads+=1
    if n==21:
     body.update(centerX=0,centerY=0,padOffsetX=0,padOffsetY=0,connectMode='DIRECT')
     body['defaultPad'].update(width=mil(1.6),height=mil(2.6))
    else:
     x,y,w,h=u3loc(n)
     body.update(centerX=mil(x),centerY=mil(y),padOffsetX=0,padOffsetY=0)
     body['defaultPad']={'padType':'RECT','width':mil(w),'height':mil(h),'radius':0}
    changed=True
   if head['type']=='FILL' and body['layerId']==7:
    z=body['zIndex'];paste+=1
    if z==56:body['path']=[rectangle(-.675,0,1.15,1.47),rectangle(.675,0,1.15,1.47)]
    else:
     order={**dict(zip(range(57,63),range(16,10,-1))),**dict(zip(range(63,69),range(6,0,-1))),**dict(zip(range(69,73),range(20,16,-1))),**dict(zip(range(73,77),range(10,6,-1)))}
     assert z in order,z
     body['path']=[rectangle(*u3loc(order[z]))]
    body['width']=0;changed=True
  if doc.get('docType')=='FOOTPRINT' and uid=='077fd60b0473b66b' and head['type']=='PAD':
   n=int(body['num']);u4+=1
   # Library orientation is rotated 180 degrees from TI's drawing, not mirrored.
   x=1.35 if n<=3 else -1.35;y={1:-.95,2:0,3:.95,4:.95,5:0,6:-.95}[n]
   body.update(centerX=mil(x),centerY=mil(y),padOffsetX=0,padOffsetY=0,topSolderExpansion=mil(.05),topPasteExpansion=0)
   body['defaultPad']={'padType':'RECT','width':mil(1.1),'height':mil(.6),'radius':0};changed=True
  if head['type']=='ATTR' and body.get('parentId')=='e55694400726d0e2' and body.get('key')=='Version':body['value']='V1';changed=True
  if changed:
   head['ticket']+=300000
   lines[i]=json.dumps(head,separators=(',',':'))+'||'+json.dumps(body,ensure_ascii=False,separators=(',',':'))+'|'
 assert (pads,paste,u4)==(21,21,6),(p,pads,paste,u4)
 p.write_text('\n'.join(lines)+'\n',encoding='utf-8')
 changes.append({'file':str(p.relative_to(r)),'u3_pads':pads,'u3_paste_shapes':paste+1,'u4_pads':u4,'u3_ep_paste_coverage_percent':2*1.15*1.47/(2.6*1.6)*100})
(r/'verification/v1-native-footprint-edits.json').write_text(json.dumps(changes,indent=2),encoding='utf-8');print(json.dumps(changes,indent=2))
