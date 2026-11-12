from eda_cli import call,ROOT
import json
data=call(['invoke','--session','321a778b-0ed8-418d-98ca-3dc3856149fb','--ext-uuid','eda','--timeout','180000','--args',(ROOT/'hardware/pin_plan.json').read_text(),'--code',(ROOT/'automation/wire_schematic.js').read_text()], 'schematic-wiring',240)
print('Wired pin count:',len(data['value']))
