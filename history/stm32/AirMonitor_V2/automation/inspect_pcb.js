await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');await new Promise(r=>setTimeout(r,1000));
const errors=await eda.pcb_Drc.check(true,false,true);
await eda.pcb_Document.zoomToBoardOutline();
const b=await eda.dmt_EditorControl.getCurrentRenderedAreaImage();
const b64=b=>new Promise(r=>{const f=new FileReader();f.onload=()=>r(String(f.result).split(',')[1]);f.readAsDataURL(b);});
return {errors,preview:b?await b64(b):null,components:await eda.pcb_PrimitiveComponent.getAll()};
