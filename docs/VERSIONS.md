# 历史版本识别

|版本|识别依据|处理|
|AirMonitor_ESP8266|OLED、PMS D5、21VOC UART0|保留原样|
|AirMonitor_ESP8266 (1)|OLED，新增数据超时和页面刷新控制|不是重复文件，保留|
|V4_Integrated|HF035，早期集成；函数名 u16 与核心类型可能冲突|保留，非主版本|
|V4_Fixed|HF035，旧控件 ID 集合 0,2..9|保留，不能用于 0..8 页面|
|V4_9Text_Matched|HF035 控件 0..8、明确字段映射|主源码基线|
|ESP8266_V3|内容实际以 V4/HF035 开头|不按文件名降级，保留独立副本|
|sketch_sep15a|SSD1306 60秒测试|历史诊断程序|
|KiCad V1.5|DRAFT/UNSIGNED|禁止直接生产|
|STM32 V2、V1、V1 fix、KEY3|STM32G0B1 + ST7789；文件名与内部版本称呼不完全一致|保留整套快照和警示|

下载时间不是可靠开发顺序。V3 与 V4_9Text_Matched 的精确差异见 SHA-256 和源码；不推断无法证明的发布历史。

旧 V2 Git 工作树存在未提交修改和未跟踪文件：本次同时保存当前目录快照与已有 Git bundle，不能把旧 HEAD 当成全部恢复成果。原仓库保持不动。
