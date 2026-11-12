await eda.dmt_EditorControl.openDocument('5f3d3d8e1737d3ae');
const all=await eda.sch_PrimitiveComponent.getAll();
if(all.some(c=>c.getState_Designator()==='U3'))throw Error('P1 parts already exist');
// Net ports and wires will be regenerated against the reviewed pin plan.
await eda.sch_PrimitiveWire.delete(await eda.sch_PrimitiveWire.getAll());
await eda.sch_PrimitiveComponent.delete(all.filter(c=>c.getState_ComponentType()==='netport'||['J1','U2'].includes(c.getState_Designator())));
const spec=[
 ['J1','USB4125-GF-A-0190',1500,1300,0],
 ['U2','MCP1826S-3302E/DB',1500,850,0],
 ['U3','TPS25940LRVCR',2200,1900,0],
 ['R7','RT0603BRD0744K2L',1600,1850,0],
 ['R8','0603WAF1000T5E',1600,1720,0],
 ['R9','RT0603BRD0710KL',1600,1590,0],
 ['R10','0603WAF1503T5E',2700,1850,0],
 ['C11','CC0603KRX7R9BB104',1900,1600,90],
 ['C12','CC0603KRX7R9BB104',2700,1650,90],
 ['C13','CL10A475KO8NNNC',2700,2100,90]
];
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
await eda.sch_Document.save();
return out;
