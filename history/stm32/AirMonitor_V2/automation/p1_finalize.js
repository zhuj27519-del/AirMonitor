await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
const changes=[];
for(const s of await eda.pcb_PrimitiveString.getAll()){
 const before=s.getState_Text();
 let after=before.replaceAll('V2-P0','V2-P1').replaceAll('V2 P0','V2 P1');
 if(before.includes('MOUNTING HOLES AND STANDOFFS NOT RELEASED'))after='LCD: 44x29 HOLES / M2.5 NYLON SPACERS - FIT CHECK';
 if(before.includes('ENVELOPE ONLY'))after=after.replace('ENVELOPE ONLY','MODULE ENVELOPE');
 if(after!==before){await eda.pcb_PrimitiveString.modify(s.getState_PrimitiveId(),{text:after});changes.push({before,after});}
}
await eda.pcb_Document.save();
await eda.pcb_Document.zoomToBoardOutline();
return {changes,strings:await eda.pcb_PrimitiveString.getAll(),pours:await eda.pcb_PrimitivePour.getAll(),filled:await eda.pcb_PrimitivePoured.getAll(),layers:await eda.pcb_Layer.getAllLayers()};
