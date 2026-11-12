import pathlib,json,base64,zipfile,hashlib
R=pathlib.Path(__file__).resolve().parents[1]
def result(name):
    d=json.loads((R/'verification'/f'{name}.txt').read_text(encoding='utf-8'));assert d['ok'];return d['value']
s=result('p1-erc-final')
(R/'hardware/schematic.net').write_text(s['netlist'],encoding='utf-8')
(R/'hardware/BOM-native.csv').write_text(s['bom'],encoding='utf-8')
(R/'manufacturing/BOM-native.csv').write_text(s['bom'],encoding='utf-8')
(R/'hardware/schematic-preview.png').write_bytes(base64.b64decode(s['png']['b64']))
m=result('p1-manufacturing-final')
for key,path in [('gerber','manufacturing/AirMonitor_V2_P1_DO_NOT_FABRICATE.zip'),('cpl','manufacturing/CPL-native.csv'),('netlist','hardware/pcb.net')]:
    assert m.get(key) and 'b64' in m[key],(key,m.get(key));(R/path).write_bytes(base64.b64decode(m[key]['b64']))
z=zipfile.ZipFile(R/'manufacturing/AirMonitor_V2_P1_DO_NOT_FABRICATE.zip')
for name in z.namelist():
    if name.endswith('.DRL'):(R/'manufacturing'/pathlib.Path(name).name).write_bytes(z.read(name))
for name,data in result('p1-previews-final').items():
    assert data,name;(R/'hardware'/f'pcb-{name}.png').write_bytes(base64.b64decode(data))
def pins(d):
    return {(c['props']['Designator'],str(p['number'])):p.get('net','') for c in d['components'].values() for p in c['pinInfoMap'].values()}
a=pins(json.loads(s['netlist']));b=pins(json.loads((R/'hardware/pcb.net').read_text(encoding='utf-8')))
plan=json.loads((R/'hardware/pin_plan.json').read_text())
diff=[{'pin':k,'schematic':v,'pcb':b.get(k)} for k,v in a.items() if b.get(k)!=v]
diff.extend({'extra':k,'pcb':v} for k,v in b.items() if k not in a)
wrong=[{'pin':[ref,num],'expected':net,'actual':a.get((ref,num))} for ref,p in plan.items() for num,net in p.items() if a.get((ref,num))!=net]
state=result('p1-state-final')
checks={'same_pin_maps':not diff,'pin_map_differences':diff,'pin_plan_matches':not wrong,'pin_plan_differences':wrong,'all_symbol_pins_compared':len(a),'intended_connections':sum(map(len,plan.values())),'component_count':len(state['components']),'trace_segments':len(state['lines']),'vias':len(state['vias']),'copper_layers':2,'gerber_files':z.namelist(),'drills':[n for n in z.namelist() if n.endswith('.DRL')],'copper_fill_regions':{n:z.read(n).count(b'G36') for n in ['Gerber_TopLayer.GTL','Gerber_BottomLayer.GBL']}}
(R/'verification/independent-checks.json').write_text(json.dumps(checks,indent=2,ensure_ascii=False),encoding='utf-8')
print(json.dumps({k:v for k,v in checks.items() if k not in ['gerber_files']},indent=2))
