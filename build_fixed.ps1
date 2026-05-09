# ShuTongWen IME Build Script - Fixed Version

Write-Host "================================================"
Write-Host "Building ShuTongWen IME (Fixed Build)"
Write-Host "================================================"
Write-Host ""

# Setup environment
$vcDir = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207"
$winSdkDir = "C:\Program Files (x86)\Windows Kits\10"
$winSdkVer = "10.0.26100.0"

$env:Path = "$vcDir\bin\Hostx64\x64;" + $env:Path
$env:Include = "$vcDir\include;$vcDir\ATLMFC\include;$winSdkDir\include\$winSdkVer\ucrt;$winSdkDir\include\$winSdkVer\shared;$winSdkDir\include\$winSdkVer\um;$winSdkDir\include\$winSdkVer\winrt"
$env:Lib = "$vcDir\lib\x64;$winSdkDir\lib\$winSdkVer\ucrt\x64;$winSdkDir\lib\$winSdkVer\um\x64;$vcDir\ATLMFC\lib\x64"

# Create output directories
$buildDir = Join-Path $PSScriptRoot "build\bin"
$objDir = Join-Path $PSScriptRoot "build\obj"

if (!(Test-Path $buildDir)) { New-Item -ItemType Directory -Path $buildDir | Out-Null }
if (!(Test-Path $objDir)) { New-Item -ItemType Directory -Path $objDir | Out-Null }

Write-Host "Output: $buildDir"
Write-Host ""

# Compile single file
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
        "/I""$($PSScriptRoot)\include""",
        "/Fo""$OutputObj"""
    )
    
    Write-Host "  Compiling: $(Split-Path -Leaf $SourceFile)"
    $result = & cl $flags "$SourceFile" 2>&1
    if ($LASTEXITCODE -ne 0) {
        $result | Select-String "error" | Select-Object -First 3 | ForEach-Object { Write-Host "    $_" -ForegroundColor Red }
    }
    return ($LASTEXITCODE -eq 0)
}

# Only compile the most basic files
$coreFiles = @(
    "src\ime\PinyinParser.cpp",
    "src\utils\string_utils.cpp",
    "src\utils\win32_utils.cpp"
) | ForEach-Object { Join-Path $PSScriptRoot $_ }

$successCount = 0
$failCount = 0

Write-Host "[Step 1/2] Compiling core modules..."
foreach ($file in $coreFiles) {
    if (Test-Path $file) {
        $objFile = Join-Path $objDir ((Split-Path -Leaf $file) -replace '\.cpp$', '.obj')
        if (Compile-File -SourceFile $file -OutputObj $objFile) {
            $successCount++
        } else {
            $failCount++
        }
    } else {
        Write-Host "  Skip (not found): $(Split-Path -Leaf $file)" -ForegroundColor Yellow
    }
}

Write-Host "  Success: $successCount, Failed: $failCount"
Write-Host ""

# Link DLL
Write-Host "[Step 2/2] Linking DLL..."
$outputDll = Join-Path $buildDir "ShuTongWenIME.dll"
$objFiles = Get-ChildItem -Path $objDir -Filter "*.obj"

if ($objFiles.Count -gt 0) {
    $objList = $objFiles.FullName -join " "
    $linkCommand = "link /DLL /OUT:`"$outputDll`" /INCREMENTAL:NO /SUBSYSTEM:WINDOWS $objList kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib imm32.lib msctf.lib oleacc.lib"
    
    Write-Host "  Linking $($objFiles.Count) object files..."
    Write-Host "  Command: $linkCommand"
    & cmd /c $linkCommand 2>&1 | ForEach-Object {
        if ($_ -match "error") {
            Write-Host "    $_" -ForegroundColor Red
        } elseif ($_ -match "warning") {
            Write-Host "    $_" -ForegroundColor Yellow
        }
    }
} else {
    Write-Host "  Error: No object files found" -ForegroundColor Red
}

Write-Host ""
Write-Host "================================================"
if (Test-Path $outputDll) {
    Write-Host "[SUCCESS] Build completed!" -ForegroundColor Green
    Write-Host "Output: $outputDll" -ForegroundColor Green
    Write-Host "Size: $([math]::Round((Get-Item $outputDll).Length / 1KB, 2)) KB" -ForegroundColor Green
} else {
    Write-Host "[FAILED] Build failed!" -ForegroundColor Red
}
Write-Host "================================================"
