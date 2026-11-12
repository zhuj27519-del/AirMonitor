await eda.dmt_EditorControl.openDocument('5f3d3d8e1737d3ae');
await new Promise(r=>setTimeout(r,1500));
const out=[];
for(const c of (await eda.sch_PrimitiveComponent.getAll()).filter(c=>c.getState_ComponentType()==='sheet')){
 const props=c.getState_OtherProperty();
 await eda.sch_PrimitiveComponent.modify(c.getState_PrimitiveId(),{otherProperty:{...props,Version:'V2-P1'}});
 out.push(c.getState_PrimitiveId());
}
await eda.sch_Document.save();return out;
