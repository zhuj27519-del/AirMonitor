await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
const m=v=>v/.0254;
const comps=await eda.pcb_PrimitiveComponent.getAll();
if(comps.length!==37)throw Error('Expected 37 components');
// Old routing belongs to the replaced power topology; regenerate against P1.
await eda.pcb_PrimitiveLine.delete((await eda.pcb_PrimitiveLine.getAll()).filter(p=>[1,2].includes(p.getState_Layer())));
await eda.pcb_PrimitiveVia.delete(await eda.pcb_PrimitiveVia.getAll());
await eda.pcb_PrimitivePoured.delete(await eda.pcb_PrimitivePoured.getAll());
const positions={J1:[2,40,270],U2:[27,40,0],U3:[17,45,0],F1:[11,40,0],D1:[10,35,90],R1:[8,47,90],R2:[10.5,49,90],C5:[32.5,37.7,90],C6:[32.5,42.3,90],R7:[18,51,0],R8:[15.5,51,0],R9:[13,51,90],R10:[12.5,46,90],C11:[22,48,90],C12:[12.5,43.5,90],C13:[25,47,90],J5:[25,63,0],C9:[80,65,0]};
for(const c of comps){
 const p=positions[c.getState_Designator()];
 if(p)await eda.pcb_PrimitiveComponent.modify(c.getState_PrimitiveId(),{x:m(p[0]),y:m(p[1]),rotation:p[2]});
}
const usb=comps.find(c=>c.getState_Designator()==='J1');
const upins=await eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId(usb.getState_PrimitiveId());
const row=upins.find(p=>p.getState_PadNumber()==='A5');
// GCT A6 drawing: pad-row centre to front face 6.32mm. Front face aligned to X=-2mm.
await eda.pcb_PrimitiveComponent.modify(usb.getState_PrimitiveId(),{x:m(2)+m(4.32)-row.getState_X()});
const usbPads=await eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId(usb.getState_PrimitiveId());
for(const p of usbPads){
 if(['A9','B9'].includes(p.getState_PadNumber())){p.setState_Pad(['RECT',m(.76),m(1.2),0]);await p.done();}
}
// M2.5 hardware through 3.2mm clearance holes; Waveshare hole-centre spacing 44x29mm.
const existing=(await eda.pcb_PrimitivePad.getAll()).map(p=>p.getState_PadNumber());
for(const [name,x,y] of [['H5',33,30.5],['H6',77,30.5],['H7',77,59.5],['H8',33,59.5]]){
 if(existing.includes(name))throw Error('LCD mounting holes already exist');
 await eda.pcb_PrimitivePad.create(12,name,m(x),m(y),0,['ELLIPSE',m(3.2),m(3.2)],'', ['ROUND',m(3.2)],0,0,0,false,0);
 // 5mm nylon standoff envelope on document layer.
 await eda.pcb_PrimitivePolyline.create('',13,eda.pcb_MathPolygon.createPolygon(['CIRCLE',m(x),m(y),m(2.5)]),4,false);
}
for(const ref of ['U2','U3']){
 const c=comps.find(c=>c.getState_Designator()===ref);
 const pins=await eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId(c.getState_PrimitiveId());
 const p=pins.find(p=>p.getState_PadNumber()===(ref==='U2'?'4':'21'));
 p.setState_HeatWelding({connectionMethod:'Direct-connected'});await p.done();
 // Adjacent vias avoid via-in-pad assembly requirements.
 const x=p.getState_X(),y=p.getState_Y();
 const offsets=ref==='U2'?[[-2, -1],[-2,0],[-2,1]]:[[0,-2.7],[0,2.7]];
 for(const [dx,dy] of offsets)await eda.pcb_PrimitiveVia.create('GND',x+m(dx),y+m(dy),m(.3),m(.6));
}
await eda.pcb_Document.save();
const out=[];
for(const c of await eda.pcb_PrimitiveComponent.getAll())out.push({c,pads:await eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId(c.getState_PrimitiveId())});
return {components:out,netRules:await eda.pcb_Drc.getNetRules(),netByNet:await eda.pcb_Drc.getNetByNetRules(),rules:await eda.pcb_Drc.getCurrentRuleConfiguration()};
