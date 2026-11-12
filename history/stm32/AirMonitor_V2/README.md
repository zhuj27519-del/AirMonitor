# AirMonitor V2 — V2-P1 工程审查版

**P0 的 USB 间距错误和未填地铜已解决；P1 原生 ERC/DRC 均为 0 项问题。仍未完成全部制造验收，不可直接下单。**

本目录包含可编辑 EasyEDA Pro 工程、双层布线和实际地铜、P1 原生制造导出，以及已构建的 STM32CubeIDE 固件工程。

## 本次修改

- USB-C 换为 GCT USB4125-GF-A-0190，并按厂家图纸修正电源焊盘；保持 0.20mm 间距规则。
- F1 后接 TVS，再经 TPS25940L eFuse 输出 5V；3V3 改用 MCP1826S SOT-223，加入限流、缓启动及过压关断。
- 完成两面地铜、散热焊盘直连和相邻地过孔；已从重新打开的工程及导出 Gerber 验证。
- 按 Waveshare 18079 官方 44×29mm 孔中心距加入四个 LCD 安装孔，连同四个主板孔共八孔；物理叠层已核实为 1.6mm FR4、两面 35µm 铜。
- 按用户确认，颗粒物模块采用图片中的八针接口；主板六针 J2 使用信号对应转接线束。

## 检查结果

37 个 PCB 器件、433 段走线、38 个过孔。168 个符号引脚网名逐项一致，139 项预定连接与引脚计划一致。最终 ERC 和含铜面 DRC 均为 0。固件源码未因本次电源/封装修订改变：保留先前 CubeIDE 构建 0 错误/0 警告及 107 项 ARM 解析器测试通过的证据，尚未进行实物测试。

## 从这里开始

1. 阅读 [验证报告](verification/report.md) 与 [封装复核](documentation/footprint-review.md)。
2. EasyEDA Pro 打开 `hardware/EasyEDA/AirMonitor V2 - PROVISIONAL.eprj3`；必须保留旁边的 pcb/、sch/ 等文件，不能只复制索引。
3. 接线参照 [接口说明](documentation/interfaces.md) 和 [八针线束表](documentation/sensor-harness-options.md)。显示屏 PH 端针号与主板 J4 相反，按信号配线。
4. CubeIDE 导入 `firmware/`，参照 [固件说明](firmware/README.md)。已含 ELF/HEX/BIN/MAP。

## 尚未关闭

OVP 阈值全温/瞬态余量、LCD 实际最大电流和热测试、全部封装/钢网及双面 CPL 装配终审、21VOC 实物插接件、传感器八针配套线与真实 UART 帧、显示模块/外壳试装。eFuse 的典型阈值不是传感器端最大电压的保证，详见 [电源审查](documentation/power-and-layout-review.md)。

制造目录采用 `AirMonitor_V2_P1_DO_NOT_FABRICATE.zip`，明确未发布。BOM 为 UTF-8 Tab 分隔，CPL 为 UTF-16 Tab 分隔；不是已经验证可直接上传的下单模板。背面预览保持顶视坐标，非镜像装配图。STEP 包含库模型，不代替实际装配验收。

P1 结果以 `verification/p1-*-final.txt`、`independent-checks.json`、`p1-native-readback.json` 和本报告为准。历史日志保留失败尝试和 P0 状态，不代表当前结果。P0 归档和 Git 历史保留，P1 另行打包。

未购买元件、未下单、未上传生产文件。eCO₂ 为模块估算值，HCHO 为交叉敏感的模块报告值，不是独立参考仪器结果。
