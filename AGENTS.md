# AirMonitor 协作约定

- 主工程是 `src/AirMonitor` 的 ESP8266/HF035 固件；STM32、OLED、KiCad 均为独立历史方案。
- 修改前阅读 README、docs/AUDIT.md 和 TODO.md；以源码及实际测量为依据，区分历史声明与本次验证。
- 不删除来源原件，不覆盖 history 快照。新工作从 src 或独立硬件工作目录开展。
- Arduino 主文件必须与目录同名。用 ESP8266 core 3.1.2、nodemcuv2 验证编译。不要向 Serial 添加调试文本，它被 HF035 占用。
- 协议、引脚或电平改动需有数据手册或实物帧证据，并运行对应测试。编译通过不能写成实机通过。
- 禁止提交 Wi-Fi 凭据、token、私钥、订单隐私、local-only、旧 Git 历史包及未经检查的压缩包。提交前执行 tools/scan_secrets.py 并人工检查报告。
- 不自动烧录、下单或改变硬件制造放行状态。保留第三方许可证。
- 用户已经授权本次整理、建私有仓库与首次提交推送，无需重复询问；GitHub 登录由用户完成。用户后续已明确授权将仓库公开，当前仓库为 Public；仍须禁止上传凭据和 local-only 内容。
