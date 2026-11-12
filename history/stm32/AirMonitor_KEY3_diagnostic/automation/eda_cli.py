"""Invoke the installed official CLI and preserve its complete response."""
import argparse, json, pathlib, subprocess, sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
EXE = r'C:\Program Files\lceda-pro\lceda-pro.exe'
sys.stdout.reconfigure(encoding='utf-8', errors='replace')

def call(args, name, timeout=90):
    p = subprocess.run([EXE, *args], capture_output=True, timeout=timeout,
                       creationflags=subprocess.CREATE_NO_WINDOW)
    try:
        out = p.stdout.decode('utf-8')
    except UnicodeDecodeError:
        out = p.stdout.decode('gb18030', errors='replace')
    (ROOT / 'verification' / (name + '.txt')).write_text(out + p.stderr.decode('utf-8', errors='replace'), encoding='utf-8')
    try:
        data = json.loads(out)
    except json.JSONDecodeError:
        print(out)
        raise RuntimeError('CLI did not return a JSON envelope')
    if not data.get('ok'):
        print(json.dumps(data, ensure_ascii=False))
        raise RuntimeError('CLI operation failed')
    return data

if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('--name', required=True)
    p.add_argument('--code-file')
    p.add_argument('--summary', action='store_true')
    p.add_argument('args', nargs=argparse.REMAINDER)
    a = p.parse_args()
    args = a.args[1:] if a.args[:1] == ['--'] else a.args
    if a.code_file:
        args += ['--code', pathlib.Path(a.code_file).read_text(encoding='utf-8')]
    data=call(args, a.name, timeout=360)
    print(json.dumps({'ok':data['ok'],'evidence':str(ROOT/'verification'/(a.name+'.txt'))} if a.summary else data, ensure_ascii=False, indent=2))
