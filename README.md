# ShuTongWen IME

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/wujupeng/shutongwen)
[![Version](https://img.shields.io/badge/version-1.0.0-blue)](https://github.com/wujupeng/shutongwen/releases)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

书同文输入法 - 书同文，车同轨。一个现代化的 Windows 11 中文输入法

## 特性

- ✅ **高性能** - 基于 C++20 开发，输入响应迅速
- ✅ **智能输入** - 支持拼音输入、中英混输、Emoji联想
- ✅ **轻量级** - 独立进程架构，稳定性高
- ✅ **跨应用兼容** - 完美支持 Chrome、VSCode、Office 等应用

## 系统要求

- Windows 11 (ARM64 / x64)
- Windows SDK 10.0.26100.0+

## 安装

### 方法一：EXE 安装程序（推荐）

1. 下载最新版本：[ShuTongWenIME_Setup.exe](https://github.com/wujupeng/shutongwen/releases)
2. 运行安装程序（需要管理员权限）
3. 启用输入法：
   - 设置 → 时间和语言 → 语言和区域
   - 添加语言 → 搜索 "中文(中国)"
   - 点击选项 → 选择 "舒通文输入法"

#### 静默安装

```cmd
ShuTongWenIME_Setup.exe /silent
```

### 方法二：手动安装

```powershell
# 以管理员身份运行
regsvr32 /s "ShuTongWenIME.dll"
```

## 技术栈

| 组件 | 技术 |
|------|------|
| 主语言 | C++20 |
| UI框架 | WinUI 3 |
| 数据库 | SQLite3 |
| 日志 | spdlog |
| JSON | nlohmann/json |
| 构建工具 | CMake + vcpkg |

## 架构设计

```
┌─────────────────────────────────────────────────────────────┐
│                    ShuTongWen IME                          │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌───────────────────┐  │
│  │  IME Core   │  │   UI Process│  │    AI Process     │  │
│  │ (TSF DLL)   │  │ (WinUI 3)   │  │ (ONNX Runtime)   │  │
│  └──────┬──────┘  └──────┬──────┘  └─────────┬─────────┘  │
│         │                │                    │            │
│         └────────────────┼────────────────────┘            │
│                          ▼                                │
│              ┌─────────────────────┐                       │
│              │    IPC Layer        │                       │
│              │  (Named Pipes)      │                       │
│              └─────────────────────┘                       │
└─────────────────────────────────────────────────────────────┘
```

## 构建说明

### 环境要求

- Visual Studio 2022 Build Tools
- CMake 3.29+
- vcpkg

### 编译步骤

```powershell
# 克隆仓库
git clone https://github.com/wujupeng/shutongwen.git
cd shutongwen

# 设置代理（如需）
$env:HTTP_PROXY="http://127.0.0.1:10808"

# 编译
.\build_full.ps1

# 生成安装包
.\build_installer.ps1
```

## 版本历史

### v1.0.0 (2024)
- 初始版本发布
- 支持基本拼音输入
- 支持 SQLite 词库
- 支持 WinUI 3 界面

## 许可证

MIT License - 详见 [LICENSE](LICENSE)

## 贡献

欢迎提交 Issue 和 Pull Request！

## 联系方式

- GitHub: [wujupeng](https://github.com/wujupeng)
- 项目地址: [https://github.com/wujupeng/shutongwen](https://github.com/wujupeng/shutongwen)
