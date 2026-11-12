await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
await eda.pcb_Document.save();
const b64=b=>new Promise(r=>{const f=new FileReader();f.onload=()=>r(String(f.result).split(',')[1]);f.readAsDataURL(b);});
const out={};
for(const [key,fn] of [
 ['gerber',()=>eda.pcb_ManufactureData.getGerberFile('AirMonitor_V1_Gerber_REVIEW')],
 ['cpl',()=>eda.pcb_ManufactureData.getPickAndPlaceFile('AirMonitor_V1','csv','mm')],
 ['netlist',()=>eda.pcb_ManufactureData.getNetlistFile('AirMonitor_V1')]]){
 try{const f=await fn();out[key]=f?{name:f.name,b64:await b64(f)}:null;}catch(e){out[key]={error:String(e)};}
}
return out;
