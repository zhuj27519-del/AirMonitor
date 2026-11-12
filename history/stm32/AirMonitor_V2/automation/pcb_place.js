await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');await new Promise(r=>setTimeout(r,2000));
const m=x=>x/0.0254;
const poly=eda.pcb_MathPolygon.createPolygon([0,0,'L',m(100),0,m(100),m(80),0,m(80),0,0]);
await eda.pcb_PrimitivePolyline.create('',11,poly,4,true);
const pos={U1:[45,40,0],U2:[20,40,0],J1:[5,40,90],J2:[85,20,0],J3:[85,58,0],J4:[60,20,0,2],J5:[35,55,0],F1:[12,42,0],D1:[12,36,90],R1:[10,47,90],R2:[13,47,90],R3:[35,38,90],C1:[39,40,0],C2:[39,43,0],C3:[39,46,0],C4:[35,42,0],C5:[17,40,90],C6:[23,40,90],C7:[39,37,0],C8:[60,25,90,2],C9:[79,58,90],C10:[79,20,90],R4:[40,60,90,2],R5:[50,60,90,2],R6:[60,60,90,2],SW1:[40,65,0,2],SW2:[50,65,0,2],SW3:[60,65,0,2],SW4:[32,46,0]};
for(const c of await eda.pcb_PrimitiveComponent.getAll()){
 const p=pos[c.getState_Designator()];if(!p)throw new Error('No placement '+c.getState_Designator());
 await eda.pcb_PrimitiveComponent.modify(c.getState_PrimitiveId(),{x:m(p[0]),y:m(p[1]),rotation:p[2],layer:p[3]||1});
}
for(const [i,x,y] of [[1,4,4],[2,96,4],[3,96,76],[4,4,76]])await eda.pcb_PrimitivePad.create(12,'H'+i,m(x),m(y),0,['ELLIPSE',m(3.2),m(3.2)],'', ['ROUND',m(3.2)],0,0,0,false,0);
await eda.pcb_PrimitiveString.create(3,m(15),m(72),'AIRMONITOR V2-P0 - NOT FOR FABRICATION','default',45,6,3,0,false,0,false,true);
for(const [x,y,t] of [[72,26,'PMS HOST HARNESS'],[72,64,'21VOC 3V3'],[28,60,'SWD: 1=3V3 REF']])await eda.pcb_PrimitiveString.create(3,m(x),m(y),t,'default',40,6,3,0,false,0,false,true);
await eda.pcb_Document.save();
const components=[];for(const c of await eda.pcb_PrimitiveComponent.getAll())components.push({ref:c.getState_Designator(),id:c.getState_PrimitiveId(),pins:await eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId(c.getState_PrimitiveId())});
return {components,rules:await eda.pcb_Drc.getCurrentRuleConfiguration(),layers:await eda.pcb_Layer.getAllLayers(),stack:await eda.pcb_Layer.getCurrentPhysicalStackingConfiguration()};
