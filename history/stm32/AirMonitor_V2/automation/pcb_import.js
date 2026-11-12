await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
await new Promise(r=>setTimeout(r,5000));
const imported=await eda.pcb_Document.importChanges('452ccd4cf43911d0',true);
if(!imported)throw new Error('Schematic import failed');
await new Promise(r=>setTimeout(r,5000));
await eda.pcb_Document.save();
const out=[];for(const c of await eda.pcb_PrimitiveComponent.getAll())out.push({ref:c.getState_Designator(),id:c.getState_PrimitiveId(),pins:await eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId(c.getState_PrimitiveId())});
return {imported,components:out};
