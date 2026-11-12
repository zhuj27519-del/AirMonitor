# 自动化与复核

通过已安装 EasyEDA Pro 桌面客户端的官方 CLI/extension API 工作。参考安装目录 `docs/cli-for-ai.md`、`docs/easyeda-format.md` 及保存于 `verification/` 的 API 查询结果。脚本不是独立 EDA 实现，不支持脱离原生编辑器生成“近似工程”。

`eda_cli.py` 调用 `C:/Program Files/lceda-pro/lceda-pro.exe`，保存完整 JSON 响应并检查顶层 `ok`。可按本机安装路径修改 `EXE`。通过 CLI `open` 打开工程后，使用本次返回的会话 ID；历史会话 ID 已关闭，不可复用。

示例（PowerShell，先用官方 CLI 帮助确认本机版本的 open 参数）：

```powershell
python automation/eda_cli.py --name recheck-erc --summary --code-file automation/check_schematic.js -- invoke --session YOUR_SESSION_ID --ext-uuid eda --timeout 180000
python automation/eda_cli.py --name recheck-drc --summary --code-file automation/pcb_drc.js -- invoke --session YOUR_SESSION_ID --ext-uuid eda --timeout 180000
```

这些脚本中的文档 UUID 对应该交付工程。复制成全新工程或重新建立文档后必须先查文档列表并更新 UUID。`create_project.js`、`place_*.js`、`wire_schematic.js`、`pcb_import.js`、`add_planes.js`、`finalize_board.js` 等为历史构建步骤，**不是幂等脚本，不要在现有工程上逐个重跑**；可能新增重复对象。

| 脚本 | 用途 |
|---|---|
| `check_schematic.js` | 原生 ERC、原理图网表/BOM/预览 |
| `pcb_drc.js` | 原生 PCB DRC |
| `pcb_state.js` | PCB 对象与走线状态 |
| `compare_nets.js` | 官方独立网表比较；本版观察到异常，见报告 |
| `export_manufacturing.js` | 从当前 PCB 导出原生 Gerber、CPL、网表 |
| `export_previews.js` | 正/背层原生截图，背层保持顶视方向 |
| `step_chunked.py` | 分块读取原生 STEP，避免工具输出过大 |
| `package_exports.py` | 从指定最终日志提取文件并独立比对网表与引脚计划 |
| `verify_delivery.py` | 交付结构、BOM/CPL 器件集合、制造 ZIP 和 STEP 完整性检查 |

读取 CLI 输出时先解析 JSON 再选取摘要；不要把包含 PNG/Gerber Base64 的整份日志打印到对话。`ok=true` 仅表示调用成功，必须进一步检查 DRC/ERC 结果及导出文件实际内容。

复核流程：保存 → 等待编辑器完成更新 → 原生 ERC/DRC → 导出双方网表 → 独立比对 → 导出制造文件/预览 → 验证 → 保存/关闭 CLI 会话 → 更新哈希/Git。发生修改后必须重走相关流程；`package_exports.py` 使用固定的 `*-final` 日志名，重跑前确认它们均来自同一新版本。

固件构建与主机测试说明在 `../firmware/README.md`。脚本和工程保留 Windows 本机路径，移到其他机器需要修改编译器/Python 路径；没有声称跨机器零配置可复现。

P1 使用 `export_p1.py SESSION` 顺序导出，`package_exports.py` 已改读 `p1-*-final` 证据。`--no-step` 仅用于机械几何未变的丝印调整。`verify_native_p1.py` 校验保存后的 USB 电源铜盘、散热直连设置、八孔及 FR4 物理叠层。`power_corners.py` 给出静态解析边界，不是电路瞬态仿真或实测。

`p1_persist_footprints.py` 是一次性修复步骤，只能在保存并关闭编辑器后执行；根据保存的官方 TPcbPad/EPadConnect 格式修改工程内嵌资源。禁止将 API setter 返回成功当作跨重开持久化成功。每次此类修改后都必须重开读回、重新补铜、DRC 与制造导出。

封包排除编辑器自动备份、临时写入文件、Python 缓存和可重建的 .o。P0 ZIP/Git bundle 在 outputs 顶层保留为历史快照。
