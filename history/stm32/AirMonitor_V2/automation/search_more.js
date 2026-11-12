const out={};
for(const q of ['USB4105-GF-A','CL10A475KO8NNNC','CL10A105KB8NNNC','1206L050YR','SMF5.0A','TS-1088','B-2200S04P','B-2200S06P','B-2200S08P'])out[q]=(await eda.lib_Device.search(q)).slice(0,5);
return out;
