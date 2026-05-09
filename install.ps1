# ShuTongWen IME 安装脚本
# 用于注册输入法到系统

Write-Host "================================================"
Write-Host "ShuTongWen IME 安装程序"
Write-Host "================================================"
Write-Host ""

# 检查是否以管理员身份运行
$currentPrincipal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
$isAdmin = $currentPrincipal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "错误：需要以管理员身份运行此脚本！" -ForegroundColor Red
    Write-Host "请右键点击 PowerShell 并选择 '以管理员身份运行'" -ForegroundColor Yellow
    pause
    exit 1
}

$scriptPath = $PSScriptRoot
$imeDllPath = Join-Path $scriptPath "build\bin\ShuTongWenIME.dll"

# 检查 DLL 是否存在
if (-not (Test-Path $imeDllPath)) {
    Write-Host "错误：找不到 IME DLL 文件: $imeDllPath" -ForegroundColor Red
    Write-Host "请先运行 build_full.ps1 编译项目" -ForegroundColor Yellow
    pause
    exit 1
}

Write-Host "[1/4] 正在注册 COM 组件..." -ForegroundColor Cyan
$regsvr32Path = "C:\Windows\System32\regsvr32.exe"
if (Test-Path $regsvr32Path) {
    & $regsvr32Path /s $imeDllPath
    if ($LASTEXITCODE -eq 0) {
        Write-Host "COM 组件注册成功" -ForegroundColor Green
    } else {
        Write-Host "COM 组件注册失败，错误代码: $LASTEXITCODE" -ForegroundColor Red
        pause
        exit 1
    }
}

Write-Host "[2/4] 正在创建注册表项..." -ForegroundColor Cyan

# 创建 IME 注册表项
$imeRegPath = "HKLM:\SOFTWARE\Microsoft\CTF\TIP\{12345678-1234-1234-1234-123456789ABC}"
if (-not (Test-Path $imeRegPath)) {
    New-Item -Path $imeRegPath -Force | Out-Null
}

# 设置 IME 属性
Set-ItemProperty -Path $imeRegPath -Name "Description" -Value "ShuTongWen IME"
Set-ItemProperty -Path $imeRegPath -Name "IconPath" -Value "$imeDllPath,0"
Set-ItemProperty -Path $imeRegPath -Name "Language" -Value 0x0804  # 中文(中国)
Set-ItemProperty -Path $imeRegPath -Name "Profile" -Value "{12345678-1234-1234-1234-123456789ABC}"
Set-ItemProperty -Path $imeRegPath -Name "ShowStatus" -Value 1
Set-ItemProperty -Path $imeRegPath -Name "SortOrder" -Value 0x80000000
Set-ItemProperty -Path $imeRegPath -Name "TipDescription" -Value "书同文输入法"

Write-Host "注册表项创建成功" -ForegroundColor Green

Write-Host "[3/4] 正在创建语言配置..." -ForegroundColor Cyan

# 创建语言配置
$langRegPath = "HKLM:\SOFTWARE\Microsoft\CTF\LanguageProfile\0x00000804\KeyboardLayout\00000804"
if (-not (Test-Path $langRegPath)) {
    New-Item -Path $langRegPath -Force | Out-Null
}

Set-ItemProperty -Path $langRegPath -Name "Layout File" -Value "kbdus.dll"
Set-ItemProperty -Path $langRegPath -Name "IME File" -Value $imeDllPath

Write-Host "语言配置创建成功" -ForegroundColor Green

Write-Host "[4/4] 正在更新输入法列表..." -ForegroundColor Cyan

# 更新输入法列表
$inputMethodPath = "HKCU:\Software\Microsoft\CTF\TIP\{12345678-1234-1234-1234-123456789ABC}"
if (-not (Test-Path $inputMethodPath)) {
    New-Item -Path $inputMethodPath -Force | Out-Null
}
Set-ItemProperty -Path $inputMethodPath -Name "Enabled" -Value 1

# 重启 CTF 服务
Write-Host "正在重启 CTF 服务..." -ForegroundColor Cyan
Stop-Service -Name "ctfmon" -Force -ErrorAction SilentlyContinue
Start-Service -Name "ctfmon" -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "================================================" -ForegroundColor Green
Write-Host "安装成功！" -ForegroundColor Green
Write-Host "================================================" -ForegroundColor Green
Write-Host ""
Write-Host "输入法已注册到系统，请按以下步骤启用：" -ForegroundColor Yellow
Write-Host "1. 打开 设置 -> 时间和语言 -> 语言和区域"
Write-Host "2. 点击 '添加语言'，搜索并添加 '中文(中国)'"
Write-Host "3. 点击 '选项'，在输入法列表中找到 '书同文输入法'"
Write-Host "4. 使用 Win+Space 切换输入法"
Write-Host ""
pause
