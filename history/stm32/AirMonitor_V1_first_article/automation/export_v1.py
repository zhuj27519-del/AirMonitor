"""Sequential native export pipeline; do not run while routing or filling copper."""
import sys,subprocess
from eda_cli import call,ROOT
session=sys.argv[1]
for name,script in [('v1-erc-final','check_schematic.js'),('v1-state-final','pcb_state.js'),('v1-manufacturing-final','export_v1_manufacturing.js'),('v1-previews-final','export_previews.js')]:
 v=call(['invoke','--session',session,'--ext-uuid','eda','--timeout','180000','--code',(ROOT/'automation'/script).read_text(encoding='utf-8')],name,timeout=240)
 print(name,'saved',flush=True)
 if name=='v1-erc-final':assert v['value']['passed'] and not v['value']['errors']
if '--no-step' not in sys.argv:
 subprocess.run([sys.executable,str(ROOT/'automation/step_v1_chunked.py'),session],check=True)
subprocess.run([sys.executable,str(ROOT/'automation/package_v1_exports.py')],check=True)
