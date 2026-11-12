const out={};
for(const q of ['STM32G0B1CBT6','AP2112K-3.3TRG1','USB-4085','0603WAF5101T5E','CC0603KRX7R9BB104','0603WAF1002T5E','PMS7003M']) {
 out[q]=await eda.lib_Device.search(q);
}
return out;
