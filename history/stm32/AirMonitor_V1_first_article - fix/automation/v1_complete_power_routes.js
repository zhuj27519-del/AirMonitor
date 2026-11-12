await eda.dmt_EditorControl.openDocument('0d571ee1a4bcf754');
const m=v=>v/.0254;
await eda.pcb_PrimitivePoured.delete(await eda.pcb_PrimitivePoured.getAll());
const cfg=(await eda.pcb_Drc.getCurrentRuleConfiguration()).config;
const fine=JSON.parse(JSON.stringify(cfg.Physics.Track.copperThickness1oz));
fine.editName='QFN_control_0p15';fine.isSetDefault=false;fine.form.data['1']={minValue:.15,defaultValue:.15,maxValue:2.54};cfg.Physics.Track.QFN_control_0p15=fine;
await eda.pcb_Drc.overwriteCurrentRuleConfiguration(cfg);
const rules=await eda.pcb_Drc.getNetRules();for(const n of rules)if(['EF_EN','EF_ILIM','EF_DVDT','OVP_DIV'].includes(n.name))n.Track='QFN_control_0p15';
await eda.pcb_Drc.overwriteNetRules(rules);
const out=[];
async function line(net,pts,w=.2,layer=1){for(let i=1;i<pts.length;i++)out.push(await eda.pcb_PrimitiveLine.create(net,layer,m(pts[i-1][0]),m(pts[i-1][1]),m(pts[i][0]),m(pts[i][1]),m(w),false));}
// Short 0.20mm fanouts from TI 0.20mm lands into 0.50mm power trunks.
for(const x of [17.25,17.75,18.25]){await line('5V',[[x,43.6],[x,43.0]]);await line('5V_FUSED',[[x,46.4],[x,47.0]]);}
for(const y of [44.25,44.75])await line('5V',[[18.9,y],[19.5,y]]);
for(const y of [45.25,45.75])await line('5V_FUSED',[[18.9,y],[19.5,y]]);
await line('5V',[[17.25,43.0],[19.5,43.0],[19.5,44.75]],.2);
await line('5V_FUSED',[[17.25,47.0],[19.5,47.0],[19.5,45.25]],.2);
// Separate via positions avoid placing drills in fine-pitch component lands.
await line('5V',[[19.5,43],[20.3,43]],.5);
await line('5V_FUSED',[[19.5,47],[20.3,47]],.5);
for(const [net,x,y] of [['5V',20.3,43],['5V_FUSED',20.3,47],['GND',31.7,40],['GND',33.5,43]])await eda.pcb_PrimitiveVia.create(net,m(x),m(y),m(.3),m(.6));
await line('GND',[[29.87,40],[31.7,40]],.4);
await line('GND',[[32.5,43],[33.5,43]],.3);
await eda.pcb_Document.save();return {added:out.length,rules};
