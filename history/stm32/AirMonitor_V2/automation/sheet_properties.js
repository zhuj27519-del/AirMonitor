await eda.dmt_EditorControl.openDocument('5f3d3d8e1737d3ae');await new Promise(r=>setTimeout(r,1500));
const sheets=(await eda.sch_PrimitiveComponent.getAll()).filter(c=>c.getState_ComponentType()==='sheet');
const out=[];
for(const c of sheets){const props=c.getState_OtherProperty();out.push({id:c.getState_PrimitiveId(),props});
await eda.sch_PrimitiveComponent.modify(c.getState_PrimitiveId(),{otherProperty:{...props,Size:'A1',Width:'3311',Height:'2339','Page Size':'A1'}});}
await eda.sch_Document.save();return out;
