"""Convert native tab-delimited exports without altering coordinates or rotations."""
from pathlib import Path
import csv,io,json
r=Path(__file__).resolve().parents[1]
def read(p):
 b=p.read_bytes();return list(csv.DictReader(io.StringIO(b.decode('utf-16' if b[:2] in [b'\xff\xfe',b'\xfe\xff'] else 'utf-8-sig')),delimiter='\t'))
bom=read(r/'manufacturing/BOM-native.csv');cpl=read(r/'manufacturing/CPL-native.csv')
def write(name,fields,rows):
 with (r/'manufacturing'/name).open('w',encoding='utf-8-sig',newline='') as f:
  w=csv.DictWriter(f,fieldnames=fields);w.writeheader();w.writerows(rows)
full=[{'Comment':x['Manufacturer Part'],'Designator':x['Designator'],'Footprint':x['Footprint'],'LCSC Part #':x['Supplier Part']} for x in bom]
fields=['Comment','Designator','Footprint','LCSC Part #']
write('BOM-JLCPCB-all.csv',fields,full)
sm=[];manual=[]
for row in full:
 a=[x for x in row['Designator'].split(',') if x!='J5'];b=[x for x in row['Designator'].split(',') if x=='J5']
 if a:sm.append({**row,'Designator':','.join(a)})
 if b:manual.append({**row,'Designator':','.join(b)})
write('BOM-JLCPCB-SMT.csv',fields,sm);write('BOM-manual.csv',fields,manual)
coords=[{'Designator':x['Designator'],'Mid X':x['Mid X'].removesuffix('mm'),'Mid Y':x['Mid Y'].removesuffix('mm'),'Layer':{'T':'Top','B':'Bottom'}[x['Layer']],'Rotation':x['Rotation']} for x in cpl]
fields=['Designator','Mid X','Mid Y','Layer','Rotation']
write('CPL-JLCPCB-all.csv',fields,coords);write('CPL-JLCPCB-SMT.csv',fields,[x for x in coords if x['Designator']!='J5'])
assert len(coords)==42 and len({x['Designator'] for x in coords})==42
smrefs={q for x in sm for q in x['Designator'].split(',')};assert smrefs=={x['Designator'] for x in coords if x['Designator']!='J5'}
d={'all_components':42,'smt_components':len(smrefs),'manual_components':['J5'],'coordinate_unit':'mm','rotations_preserved_from_native':True,'assembly_portal_pin1_review_completed':False,'stock_and_assembly_eligibility_verified':False}
(r/'verification/v1-jlc-table-checks.json').write_text(json.dumps(d,indent=2),encoding='utf-8');print(d)
