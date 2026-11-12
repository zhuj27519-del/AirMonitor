"""Seal the review package. Run only after saving/closing EDA and final checks.

make-manifest: hash deliverables; archive: ZIP current files and check each hash.
The Git repository stays in the working folder; use git bundle for portability.
"""
import hashlib
from pathlib import Path
import sys
import zipfile

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / 'SHA256SUMS.txt'
ARCHIVE = ROOT.parent / 'AirMonitor_V2_P1_review_package.zip'

def files():
    return sorted(p for p in ROOT.rglob('*') if p.is_file()
                  and '.git' not in p.relative_to(ROOT).parts
                  and '__pycache__' not in p.relative_to(ROOT).parts
                  and not any(part.endswith('_backup') for part in p.relative_to(ROOT).parts)
                  and p.suffix not in ('.pyc', '.o', '.tmp'))

def digest(p):
    with p.open('rb') as f:
        return hashlib.file_digest(f, 'sha256').hexdigest()

if sys.argv[1] == 'make-manifest':
    entries = [f'{digest(p)}  {p.relative_to(ROOT).as_posix()}' for p in files() if p != MANIFEST]
    MANIFEST.write_text('\n'.join(entries) + '\n', encoding='utf-8')
    print(f'Manifest: {len(entries)} files')
elif sys.argv[1] == 'archive':
    entries = [line.split('  ', 1) for line in MANIFEST.read_text(encoding='utf-8').splitlines()]
    for expected, rel in entries:
        assert digest(ROOT / rel) == expected, f'Changed after seal: {rel}'
    with zipfile.ZipFile(ARCHIVE, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
        for p in files():
            z.write(p, 'AirMonitor_V2/' + p.relative_to(ROOT).as_posix())
    with zipfile.ZipFile(ARCHIVE) as z:
        assert z.testzip() is None
        for expected, rel in entries:
            assert hashlib.sha256(z.read('AirMonitor_V2/' + rel)).hexdigest() == expected, rel
        count = len(z.namelist())
    print(f'Archive verified: {count} files, {ARCHIVE.stat().st_size} bytes; SHA256 {digest(ARCHIVE)}')
else:
    raise SystemExit('Use make-manifest or archive')
