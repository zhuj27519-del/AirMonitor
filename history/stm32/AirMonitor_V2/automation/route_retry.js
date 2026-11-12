await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
await new Promise(r=>setTimeout(r,1500));
return await eda.pcb_Document.autoRouting({layers:[1,2]});
