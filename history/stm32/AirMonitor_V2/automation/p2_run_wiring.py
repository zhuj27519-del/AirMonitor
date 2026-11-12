from eda_cli import call,ROOT
import sys
d=call(['invoke','--session',sys.argv[1],'--ext-uuid','eda','--timeout','180000','--args',(ROOT/'hardware/pin_plan.json').read_text(encoding='utf-8'),'--code',(ROOT/'automation/wire_schematic.js').read_text(encoding='utf-8')],'p2-schematic-wiring',240)
print('Wired pin count:',len(d['value']))
