const out={};
for(const q of ['S4B-PH-SM4-TB','S6B-PH-SM4-TB','S8B-PH-SM4-TB','TS-1088-AR02026','SMF5.0A'])out[q]=(await eda.lib_Device.search(q)).slice(0,6);
return out;
