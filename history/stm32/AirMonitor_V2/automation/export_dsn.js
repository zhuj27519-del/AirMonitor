await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
await new Promise(r=>setTimeout(r,1200));
const f=await eda.pcb_ManufactureData.getDsnFile('AirMonitor_V2');
return {dsn:f?await f.text():null};
