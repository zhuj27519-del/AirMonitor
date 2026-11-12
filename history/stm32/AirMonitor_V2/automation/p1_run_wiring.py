from eda_cli import call, ROOT
import sys
data=call(['invoke','--session',sys.argv[1],'--ext-uuid','eda','--timeout','180000','--args',(ROOT/'hardware/pin_plan.json').read_text(),'--code',(ROOT/'automation/wire_schematic.js').read_text()], 'p1-schematic-wiring',240)
print('Wired pin count:',len(data['value']))
