from pathlib import Path
import json,math
r=Path(__file__).resolve().parents[1]
def value(n):return json.loads((r/'verification'/n).read_text(encoding='utf-8'))['value']
before=value('v1-drc-before-repair.txt')['errors'];rows=[]
for g in before:
 if g['name']=='Connection Error':
  for net in g['list']:
   for e in net['list']:rows.append({'net':net['name'],'pin':e['obj1']['suffix'],'position_mm':{k:round(v*.254,4) for k,v in e['pos'].items()},'classification':'Real required connection; not an allowed NC'})
state=value('v1-state-final.txt');power={}
for net in ['VBUS','5V_FUSED','5V','3V3']:
 lines=[l for l in state['lines'] if l['net']==net and l['layer'] in [1,2]]
 widths=sorted(set(round(l['lineWidth']*.0254,5) for l in lines))
 necks=[{'width_mm':l['lineWidth']*.0254,'length_mm':math.hypot(l['endX']-l['startX'],l['endY']-l['startY'])*.0254,'start_mm':[l['startX']*.0254,l['startY']*.0254],'end_mm':[l['endX']*.0254,l['endY']*.0254]} for l in lines if l['lineWidth']*.0254<.3]
 power[net]={'widths_mm':widths,'short_segments_below_0p3mm':necks,'max_short_segment_mm':max([v['length_mm'] for v in necks],default=0)}
d={'before_connection_errors':len(rows),'before_disconnected_smd_pads':21,'before_track_errors':2,'before_disconnected_nets':len(set(x['net'] for x in rows)),'before_pins':rows,'power_trace_geometry':power,'allowed_nc':['U3.2 PGOOD','U3.19 IMON','U3.20 FLT','U4.1 OUTA','Unused MCU GPIO pins declared NC'],'thermal_measurements_performed':False}
(r/'verification/v1-connection-and-width-audit.json').write_text(json.dumps(d,indent=2),encoding='utf-8')
print('Real disconnected pads before:',len(rows),'nets:',d['before_disconnected_nets'])
for n,v in power.items():print(n,v['widths_mm'],'max neck length',v['max_short_segment_mm'])
