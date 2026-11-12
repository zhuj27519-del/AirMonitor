// Read-only exports. No save, routing, placement, net or rule mutations.
const out={};
const file=async f=>{if(!f)throw Error('Export returned no file');return {name:f.name,b64:await new Promise(r=>{const q=new FileReader();q.onload=()=>r(String(q.result).split(',')[1]);q.readAsDataURL(f);})};};
await eda.dmt_EditorControl.openDocument('5f3d3d8e1737d3ae');
await new Promise(r=>setTimeout(r,3000));
out.schematic=await file(await eda.sch_ManufactureData.getNetlistFile('AirMonitor_KEY3_Candidate'));
out.bom=await file(await eda.sch_ManufactureData.getBomFile('AirMonitor_KEY3_Candidate','csv'));
await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
await new Promise(r=>setTimeout(r,3000));
out.pcb=await file(await eda.pcb_ManufactureData.getNetlistFile('AirMonitor_KEY3_Candidate'));
out.gerber=await file(await eda.pcb_ManufactureData.getGerberFile('AirMonitor_KEY3_Candidate'));
out.cpl=await file(await eda.pcb_ManufactureData.getPickAndPlaceFile('AirMonitor_KEY3_Candidate','csv','mm'));
return out;
