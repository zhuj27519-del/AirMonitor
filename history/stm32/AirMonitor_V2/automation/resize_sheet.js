await eda.dmt_EditorControl.openDocument('5f3d3d8e1737d3ae');
await new Promise(r=>setTimeout(r,2000));
const info=await eda.dmt_Schematic.getSchematicPageInfo('5f3d3d8e1737d3ae');
const data=info.titleBlockData;
for(const [k,v] of Object.entries({'Size':'A1','Page Size':'A1','Width':3311,'Height':2339,'Version':'V2-P0'})){
 data[k]={...(data[k]||{}),showTitle:false,showValue:k==='Version',value:v};
}
const result=await eda.dmt_Schematic.modifySchematicPageTitleBlock(true,data);
await eda.sch_Document.save();
return {result,info:await eda.dmt_Schematic.getSchematicPageInfo('5f3d3d8e1737d3ae')};
