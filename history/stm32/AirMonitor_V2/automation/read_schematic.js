await eda.dmt_EditorControl.openDocument('5f3d3d8e1737d3ae');
await eda.sch_Document.save();
const out=[];
for(const c of await eda.sch_PrimitiveComponent.getAll())if(c.getState_Designator())out.push({ref:c.getState_Designator(),id:c.getState_PrimitiveId(),pins:await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(c.getState_PrimitiveId())});
return out;
