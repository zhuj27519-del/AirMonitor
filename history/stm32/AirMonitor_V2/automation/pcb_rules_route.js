await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');await new Promise(r=>setTimeout(r,1500));
await eda.pcb_Layer.setTheNumberOfCopperLayers(2);
const current=await eda.pcb_Drc.getCurrentRuleConfiguration();
const cfg=current.config;
const safe=cfg.Spacing['Safe Spacing'].copperThickness1oz;
for(const row of safe.tables['1'].content)for(let i=0;i<row.length;i++)row[i]=Math.max(row[i],0.2);
cfg.Physics.Track.copperThickness1oz.form.data['1']={minValue:0.2,defaultValue:0.25,maxValue:2.54};
const vias=cfg.Physics['Via Size'].viaSize.form;
vias.viaOuterdiameterMin=0.6;vias.viaOuterdiameterDefault=0.65;vias.viaInnerdiameterMin=0.3;vias.viaInnerdiameterDefault=0.3;
if(!await eda.pcb_Drc.overwriteCurrentRuleConfiguration(cfg))throw new Error('Rules update failed');
const pos={C1:[44.75,33.5,0],C2:[43.2,31.5,0],C3:[42.3,33.5,0],C7:[44.5,29.5,0]};
for(const c of await eda.pcb_PrimitiveComponent.getAll()){const p=pos[c.getState_Designator()];if(p)await eda.pcb_PrimitiveComponent.modify(c.getState_PrimitiveId(),{x:p[0]/.0254,y:p[1]/.0254,rotation:p[2]});}
await eda.pcb_Document.save();
return {rules:await eda.pcb_Drc.getCurrentRuleConfiguration(),netRules:await eda.pcb_Drc.getNetRules(),job:await eda.pcb_Document.startAutoRouting({layers:[1,2]})};
