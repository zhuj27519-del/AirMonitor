await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
await eda.pcb_Document.save();
const filled=await eda.pcb_PrimitivePoured.getAll();
if(filled.length!==2)throw Error('Expected actual copper fill on both layers');
const errors=await eda.pcb_Drc.check(true,false,true);
return {filled,errors};
