"""Read-only validation of native exports; writes a compact evidence report."""
import csv
import io
import json
from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parents[1]

def native_table(path):
    raw = path.read_bytes()
    encoding = 'utf-16' if raw[:2] in (b'\xff\xfe', b'\xfe\xff') else 'utf-8-sig'
    return list(csv.DictReader(io.StringIO(raw.decode(encoding)), delimiter='\t'))

def main():
    bom = native_table(ROOT / 'manufacturing/BOM-native.csv')
    cpl = native_table(ROOT / 'manufacturing/CPL-native.csv')
    refs = {r.strip() for row in bom for r in row['Designator'].split(',')}
    placement_refs = [row['Designator'] for row in cpl]
    net = json.loads((ROOT / 'hardware/pcb.net').read_text(encoding='utf-8'))
    net_refs = {c['props']['Designator'] for c in net['components'].values()}
    archive = ROOT / 'manufacturing/AirMonitor_V1_Gerber_REVIEW.zip'
    with zipfile.ZipFile(archive) as z:
        bad_member = z.testzip()
        names = z.namelist()
        drills = [n for n in names if n.endswith('.DRL')]
        drills_match = all(z.read(n) == (ROOT / 'manufacturing' / Path(n).name).read_bytes() for n in drills)
    step = ROOT / 'hardware/AirMonitor_V1.step'
    head = tail = b''
    if step.exists():
        with step.open('rb') as f:
            head = f.read(256)
            f.seek(max(0, step.stat().st_size - 256))
            tail = f.read()
    required = [
        'hardware/EasyEDA/AirMonitor V2 - PROVISIONAL.eprj3',
        'hardware/EasyEDA/sch/Schematic1/P1.esch2',
        'hardware/EasyEDA/pcb/PCB1.epcb2',
        'hardware/schematic-preview.png', 'hardware/pcb-front.png',
        'hardware/pcb-back-layer-top-view.png', 'firmware/.project',
        'firmware/.cproject', 'firmware/build/AirMonitor_V2.elf',
        'firmware/build/AirMonitor_V2.hex', 'firmware/build/AirMonitor_V2.bin',
        'firmware/build/AirMonitor_V2.map', 'verification/report.md',
    ]
    missing = [p for p in required if not (ROOT / p).is_file() or (ROOT / p).stat().st_size == 0]
    checks = {
        'bom_quantity': sum(int(r['Quantity']) for r in bom),
        'bom_unique_designators': len(refs),
        'cpl_rows': len(cpl), 'cpl_unique_designators': len(set(placement_refs)),
        'bom_cpl_netlist_designators_match': refs == set(placement_refs) == net_refs,
        'gerber_archive_crc_error': bad_member,
        'gerber_member_count': len(names), 'drill_file_count': len(drills),
        'extracted_drills_match_archive': drills_match,
        'step_bytes': step.stat().st_size if step.exists() else 0,
        'step_export_status': 'available' if step.exists() else 'Timed out; optional STEP omitted; no old STEP reused',
        'step_iso10303_header_and_terminator': b'ISO-10303-21;' in head and b'END-ISO-10303-21;' in tail,
        'required_files_missing_or_empty': missing,
        'scope': 'Artifact integrity only; does not change NOT RELEASED engineering status',
    }
    checks['artifact_integrity_passed'] = (
        checks['bom_quantity'] == len(refs) == len(cpl) == len(set(placement_refs)) == 42
        and checks['bom_cpl_netlist_designators_match'] and bad_member is None
        and len(drills) == 3 and drills_match
        and (not step.exists() or checks['step_iso10303_header_and_terminator']) and not missing
    )
    (ROOT / 'verification/delivery-checks.json').write_text(json.dumps(checks, ensure_ascii=False, indent=2), encoding='utf-8')
    print(json.dumps(checks, ensure_ascii=False, indent=2))
    assert checks['artifact_integrity_passed'], 'Delivery integrity failed'

if __name__ == '__main__':
    main()
