const diff=await eda.sys_Tool.netlistComparison('452ccd4cf43911d0','0d571ee1a4bcf754');
await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
const f=await eda.pcb_ManufactureData.getNetlistFile('AirMonitor_V2');
return {diff,netlist:f?await f.text():null};
