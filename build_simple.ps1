# ShuTongWen IME 编译脚本 - 简化版

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "开始编译 ShuTongWen IME" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 设置环境变量
$vcDir = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207"
$winSdkDir = "C:\Program Files (x86)\Windows Kits\10"
$winSdkVer = "10.0.19041.0"

$env:Path = "$vcDir\bin\Hostx64\x64;" + $env:Path
$env:Include = "$vcDir\include;$vcDir\ATLMFC\include;$winSdkDir\include\$winSdkVer\ucrt;$winSdkDir\include\$winSdkVer\shared;$winSdkDir\include\$winSdkVer\um;$winSdkDir\include\$winSdkVer\winrt"
$env:Lib = "$vcDir\lib\x64;$winSdkDir\lib\$winSdkVer\ucrt\x64;$winSdkDir\lib\$winSdkVer\um\x64;$vcDir\ATLMFC\lib\x64"

# 创建输出目录
$buildDir = Join-Path $PSScriptRoot "build\bin"
$objDir = Join-Path $PSScriptRoot "build\obj"

if (!(Test-Path $buildDir)) { New-Item -ItemType Directory -Path $buildDir | Out-Null }
if (!(Test-Path $objDir)) { New-Item -ItemType Directory -Path $objDir | Out-Null }

Write-Host "输出目录：$buildDir" -ForegroundColor Green
Write-Host ""

# 编译单个文件函数
function Compile-File {
    param(
        [string]$SourceFile,
        [string]$OutputObj
    )
    
    $flags = @(
        "/c",
        "/std:c++20",
        "/O2",
        "/EHsc",
        "/MD",
        "/W3",
        "/I" + (Join-Path $PSScriptRoot "include"),
        "/I" + (Join-Path $PSScriptRoot "third_party\include"),
        "/Fo$OutputObj",
        $SourceFile
    )
    
    Write-Host "  编译：$(Split-Path -Leaf $SourceFile)" -ForegroundColor Gray
    $result = & cl $flags 2>&1
    if ($LASTEXITCODE -ne 0) {
        $result | Select-String "error" | ForEach-Object { Write-Host $_ -ForegroundColor Red }
    }
    return ($LASTEXITCODE -eq 0)
}

# 编译所有源文件
$allSources = Get-ChildItem -Path (Join-Path $PSScriptRoot "src") -Recurse -Filter "*.cpp"
$successCount = 0
$failCount = 0

Write-Host "[1/2] 编译所有源文件..." -ForegroundColor Yellow
foreach ($file in $allSources) {
    $objFile = Join-Path $objDir ($file.BaseName + ".obj")
    if (Compile-File -SourceFile $file.FullName -OutputObj $objFile) {
        $successCount++
    } else {
        $failCount++
    }
}

Write-Host "  成功：$successCount, 失败：$failCount" -ForegroundColor $(if ($failCount -eq 0) { "Green" } else { "Red" })
Write-Host ""

# 链接 DLL
Write-Host "[2/2] 链接 DLL..." -ForegroundColor Yellow
$outputDll = Join-Path $buildDir "ShuTongWenIME.dll"
$objFiles = Get-ChildItem -Path $objDir -Filter "*.obj"

if ($objFiles.Count -gt 0) {
    $linkFlags = @(
        "/DLL",
        "/OUT:$outputDll",
        "/INCREMENTAL:NO",
        "/SUBSYSTEM:WINDOWS",
        $objFiles.FullName,
        "kernel32.lib", "user32.lib", "gdi32.lib", "winspool.lib", "comdlg32.lib",
        "advapi32.lib", "shell32.lib", "ole32.lib", "oleaut32.lib", "uuid.lib",
        "odbc32.lib", "odbccp32.lib", "imm32.lib", "msctf.lib", "oleacc.lib"
    )
    
    & link $linkFlags 2>&1 | Select-String "error", "warning" | ForEach-Object {
        if ($_ -match "error") {
            Write-Host $_ -ForegroundColor Red
        } else {
            Write-Host $_ -ForegroundColor Yellow
        }
    }
} else {
    Write-Host "  错误：没有找到目标文件" -ForegroundColor Red
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
if (Test-Path $outputDll) {
    Write-Host "✓ 编译成功！" -ForegroundColor Green
    Write-Host "输出文件：$outputDll" -ForegroundColor Green
    Write-Host "文件大小：$((Get-Item $outputDll).Length / 1KB) KB" -ForegroundColor Green
} else {
    Write-Host "✗ 编译失败！" -ForegroundColor Red
}
Write-Host "========================================" -ForegroundColor Cyan
