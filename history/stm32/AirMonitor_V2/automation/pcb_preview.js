const tab=await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
await eda.pcb_Document.zoomToBoardOutline();await new Promise(r=>setTimeout(r,500));
const b=await eda.dmt_EditorControl.getCurrentRenderedAreaImage(tab);
return b?await new Promise(r=>{const f=new FileReader();f.onload=()=>r(String(f.result).split(',')[1]);f.readAsDataURL(b);}):null;
