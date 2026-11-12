await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
const results=[];
for(const layer of [1,2]){
 const polygon=eda.pcb_MathPolygon.createPolygon(['R',.5/.0254,.5/.0254,99/.0254,79/.0254,0,0]);
 const pour=await eda.pcb_PrimitivePour.create('GND',layer,polygon,'solid',false,'GND_'+layer,5,10,false);
 let filled=false,error=null;
 try{filled=(await pour.rebuildCopperRegion())!==undefined;}catch(e){error=String(e);}
 results.push({layer,id:pour.getState_PrimitiveId(),filled,error});
}
await eda.pcb_Document.save();
return {results,stackups:await eda.pcb_Layer.getAllPhysicalStackingConfigurations()};
