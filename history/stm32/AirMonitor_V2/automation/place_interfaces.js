await eda.dmt_EditorControl.openDocument('5f3d3d8e1737d3ae');
const old=(await eda.sch_PrimitiveComponent.getAll()).find(c=>c.getState_Designator()==='J1');
await eda.sch_PrimitiveComponent.delete(old.getState_PrimitiveId());
const spec=[
 ['J1','USB4105-GF-A',1500,1300,0],
 ['J2','S6B-PH-SM4-TB(LF)(SN)',2500,1350,0],
 ['J3','S4B-PH-SM4-TB(LF)(SN)',2500,1050,0],
 ['J4','S8B-PH-SM4-TB(LF)(SN)',2500,650,0],
 ['J5','B-2200S06P-A120',500,1300,0],
 ['F1','1206L050YR',1800,1050,0],
 ['D1','SMF5.0A-T13',2000,1050,90],
 ['C5','CL10A475KO8NNNC',1350,600,90],
 ['C6','CL10A475KO8NNNC',1600,600,90],
 ['C7','CL10A105KB8NNNC',1150,250,90],
 ['C8','CL10A475KO8NNNC',2800,650,90],
 ['C9','CC0603KRX7R9BB104',2800,1050,90],
 ['C10','CL10A475KO8NNNC',2800,1350,90],
 ['SW1','TS-1088-AR02026',1500,250,0],
 ['SW2','TS-1088-AR02026',1900,250,0],
 ['SW3','TS-1088-AR02026',2300,250,0],
 ['SW4','TS-1088-AR02026',1000,1100,0],
 ['R4','0603WAF1002T5E',1500,400,90],
 ['R5','0603WAF1002T5E',1900,400,90],
 ['R6','0603WAF1002T5E',2300,400,90]
];
const out=[];
for(const [ref,name,x,y,rot] of spec){
 const found=(await eda.lib_Device.search(name)).find(v=>v.name===name);
 if(!found)throw new Error('Exact part missing '+name);
 const c=await eda.sch_PrimitiveComponent.create(found,x,y,undefined,rot,false,true,true);
 const id=c.getState_PrimitiveId(); const props=(await eda.sch_PrimitiveComponent.get(id)).getState_OtherProperty();
 await eda.sch_PrimitiveComponent.modify(id,{designator:ref,otherProperty:props});
 out.push({ref,id,name,supplierId:found.supplierId,pins:await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(id)});
}
await eda.sch_PrimitiveText.create(2200,1550,'J2: CUSTOM HOST HARNESS; VERIFY SENSOR-END CABLE',0,'#AA0000','Arial',14);
await eda.sch_Document.save();return out;
