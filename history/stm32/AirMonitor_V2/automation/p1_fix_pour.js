await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
const m=v=>v/.0254;
// Use explicit vertices to avoid the rectangle shorthand's Y-origin convention.
const p=eda.pcb_MathPolygon.createPolygon([m(-1.5),m(.5),'L',m(99.5),m(.5),m(99.5),m(79.5),m(-1.5),m(79.5),m(-1.5),m(.5)]);
const out=[];
for(const pour of await eda.pcb_PrimitivePour.getAll()){
 await eda.pcb_PrimitivePour.modify(pour.getState_PrimitiveId(),{complexPolygon:p});
 out.push({id:pour.getState_PrimitiveId(),layer:pour.getState_Layer()});
}
await eda.pcb_Document.zoomToBoardOutline();
await eda.pcb_Document.save();
return out;
