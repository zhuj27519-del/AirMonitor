const tab=await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
const b64=b=>new Promise(r=>{const f=new FileReader();f.onload=()=>r(String(f.result).split(',')[1]);f.readAsDataURL(b);});
const previews={};
for(const [name,layers] of [['front',[1,3,11,12]],['back-layer-top-view',[2,4,11,12]]]){
 await eda.pcb_Layer.setLayerVisible(layers,true);
 await eda.pcb_Document.zoomToBoardOutline();await new Promise(r=>setTimeout(r,700));
 const b=await eda.dmt_EditorControl.getCurrentRenderedAreaImage(tab);previews[name]=b?await b64(b):null;
}
await eda.pcb_Layer.setLayerVisible([1,2,3,4,11,12,13],true);
await eda.pcb_Document.save();return previews;
