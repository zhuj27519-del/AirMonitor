"""Independent conservative KEY3 copper graph; no EDA DRC or netlist topology used."""
from pathlib import Path
import json,math,zipfile,collections
R=Path(__file__).resolve().parents[1]
d=json.loads((R/'verification/live-current.txt').read_text(encoding='utf-8'))['value']
def records(text):
 doc=None;out={}
 for line in text.splitlines():
  if '||' not in line:continue
  h,b=line.split('||',1);h=json.loads(h)
  if h['type']=='DOCHEAD':doc=json.JSONDecoder().raw_decode(b)[0]['uuid'];continue
  if not b.startswith('{'):continue
  v=json.JSONDecoder().raw_decode(b)[0];out[(doc,h['type'],str(h.get('id')))]=v
 return out
native=records((R/'hardware/EasyEDA/pcb/PCB1.epcb2').read_text(encoding='utf-8'))
with zipfile.ZipFile(R.parent/'AirMonitor_V1_first_article_review.zip') as z:
 old=records(z.read('AirMonitor_V1_first_article/hardware/EasyEDA/pcb/PCB1.epcb2').decode())
changes=[{'record':list(k),'before':old.get(k),'after':native.get(k)} for k in set(old)|set(native) if old.get(k)!=native.get(k)]
physical={'LINE','VIA','PAD','COMPONENT','COMPONENT_ATTR','PAD_NET','FILL','POUR','POURED','POLY','POLYLINE','ARC','HOLE','REGION','RULE','RULE_SELECTOR','RULE_CONFIG','RULE_CONFIG_NAME'}
important=[x for x in changes if x['record'][1] in physical]
lines=[x for x in d['lines'] if x['net']=='KEY3'];vias=[x for x in d['vias'] if x['net']=='KEY3']
pads=[{**p,'label':c['c']['designator']+'.'+p['padNumber']} for c in d['components'] for p in c['pads'] if p['net']=='KEY3']
assert {p['label'] for p in pads}=={'U1.44','R6.2','SW3.1'}
nv={k[2]:v for k,v in native.items() if k[0]=='0d571ee1a4bcf754'}
for l in lines:
 n=nv[l['primitiveId']]
 assert n['netName']==l['net'] and n['layerId']==l['layer']
 assert all(abs(n[k]-l[k])<1e-8 for k in ['startX','startY','endX','endY'])
 assert abs(n['width']-l['lineWidth'])<1e-8
def xy(p):return (p['x'],p['y'])
def endpoints(l):return [(l['startX'],l['startY']),(l['endX'],l['endY'])]
def dist(a,b):return math.hypot(a[0]-b[0],a[1]-b[1])
def pointseg(p,a,b):
 dx=b[0]-a[0];dy=b[1]-a[1];t=max(0,min(1,((p[0]-a[0])*dx+(p[1]-a[1])*dy)/(dx*dx+dy*dy))) if dx or dy else 0
 return dist(p,(a[0]+t*dx,a[1]+t*dy))
def inside_pad(p,pt):
 # Coordinates are read from official pin API, not reconstructed from placement.
 a=math.radians(-p['rotation']);dx=pt[0]-p['x'];dy=pt[1]-p['y'];x=dx*math.cos(a)-dy*math.sin(a);y=dx*math.sin(a)+dy*math.cos(a)
 shape,w,h,*_=p['pad']
 if shape=='RECT':return min(w/2-abs(x),h/2-abs(y))
 if shape=='OVAL':
  if w>=h:return h/2-pointseg((x,y),(-(w-h)/2,0),((w-h)/2,0))
  return w/2-pointseg((x,y),(0,-(h-w)/2),(0,(h-w)/2))
 raise ValueError(shape)
nodes={l['primitiveId']:{'type':'line','data':l} for l in lines}
nodes.update({p['label']:{'type':'pad','data':p} for p in pads});nodes.update({v['primitiveId']:{'type':'plated_through_via','data':v} for v in vias})
edges=[]
def edge(a,b,why,margin):edges.append({'a':a,'b':b,'basis':why,'overlap_margin_mm':margin*.0254})
for i,l in enumerate(lines):
 a,b=endpoints(l)
 for m in lines[i+1:]:
  if l['layer']!=m['layer']:continue
  c,e=endpoints(m);gap=min(pointseg(a,c,e),pointseg(b,c,e),pointseg(c,a,b),pointseg(e,a,b))
  margin=(l['lineWidth']+m['lineWidth'])/2-gap
  if margin>0:edge(l['primitiveId'],m['primitiveId'],'same-layer trace copper overlap',margin)
 for p in pads:
  if l['layer']!=p['layer']:continue
  margin=max(inside_pad(p,q) for q in [a,b])
  if margin>0:edge(l['primitiveId'],p['label'],'trace endpoint strictly inside actual pad copper',margin)
 for v in vias:
  assert v['viaType']==0
  near=pointseg(xy(v),a,b);far=max(dist(xy(v),a),dist(xy(v),b));outer=v['diameter']/2;inner=v['holeDiameter']/2;r=l['lineWidth']/2
  # Intersect annulus, not merely the drilled-out centre. Barrel links layers.
  margin=min(outer+r-near,far+r-inner)
  if margin>0:edge(l['primitiveId'],v['primitiveId'],'trace intersects annular copper of plated through via',margin)
def reachable(start,omit_via=False):
 adj={n:[] for n in nodes}
 for e in edges:
  if omit_via and any(nodes[n]['type']=='plated_through_via' for n in [e['a'],e['b']]):continue
  adj[e['a']].append(e['b']);adj[e['b']].append(e['a'])
 paths={start:[start]};q=[start]
 for n in q:
  for k in adj[n]:
   if k not in paths:paths[k]=paths[n]+[k];q.append(k)
 return paths
paths=reachable('U1.44');neg=reachable('U1.44',True)
assert all(p['label'] in paths for p in pads)
assert 'R6.2' not in neg and 'SW3.1' not in neg
result={'units':'mm in report; native inputs mil','native_live_key3_geometry_matches':True,'semantic_changes_vs_sealed_delivery':changes,'physical_or_rule_changes_vs_sealed_delivery':important,'all_key3_objects_reachable_from_U1_44':len(paths)==len(nodes),'three_required_pads_same_copper_component':all(p['label'] in paths for p in pads),'node_count':len(nodes),'trace_count':len(lines),'plated_via_count':len(vias),'pad_count':len(pads),'paths':{p['label']:paths[p['label']] for p in pads},'negative_control_remove_via_disconnects_mcu':True,'edges':edges,'key_objects':[{**l,'start_mm':[v*.0254 for v in endpoints(l)[0]],'end_mm':[v*.0254 for v in endpoints(l)[1]],'width_mm':l['lineWidth']*.0254} for l in lines if l['primitiveId'] in ['50ed7851da96bd02','3dc5ae6a88fe8cb1']],'method_limits':'Conservative geometry graph uses trace widths, actual pin API pad shapes, annular copper and plated barrel. No reliance on equal net names alone; no GND pour used to establish KEY3. Not an electrical measurement.'}
(R/'verification/key3-independent-connectivity.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps({k:v for k,v in result.items() if k not in ['edges','semantic_changes_vs_sealed_delivery','key_objects','paths']},ensure_ascii=False,indent=2))
