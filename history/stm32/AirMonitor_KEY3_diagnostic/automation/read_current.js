const components=[];
for(const c of await eda.pcb_PrimitiveComponent.getAll()) {
 if(['U1','R6','SW3'].includes(c.getState_Designator())) components.push({c,pads:await eda.pcb_PrimitiveComponent.getAllPinsByPrimitiveId(c.getState_PrimitiveId())});
}
return {project:await eda.dmt_Project.getCurrentProjectInfo(),version:await eda.sys_Environment.getEditorCurrentVersion(),components,lines:await eda.pcb_PrimitiveLine.getAll(),vias:await eda.pcb_PrimitiveVia.getAll(),rules:await eda.pcb_Drc.getCurrentRuleConfiguration(),netRules:await eda.pcb_Drc.getNetRules(),fills:await eda.pcb_PrimitivePoured.getAll()};
