await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
const m=v=>v/.0254;
for(const c of await eda.pcb_PrimitiveComponent.getAll()){
 const p={R7:[19,51],J3:[90,58],C9:[83,65]}[c.getState_Designator()];
 if(p)await eda.pcb_PrimitiveComponent.modify(c.getState_PrimitiveId(),{x:m(p[0]),y:m(p[1])});
}
const r=await eda.pcb_Drc.getCurrentRuleConfiguration();
const cfg=r.config;
const power=JSON.parse(JSON.stringify(cfg.Physics.Track.copperThickness1oz));
power.editName='Power_0p5';power.isSetDefault=false;
power.form.data['1']={minValue:.2,defaultValue:.5,maxValue:2.54};
cfg.Physics.Track.Power_0p5=power;
if(!await eda.pcb_Drc.overwriteCurrentRuleConfiguration(cfg))throw Error('Power rule failed');
const nets=await eda.pcb_Drc.getNetRules();
for(const n of nets)if(['VBUS','5V_FUSED','5V','3V3'].includes(n.name))n.Track='Power_0p5';
if(!await eda.pcb_Drc.overwriteNetRules(nets))throw Error('Power net assignment failed');
await eda.pcb_Document.save();
return {rules:await eda.pcb_Drc.getNetRules(),job:await eda.pcb_Document.startAutoRouting({layers:[1,2]})};
