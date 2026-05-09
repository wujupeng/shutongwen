#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <tchar.h>
#include <fstream>
#include <vector>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

#define APP_NAME _T("ShuTongWen IME")
#define APP_VERSION _T("1.0.0")
#define IME_TIP _T("{12345678-1234-1234-1234-123456789ABC}")

#include "embedded_dll.h"

bool ExtractResource(const unsigned char* data, size_t size, const TCHAR* path) {
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) return false;
    ofs.write(reinterpret_cast<const char*>(data), size);
    return true;
}

bool RunAsAdmin() {
    TCHAR path[MAX_PATH];
    if (GetModuleFileName(NULL, path, MAX_PATH)) {
        SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
        sei.lpVerb = _T("runas");
        sei.lpFile = path;
        sei.nShow = SW_SHOWNORMAL;
        if (ShellExecuteEx(&sei)) {
            return true;
        }
    }
    return false;
}

bool CheckAdmin() {
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID adminSid;
    if (!AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminSid)) {
        return false;
    }
    BOOL isAdmin = FALSE;
    CheckTokenMembership(NULL, adminSid, &isAdmin);
    FreeSid(adminSid);
    return isAdmin == TRUE;
}

bool RegisterCOM(const TCHAR* dllPath) {
    HINSTANCE hLib = LoadLibrary(dllPath);
    if (!hLib) return false;
    FARPROC pDllRegisterServer = GetProcAddress(hLib, "DllRegisterServer");
    if (!pDllRegisterServer) {
        FreeLibrary(hLib);
        return false;
    }
    HRESULT hr = pDllRegisterServer();
    FreeLibrary(hLib);
    return SUCCEEDED(hr);
}

bool CreateRegistryEntries(const TCHAR* installPath) {
    TCHAR regPath[MAX_PATH];
    _stprintf_s(regPath, _T("SOFTWARE\\Microsoft\\CTF\\TIP\\%s"), IME_TIP);
    
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, regPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
        return false;
    }
    
    RegSetValueEx(hKey, _T("Description"), 0, REG_SZ, (BYTE*)APP_NAME, (_tcslen(APP_NAME) + 1) * sizeof(TCHAR));
    DWORD langId = 0x0804;
    RegSetValueEx(hKey, _T("Language"), 0, REG_DWORD, (BYTE*)&langId, sizeof(DWORD));
    RegSetValueEx(hKey, _T("Profile"), 0, REG_SZ, (BYTE*)IME_TIP, (_tcslen(IME_TIP) + 1) * sizeof(TCHAR));
    DWORD showStatus = 1;
    RegSetValueEx(hKey, _T("ShowStatus"), 0, REG_DWORD, (BYTE*)&showStatus, sizeof(DWORD));
    
    TCHAR iconPath[MAX_PATH];
    _stprintf_s(iconPath, _T("%s\\ShuTongWenIME.dll,0"), installPath);
    RegSetValueEx(hKey, _T("IconPath"), 0, REG_SZ, (BYTE*)iconPath, (_tcslen(iconPath) + 1) * sizeof(TCHAR));
    
    RegCloseKey(hKey);
    return true;
}

bool EnableIMEAutomatically() {
    TCHAR psCommand[MAX_PATH * 4];
    _stprintf_s(psCommand, _T("powershell -Command \"$lang = Get-WinUserLanguageList; if (-not ($lang | Where-Object { $_.LanguageTag -eq 'zh-CN' })) { $newLang = New-WinUserLanguageList 'zh-CN'; $newLang.Add('en-US'); Set-WinUserLanguageList $newLang -Force }; $lang = Get-WinUserLanguageList; $zhLang = $lang | Where-Object { $_.LanguageTag -eq 'zh-CN' }; if ($zhLang) { if (-not ($zhLang.InputMethodTips | Where-Object { $_ -eq '%s' })) { $zhLang.InputMethodTips.Add('%s'); Set-WinUserLanguageList $lang -Force } }; $currentInputMethod = Get-WinInputMethodOverride; if ($currentInputMethod -ne '%s') { Set-WinInputMethodOverride '%s' }\""), IME_TIP, IME_TIP, IME_TIP, IME_TIP);

    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
    sei.lpVerb = _T("runas");
    sei.lpFile = _T("powershell.exe");
    sei.lpParameters = psCommand;
    sei.nShow = SW_HIDE;
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    
    if (!ShellExecuteEx(&sei)) {
        return false;
    }
    
    WaitForSingleObject(sei.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(sei.hProcess, &exitCode);
    CloseHandle(sei.hProcess);
    
    return exitCode == 0;
}

void RestartCTFServices() {
    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
    sei.lpVerb = _T("runas");
    sei.lpFile = _T("cmd.exe");
    sei.lpParameters = _T("/c sc stop ctfmon && sc start ctfmon");
    sei.nShow = SW_HIDE;
    ShellExecuteEx(&sei);
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    if (!CheckAdmin()) {
        if (!RunAsAdmin()) {
            MessageBox(NULL, _T("Please run as Administrator!"), _T("ShuTongWen IME"), MB_ICONERROR | MB_OK);
        }
        return 0;
    }
    
    TCHAR installDir[MAX_PATH];
    ExpandEnvironmentStrings(_T("%ProgramFiles%\\ShuTongWen"), installDir, MAX_PATH);
    CreateDirectory(installDir, NULL);
    
    TCHAR dllPath[MAX_PATH];
    _stprintf_s(dllPath, _T("%s\\ShuTongWenIME.dll"), installDir);
    
    if (!ExtractResource(ShuTongWenIME_dll, ShuTongWenIME_dll_size, dllPath)) {
        MessageBox(NULL, _T("Failed to extract files!"), _T("ShuTongWen IME"), MB_ICONERROR | MB_OK);
        return 1;
    }
    
    if (!RegisterCOM(dllPath)) {
        MessageBox(NULL, _T("COM registration failed!"), _T("ShuTongWen IME"), MB_ICONERROR | MB_OK);
        return 1;
    }
    
    if (!CreateRegistryEntries(installDir)) {
        MessageBox(NULL, _T("Registry creation failed!"), _T("ShuTongWen IME"), MB_ICONERROR | MB_OK);
        return 1;
    }
    
    EnableIMEAutomatically();
    RestartCTFServices();
    
    MessageBox(NULL, _T("ShuTongWen IME installed successfully!\n\nThe IME has been automatically enabled.\n\nUse Win+Space to switch input methods."), _T("Installation Complete"), MB_ICONINFORMATION | MB_OK);
    
    return 0;
}
