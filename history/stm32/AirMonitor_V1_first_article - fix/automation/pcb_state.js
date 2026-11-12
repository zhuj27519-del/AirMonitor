const tab=await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
await new Promise(r=>setTimeout(r,1000));
const components=[];
for(const c of await eda.pcb_PrimitiveComponent.getAll())components.push({c,pads:await eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId(c.getState_PrimitiveId())});
return {tab,components,lines:await eda.pcb_PrimitiveLine.getAll(),vias:await eda.pcb_PrimitiveVia.getAll()};
