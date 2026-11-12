const plan=__CLI__.args;
await eda.dmt_EditorControl.openDocument('5f3d3d8e1737d3ae');
await new Promise(r=>setTimeout(r,2000));
const done=[];
for(const c of await eda.sch_PrimitiveComponent.getAll()){
 const ref=c.getState_Designator(); if(!plan[ref])continue;
 const pins=await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(c.getState_PrimitiveId());
 for(const p of pins){
  const net=plan[ref][p.pinNumber];
  if(!net){p.noConnected=true;await p.done();continue;}
  const a=p.rotation*Math.PI/180;
  const x=Math.round((p.x+30*Math.cos(a))*1000)/1000;
  const y=Math.round((p.y+30*Math.sin(a))*1000)/1000;
  const w=await eda.sch_PrimitiveWire.create([p.x,p.y,x,y],net);
  if(!w)throw new Error('Wire failed '+ref+'.'+p.pinNumber);
  await eda.sch_PrimitiveComponent.createNetPort('BI',net,x,y,p.rotation===180?180:0,false);
  done.push({ref,pin:p.pinNumber,net,wire:w.getState_PrimitiveId()});
 }
}
await eda.sch_Document.save();
return done;
