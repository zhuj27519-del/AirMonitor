await eda.dmt_EditorControl.openDocument('5f3d3d8e1737d3ae');await new Promise(r=>setTimeout(r,1500));
const result=await eda.dmt_Schematic.modifySchematicPageTitleBlock(false);
await eda.sch_Document.save();return {result,info:await eda.dmt_Schematic.getSchematicPageInfo('5f3d3d8e1737d3ae')};
