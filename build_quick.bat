@echo off
REM ========================================
REM ShuTongWen IME 快速编译脚本
REM ========================================

setlocal enabledelayedexpansion

REM 设置环境变量
set "VSINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
set "VCINSTALLDIR=%VSINSTALLDIR%\VC"
set "VCToolsInstallDir=%VSINSTALLDIR%\VC\Tools\MSVC\14.44.35207\"
set "VCToolsVersion=14.44.35207"
set "WindowsSdkDir=C:\Program Files (x86)\Windows Kits\10\"
set "WindowsSDKVersion=10.0.19041.0\"

REM 设置路径
set "PATH=%VCToolsInstallDir%bin\Hostx64\x64;%PATH%"
set "INCLUDE=%VCToolsInstallDir%include;%VCToolsInstallDir%ATLMFC\include;%WindowsSdkDir%include\%WindowsSDKVersion%ucrt;%WindowsSdkDir%include\%WindowsSDKVersion%shared;%WindowsSdkDir%include\%WindowsSDKVersion%winrt"
set "LIB=%VCToolsInstallDir%lib\x64;%WindowsSdkDir%lib\%WindowsSDKVersion%ucrt\x64;%WindowsSdkDir%lib\%WindowsSDKVersion%um\x64"

REM 创建输出目录
if not exist "build\bin" mkdir build\bin
if not exist "build\obj" mkdir build\obj

echo ========================================
echo 开始编译 ShuTongWen IME
echo ========================================
echo.

REM 编译核心库
echo [1/5] 编译 Core 模块...
cl /c /std:c++20 /O2 /I"include" /I"third_party\include" /EHsc /MD /Fo"build\obj\core_" src\core\*.cpp 2>&1 | findstr /C:"error" /C:"warning"

echo [2/5] 编译 Utils 模块...
cl /c /std:c++20 /O2 /I"include" /I"third_party\include" /EHsc /MD /Fo"build\obj\utils_" src\utils\*.cpp 2>&1 | findstr /C:"error" /C:"warning"

echo [3/5] 编译 Platform 模块...
cl /c /std:c++20 /O2 /I"include" /I"third_party\include" /EHsc /MD /Fo"build\obj\platform_" src\platform\*.cpp 2>&1 | findstr /C:"error" /C:"warning"

echo [4/5] 编译 IME 核心模块...
cl /c /std:c++20 /O2 /I"include" /I"third_party\include" /EHsc /MD /Fo"build\obj\ime_" ^
    src\ime\IMEFramework.cpp ^
    src\ime\IMEModule.cpp ^
    src\ime\InputProcessor.cpp ^
    src\ime\PinyinParser.cpp ^
    src\ime\DictionaryManager.cpp ^
    src\ime\InputSession.cpp ^
    src\ime\semantic\InputSemanticLayer.cpp ^
    src\ime\cache\InputCache.cpp ^
    src\ime\adaptive\AdaptivePipeline.cpp ^
    src\ime\perception\LatencyOptimizer.cpp ^
    src\ime\pipeline\InputPipeline.cpp ^
    src\ime\tracing\InputTrace.cpp ^
    src\ime\replay\InputReplay.cpp ^
    2>&1 | findstr /C:"error" /C:"warning"

echo [5/5] 链接 DLL...
link /DLL /OUT:"build\bin\ShuTongWenIME.dll" ^
    build\obj\core_*.obj ^
    build\obj\utils_*.obj ^
    build\obj\platform_*.obj ^
    build\obj\ime_*.obj ^
    kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib ^
    advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib ^
    odbc32.lib odbccp32.lib imm32.lib msctf.lib 2>&1 | findstr /C:"error" /C:"warning"

echo.
echo ========================================
if exist "build\bin\ShuTongWenIME.dll" (
    echo 编译成功！
    echo 输出文件：build\bin\ShuTongWenIME.dll
) else (
    echo 编译失败！
)
echo ========================================

endlocal
