# Compile self-extracting installer EXE
$vcDir = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207"
$winSdkDir = "C:\Program Files (x86)\Windows Kits\10"
$winSdkVer = "10.0.26100.0"

$env:Path = "$vcDir\bin\Hostx64\x64;" + $env:Path
$env:Include = "$vcDir\include;$winSdkDir\include\$winSdkVer\ucrt;$winSdkDir\include\$winSdkVer\shared;$winSdkDir\include\$winSdkVer\um"
$env:Lib = "$vcDir\lib\x64;$winSdkDir\lib\$winSdkVer\ucrt\x64;$winSdkDir\lib\$winSdkVer\um\x64"

Write-Host "Compiling self-extracting installer..."
& cl /c /O2 /W3 /MT /EHsc self_extract.cpp
& link /OUT:ShuTongWenIME_Offline.exe self_extract.obj shlwapi.lib shell32.lib kernel32.lib user32.lib advapi32.lib /SUBSYSTEM:WINDOWS

if (Test-Path "ShuTongWenIME_Offline.exe") {
    Write-Host "Self-extracting installer compiled successfully!"
    Get-ChildItem "ShuTongWenIME_Offline.exe"
} else {
    Write-Host "Failed to compile installer"
    exit 1
}
