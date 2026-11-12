from pathlib import Path
import json,base64,zipfile,csv,io,hashlib
R=Path(__file__).resolve().parents[1];v=R/'verification';m=R/'manufacturing';m.mkdir(exist_ok=True)
d=json.loads((v/'candidate-exports.txt').read_text(encoding='utf-8'))['value']
paths={'schematic':'hardware/schematic.net','pcb':'hardware/pcb.net','bom':'manufacturing/BOM-native.csv','gerber':'manufacturing/AirMonitor_KEY3_Candidate_Gerber.zip','cpl':'manufacturing/CPL-native.csv'}
for k,path in paths.items():(R/path).write_bytes(base64.b64decode(d[k]['b64']))
with zipfile.ZipFile(m/'AirMonitor_KEY3_Candidate_Gerber.zip') as z:
 assert z.testzip() is None
 drills=[n for n in z.namelist() if n.endswith('.DRL')]
 assert len(drills)==3
 for n in drills:(m/Path(n).name).write_bytes(z.read(n))
 fills={n:z.read(n).count(b'G36') for n in ['Gerber_TopLayer.GTL','Gerber_BottomLayer.GBL']}
 assert min(fills.values())>0
def pins(p):
 d=json.loads(p.read_text(encoding='utf-8'));return {(c['props']['Designator'],str(p['number'])):p.get('net','') for c in d['components'].values() for p in c['pinInfoMap'].values()}
a=pins(R/'hardware/schematic.net');b=pins(R/'hardware/pcb.net');assert a==b
for key in [('U1','44'),('R6','2'),('SW3','1')]:assert a[key]=='KEY3'
def table(name):
 raw=(m/name).read_bytes();return list(csv.DictReader(io.StringIO(raw.decode('utf-16' if raw[:2] in [b'\xff\xfe',b'\xfe\xff'] else 'utf-8-sig')),delimiter='\t'))
bom=table('BOM-native.csv');cpl=table('CPL-native.csv')
refs={r.strip() for row in bom for r in row['Designator'].split(',')};assert len(refs)==len(cpl)==42 and refs=={r['Designator'] for r in cpl}
def write(name,fields,rows):
 with (m/name).open('w',encoding='utf-8-sig',newline='') as f:
  w=csv.DictWriter(f,fieldnames=fields);w.writeheader();w.writerows(rows)
rows=[{'Comment':r['Manufacturer Part'],'Designator':r['Designator'],'Footprint':r['Footprint'],'LCSC Part #':r['Supplier Part']} for r in bom]
write('BOM-JLCPCB-all.csv',['Comment','Designator','Footprint','LCSC Part #'],rows)
coords=[{'Designator':r['Designator'],'Mid X':r['Mid X'].removesuffix('mm'),'Mid Y':r['Mid Y'].removesuffix('mm'),'Layer':{'T':'Top','B':'Bottom'}[r['Layer']],'Rotation':r['Rotation']} for r in cpl]
write('CPL-JLCPCB-all.csv',['Designator','Mid X','Mid Y','Layer','Rotation'],coords)
sm=[];manual=[]
for r in rows:
 for selected,out in [(False,sm),(True,manual)]:
  rr=[q.strip() for q in r['Designator'].split(',') if (q.strip()=='J5')==selected]
  if rr:out.append({**r,'Designator':','.join(rr)})
write('BOM-JLCPCB-SMT.csv',['Comment','Designator','Footprint','LCSC Part #'],sm)
write('BOM-manual.csv',['Comment','Designator','Footprint','LCSC Part #'],manual)
write('CPL-JLCPCB-SMT.csv',['Designator','Mid X','Mid Y','Layer','Rotation'],[r for r in coords if r['Designator']!='J5'])
baseline=json.loads((v/'identity-and-backup.json').read_text(encoding='utf-8'))
src=Path(baseline['current_project_path']).parent
unchanged={name:hashlib.sha256((src/name).read_bytes()).hexdigest()==h for name,h in baseline['original_file_sha256'].items()}
assert all(unchanged.values())
assert hashlib.sha256((R.parent/'AirMonitor_V1_first_article_review.zip').read_bytes()).hexdigest()==baseline['sealed_delivery_zip_sha256']
e={'fresh_exports':True,'same_schematic_pcb_pin_maps':True,'pins_compared':len(a),'bom_cpl_components':42,'drill_files':drills,'gerber_copper_regions':fills,'source_files_unchanged_during_diagnosis':unchanged,'sealed_delivery_zip_unchanged':True,'smt_assembly_release':False}
(v/'candidate-integrity.json').write_text(json.dumps(e,ensure_ascii=False,indent=2),encoding='utf-8');print(json.dumps(e,ensure_ascii=False,indent=2))
