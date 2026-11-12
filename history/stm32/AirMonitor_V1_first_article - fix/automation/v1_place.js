await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
const m=v=>v/.0254,comps=await eda.pcb_PrimitiveComponent.getAll();
if(comps.length!==42)throw Error('Expected 42 components');
await eda.pcb_PrimitiveLine.delete((await eda.pcb_PrimitiveLine.getAll()).filter(p=>[1,2].includes(p.getState_Layer())));
await eda.pcb_PrimitiveVia.delete(await eda.pcb_PrimitiveVia.getAll());
await eda.pcb_PrimitivePoured.delete(await eda.pcb_PrimitivePoured.getAll());
const pos={R8:[15.5,51,0],U4:[24,54,0],R11:[20,55,90],R12:[20,59,0],R13:[25,59,0],C14:[28,54,90]};
for(const c of comps){const p=pos[c.getState_Designator()];if(p)await eda.pcb_PrimitiveComponent.modify(c.getState_PrimitiveId(),{x:m(p[0]),y:m(p[1]),rotation:p[2]});}
for(const ref of ['U2','U3']){
 const c=comps.find(c=>c.getState_Designator()===ref);
 const p=(await eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId(c.getState_PrimitiveId())).find(p=>p.getState_PadNumber()===(ref==='U2'?'4':'21'));
 for(const [dx,dy] of (ref==='U2'?[[-2,-1],[-2,0],[-2,1]]:[[0,-2.7],[0,2.7]]))await eda.pcb_PrimitiveVia.create('GND',p.getState_X()+m(dx),p.getState_Y()+m(dy),m(.3),m(.6));
}
for(const s of await eda.pcb_PrimitiveString.getAll())if(s.getState_Text().includes('AIRMONITOR V2-P1'))await eda.pcb_PrimitiveString.modify(s.getState_PrimitiveId(),{text:'AIRMONITOR V1 - ENGINEERING SAMPLE'});
await eda.pcb_Document.save();return {components:comps.length};
