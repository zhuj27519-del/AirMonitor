await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
const before={lines:(await eda.pcb_PrimitiveLine.getAll()).length,vias:(await eda.pcb_PrimitiveVia.getAll()).length};
const lines=(await eda.pcb_PrimitiveLine.getAll()).filter(l=>[1,2].includes(l.getState_Layer()));
if(lines.length)await eda.pcb_PrimitiveLine.delete(lines);
const vias=await eda.pcb_PrimitiveVia.getAll();if(vias.length)await eda.pcb_PrimitiveVia.delete(vias);
for(const c of await eda.pcb_PrimitiveComponent.getAll()){
 const ref=c.getState_Designator();
 if(ref==='C10')await eda.pcb_PrimitiveComponent.modify(c.getState_PrimitiveId(),{x:75/.0254,y:20/.0254});
 if(ref==='C9')await eda.pcb_PrimitiveComponent.modify(c.getState_PrimitiveId(),{x:77/.0254,y:58/.0254});
 if(ref==='J1')await eda.pcb_PrimitiveComponent.modify(c.getState_PrimitiveId(),{x:3/.0254,y:40/.0254,rotation:270});
}
await eda.pcb_Document.save();
return {before,lines:(await eda.pcb_PrimitiveLine.getAll()).length,vias:(await eda.pcb_PrimitiveVia.getAll()).length};
