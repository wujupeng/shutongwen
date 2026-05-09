# ShuTongWen IME WiX Installer Build Script

Write-Host "================================================"
Write-Host "ShuTongWen IME - WiX Installer Build"
Write-Host "================================================"
Write-Host ""

$scriptPath = $PSScriptRoot
$installerDir = Join-Path $scriptPath "installer"
$wxsFile = Join-Path $installerDir "ShuTongWenIME.wxs"
$objDir = Join-Path $installerDir "obj"
$outputDir = Join-Path $scriptPath "build\installer"

if (!(Test-Path $objDir)) { New-Item -ItemType Directory -Path $objDir | Out-Null }
if (!(Test-Path $outputDir)) { New-Item -ItemType Directory -Path $outputDir | Out-Null }

if (-not (Test-Path $wxsFile)) {
    Write-Host "Error: WiX source file not found: $wxsFile" -ForegroundColor Red
    exit 1
}

$imeDllPath = Join-Path $scriptPath "build\bin\ShuTongWenIME.dll"
if (-not (Test-Path $imeDllPath)) {
    Write-Host "Error: IME DLL not found: $imeDllPath" -ForegroundColor Red
    Write-Host "Please run build_full.ps1 first" -ForegroundColor Yellow
    exit 1
}

$candlePath = "C:\Program Files (x86)\WiX Toolset v3.11\bin\candle.exe"
$lightPath = "C:\Program Files (x86)\WiX Toolset v3.11\bin\light.exe"

if (Test-Path $candlePath) {
    Write-Host "[1/2] Compiling WiX source file..." -ForegroundColor Cyan
    
    & $candlePath -nologo -out "$objDir" -dSourceDir="$scriptPath" "$wxsFile"
    if ($LASTEXITCODE -eq 0) {
        Write-Host "WiX source compiled successfully" -ForegroundColor Green
    } else {
        Write-Host "WiX source compilation failed" -ForegroundColor Red
        exit 1
    }

    Write-Host "[2/2] Linking MSI package..." -ForegroundColor Cyan
    
    $wixobjFile = Join-Path $objDir "ShuTongWenIME.wixobj"
    $msiFile = Join-Path $outputDir "ShuTongWenIME.msi"
    
    & $lightPath -nologo -out "$msiFile" "$wixobjFile"
    if ($LASTEXITCODE -eq 0) {
        Write-Host "MSI package generated successfully" -ForegroundColor Green
        Write-Host "Output: $msiFile" -ForegroundColor Green
    } else {
        Write-Host "MSI package generation failed" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "WiX Toolset not installed, skipping MSI build" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "================================================"
Write-Host "Installer preparation completed!" -ForegroundColor Green
Write-Host "================================================"
Write-Host ""
Write-Host "Installation methods:" -ForegroundColor Cyan
Write-Host "1. PowerShell script installation:" -ForegroundColor Yellow
Write-Host "   Run as Administrator: .\install.ps1" -ForegroundColor Gray
Write-Host ""
Write-Host "2. MSI package installation:" -ForegroundColor Yellow
Write-Host "   Install WiX Toolset v3.11" -ForegroundColor Gray
Write-Host "   Run: .\build_installer.ps1" -ForegroundColor Gray
Write-Host "   Double-click: build\installer\ShuTongWenIME.msi" -ForegroundColor Gray
