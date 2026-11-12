await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
const f=await eda.pcb_ManufactureData.get3DFile('AirMonitor_V2_P0','step',['Component Model'],'Outfit',true);
if(!f)return {available:false};
return {name:f.name,b64:await new Promise(r=>{const a=new FileReader();a.onload=()=>r(String(a.result).split(',')[1]);a.readAsDataURL(f);})};
