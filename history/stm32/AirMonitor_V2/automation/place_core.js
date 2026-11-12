await eda.dmt_EditorControl.openDocument('5f3d3d8e1737d3ae');
await new Promise(r=>setTimeout(r,3000));
const spec=[['U1','STM32G0B1CBT6',500,850,0],['U2','AP2112K-3.3TRG1',1500,850,0],['J1','USB4085-GF-A',1500,1300,0],['R1','0603WAF5101T5E',1800,1300,90],['R2','0603WAF5101T5E',2000,1300,90],['R3','0603WAF1002T5E',1000,850,90],['C1','CC0603KRX7R9BB104',400,250,90],['C2','CC0603KRX7R9BB104',650,250,90],['C3','CC0603KRX7R9BB104',900,250,90],['C4','CC0603KRX7R9BB104',1000,600,90]];
const out=[];
for(const [ref,name,x,y,rot] of spec){
 const found=(await eda.lib_Device.search(name)).find(v=>v.name===name);
 if(!found)throw new Error('Exact part not found '+name);
 const c=await eda.sch_PrimitiveComponent.create(found,x,y,undefined,rot,false,true,true);
 const id=c.getState_PrimitiveId();
 const props=(await eda.sch_PrimitiveComponent.get(id)).getState_OtherProperty();
 await eda.sch_PrimitiveComponent.modify(id,{designator:ref,otherProperty:props});
 out.push({ref,id,name,props,pins:await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(id)});
}
await eda.sch_PrimitiveText.create(200,1550,'AIRMONITOR V2 / PROVISIONAL ENGINEERING DESIGN / NOT FOR FABRICATION',0,'#AA0000','Arial',22,true);
await eda.sch_Document.save();
return out;
