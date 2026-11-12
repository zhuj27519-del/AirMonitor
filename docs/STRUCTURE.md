# 项目结构

```text
src/AirMonitor/       ESP8266/HF035 主源码
hardware/hmi/        HF035 sGUI 与界面素材
hardware/sensors/    21VOC 参考图片
docs/                来源清单、重复清单、版本与检查报告
history/esp8266/     六份原始 Arduino 固件
history/oled-test/   SSD1306 独立测试程序
history/hardware/    KiCad V1.5 草稿、EasyEDA 工作副本
history/stm32/       KEY3、V1、V1 fix、V2 完整目录快照
tools/               仓库敏感信息检查脚本
local-only/          原始包、Git 历史、订单、未审查证据；不提交
```

`inventory.json` 是复制时的逐文件记录；去重后路径映射见 `deduplication.json`。仅对散放的重复界面图片做内容去重，sGUI 工程的 tempImages/backUpImages 引用应保持完整。历史 CAD/源码重复是完整快照所需的有意保留，Git 会按对象内容去重。
