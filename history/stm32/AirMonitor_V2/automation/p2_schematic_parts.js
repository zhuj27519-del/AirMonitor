await eda.dmt_EditorControl.openDocument('5f3d3d8e1737d3ae');
const all=await eda.sch_PrimitiveComponent.getAll();
if(all.some(c=>c.getState_Designator()==='U4'))throw Error('P2 already applied');
await eda.sch_PrimitiveWire.delete(await eda.sch_PrimitiveWire.getAll());
await eda.sch_PrimitiveComponent.delete(all.filter(c=>c.getState_ComponentType()==='netport'||c.getState_Designator()==='R8'));
const spec=[['R8','0603WAF1001T5E',1600,1720,0],['U4','TLV6700DDCR',1000,1900,0],['R11','0603WAF1002T5E',600,1900,0],['R12','RT0603BRD07124KL',600,1700,0],['R13','RT0603BRD0710KL',1000,1650,0],['C14','CC0603KRX7R9BB104',1000,2100,90]];
const out=[];
for(const [ref,name,x,y,rotation] of spec){
 const part=(await eda.lib_Device.search(name)).find(p=>p.name===name);
 if(!part)throw Error('Missing exact part '+name);
 const c=await eda.sch_PrimitiveComponent.create(part,x,y,undefined,rotation,false,true,true);
 const id=c.getState_PrimitiveId();
 const props=(await eda.sch_PrimitiveComponent.get(id)).getState_OtherProperty();
 await eda.sch_PrimitiveComponent.modify(id,{designator:ref,otherProperty:props});
 out.push({ref,name,supplierId:part.supplierId,pins:await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(id)});
}
await eda.sch_Document.save();return out;
