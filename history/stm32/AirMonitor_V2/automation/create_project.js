if (!eda.sys_Environment.isClient() || !eda.sys_Environment.isHalfOfflineMode()) throw new Error('Local semi-offline client required');
const uuid = await eda.dmt_Project.createProject('AirMonitor V2 - PROVISIONAL', 'airmonitor-v2', undefined, undefined, 'Unreleased engineering design. Sensor and display interfaces require verification.', undefined, EDMT_ProjectFileFormat.EPRJ3);
if (!uuid) throw new Error('Project creation failed');
await eda.dmt_Project.openProject(uuid);
await new Promise(r=>setTimeout(r,5000));
return {project:await eda.dmt_Project.getCurrentProjectInfo(),paths:await eda.sys_FileSystem.getProjectsPaths()};
