# 编译指南

## 前提条件

### 必须安装的工具

1. **Visual Studio 2022** (已安装)
   - 需要安装 "使用 C++ 的桌面开发" 工作负载
   - 需要安装 "Windows 10/11 SDK"

2. **CMake 3.24+** (需要安装)
   - 下载地址：https://cmake.org/download/
   - 安装时勾选 "Add CMake to the system PATH"

3. **WiX Toolset** (需要安装，用于生成安装包)
   - 下载地址：https://wixtoolset.org/docs/getting-started/
   - 或使用 Visual Studio 扩展：WiX Toolset Visual Studio 2022 Extensions

### 依赖库

项目使用 vcpkg 或 NuGet 管理依赖：

```powershell
# 使用 vcpkg 安装依赖
vcpkg install spdlog:x64-windows nlohmann-json:x64-windows gtest:x64-windows

# 或使用 NuGet (项目已配置)
```

## 编译步骤

### 方法一：使用 CMake（推荐）

```powershell
cd D:\DEV\shutongwen
mkdir build
cd build

# 配置项目
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build . --config Release -- /m

# 生成安装包
cmake --build . --config Release --target PACKAGE
```

### 方法二：使用 Visual Studio IDE

1. 打开 Visual Studio 2022
2. 选择 "打开本地文件夹"
3. 选择 `D:\DEV\shutongwen`
4. 等待 CMake 配置完成
5. 选择配置为 `Release`
6. 按 `Ctrl+Shift+B` 或选择 生成 -> 全部生成
7. 生成安装包：右键项目 -> 生成

### 方法三：使用 MSBuild（需要先生成.sln）

```powershell
cd D:\DEV\shutongwen\build

# 首先需要 CMake 生成解决方案文件
cmake .. -G "Visual Studio 17 2022" -A x64

# 然后使用 MSBuild 编译
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" ShuTongWen.sln /p:Configuration=Release /p:Platform=x64 /m /t:Rebuild
```

## 输出位置

编译成功后，文件位于：

```
build/bin/Release/          # 可执行文件
build/lib/Release/          # 库文件
build/package/              # 安装包 (.msi)
```

主要输出文件：
- `ShuTongWenIME.dll` - 输入法核心
- `ShuTongWenIME.exe` - UI 进程
- `ShuTongWenIME.msi` - Windows 安装包

## 测试安装

### 安装输入法

```powershell
# 管理员权限运行
msiexec /i build\package\ShuTongWenIME.msi

# 或双击 .msi 文件安装
```

### 注册输入法

```powershell
# 注册 TSF 输入法
regsvr32 build\bin\Release\ShuTongWenIME.dll

# 或使用提供的脚本
.\scripts\register-ime.ps1
```

### 验证安装

1. 打开 Windows 设置 -> 时间和语言 -> 语言
2. 查看是否显示 "ShuTongWen IME"
3. 切换到输入法，测试输入

## 常见问题

### Q: CMake 找不到 Visual Studio

**解决**: 使用 Developer Command Prompt for Visual Studio 2022

```powershell
# 在开始菜单搜索 "Developer Command Prompt for VS 2022"
# 然后运行 cmake 命令
```

### Q: 缺少依赖库

**解决**: 安装 vcpkg 并下载依赖

```powershell
git clone https://github.com/microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install spdlog nlohmann-json sqlite3 gtest
```

### Q: WiX 打包失败

**解决**: 确保 WiX Toolset 已正确安装

```powershell
# 检查 WiX 安装
where heat.exe
where candle.exe
where light.exe
```

### Q: ARM64 架构编译

**解决**: 使用 ARM64 配置

```powershell
cmake .. -G "Visual Studio 17 2022" -A ARM64 -DCMAKE_BUILD_TYPE=Release
```

## 调试版本

```powershell
# 编译 Debug 版本
cmake --build . --config Debug

# 输出位置
build/bin/Debug/
```

## 性能优化版本

```powershell
# 编译 Release 版本（带优化）
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE="/O2 /Ob2 /DNDEBUG"
```

## 下一步

编译成功后，请参考 `TESTING.md` 进行功能测试。
