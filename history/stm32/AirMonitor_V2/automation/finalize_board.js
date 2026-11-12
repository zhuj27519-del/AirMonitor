await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
const m=v=>v/.0254;
const outlines=(await eda.pcb_PrimitivePolyline.getAll()).filter(p=>p.getState_Layer()===11);
if(outlines.length!==1)throw Error('Expected one board outline');
await eda.pcb_PrimitivePolyline.modify(outlines[0].getState_PrimitiveId(),{polygon:eda.pcb_MathPolygon.createPolygon([m(-2),0,'L',m(100),0,m(100),m(80),m(-2),m(80),m(-2),0])});
/* 102 x 80 mm: left edge now matches the USB manufacturer's mating-edge datum. */
for(const p of await eda.pcb_PrimitivePour.getAll())await eda.pcb_PrimitivePour.modify(p.getState_PrimitiveId(),{complexPolygon:eda.pcb_MathPolygon.createPolygon(['R',m(-1.5),m(.5),m(101),m(79),0,0])});
await eda.pcb_PrimitivePolyline.create('',13,eda.pcb_MathPolygon.createPolygon([m(30),m(27.5),'L',m(80),m(27.5),m(80),m(62.5),m(30),m(62.5),m(30),m(27.5)]),4,true);
await eda.pcb_PrimitiveString.create(13,m(32),m(45),'BACK: WAVESHARE 18079 50x35mm ENVELOPE ONLY','default',35,5,3,0,false,0,false,true);
await eda.pcb_PrimitiveString.create(13,m(32),m(41),'MOUNTING HOLES AND STANDOFFS NOT RELEASED','default',35,5,3,0,false,0,false,true);
await eda.pcb_Document.save();
return {outline:await eda.pcb_PrimitivePolyline.getAll(),layers:await eda.pcb_Layer.getAllLayers()};
