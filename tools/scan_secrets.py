"""Scan Git-indexed text without printing secret values. Exit 1 on findings."""
from pathlib import Path
import subprocess,re,json,sys
root=Path(__file__).resolve().parents[1]
cmd=['git','-c',f'safe.directory={root.as_posix()}','-C',str(root)]
paths=list(filter(None,subprocess.check_output(cmd+['ls-files','-z']).decode().split('\0')))
batch=subprocess.run(cmd+['cat-file','--batch'],input=('\n'.join(':'+p for p in paths)+'\n').encode(),capture_output=True,check=True).stdout
offset=0
rules={
 'private_key':r'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----',
 'github_token':r'\b(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,})\b',
 'aws_key':r'\bAKIA[A-Z0-9]{16}\b',
 'assigned_secret':r'(?i)(?:password|passwd|api[_-]?key|access[_-]?token|wifi[_-]?(?:ssid|pass)|ssid)\s*[=:]\s*["\'][^"\'\r\n]{3,}["\']',
 'authenticated_url':r'https?://[^\s/:]+:[^\s/@]+@',
}
findings=[];count=0;binary=0
for name in paths:
    end=batch.index(b'\n',offset);size=int(batch[offset:end].split()[-1]);offset=end+1
    data=batch[offset:offset+size];offset+=size+1
    if b'\x00' in data[:8192]: binary+=1;continue
    count+=1;text=data.decode('utf-8',errors='replace')
    if name=='tools/scan_secrets.py':continue
    for label,pattern in rules.items():
        for m in re.finditer(pattern,text):
            findings.append(dict(file=name,line=text.count('\n',0,m.start())+1,rule=label))
print(json.dumps(dict(text_files=count,binary_files_not_text_scanned=binary,findings=findings),ensure_ascii=False,indent=2))
sys.exit(bool(findings))
