#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <tchar.h>
#include <string>
#include <iostream>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

#define APP_NAME _T("书同文输入法")
#define APP_VERSION _T("1.0.0")
#define APP_GUID _T("{12345678-1234-1234-1234-123456789ABC}")
#define INSTALL_DIR _T("%ProgramFiles%\\ShuTongWen")

bool IsSilentMode(int argc, TCHAR* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (_tcsicmp(argv[i], _T("/silent")) == 0 || 
            _tcsicmp(argv[i], _T("/s")) == 0 ||
            _tcsicmp(argv[i], _T("-silent")) == 0 ||
            _tcsicmp(argv[i], _T("-s")) == 0) {
            return true;
        }
    }
    return false;
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
    _stprintf_s(regPath, _T("SOFTWARE\\Microsoft\\CTF\\TIP\\%s"), APP_GUID);
    
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, regPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
        return false;
    }
    
    RegSetValueEx(hKey, _T("Description"), 0, REG_SZ, (BYTE*)APP_NAME, (_tcslen(APP_NAME) + 1) * sizeof(TCHAR));
    
    DWORD langId = 0x0804;
    RegSetValueEx(hKey, _T("Language"), 0, REG_DWORD, (BYTE*)&langId, sizeof(DWORD));
    
    RegSetValueEx(hKey, _T("Profile"), 0, REG_SZ, (BYTE*)APP_GUID, (_tcslen(APP_GUID) + 1) * sizeof(TCHAR));
    
    DWORD showStatus = 1;
    RegSetValueEx(hKey, _T("ShowStatus"), 0, REG_DWORD, (BYTE*)&showStatus, sizeof(DWORD));
    
    TCHAR iconPath[MAX_PATH];
    _stprintf_s(iconPath, _T("%s\\ShuTongWenIME.dll,0"), installPath);
    RegSetValueEx(hKey, _T("IconPath"), 0, REG_SZ, (BYTE*)iconPath, (_tcslen(iconPath) + 1) * sizeof(TCHAR));
    
    RegCloseKey(hKey);
    return true;
}

bool CopyFiles(const TCHAR* sourceDir, const TCHAR* destDir) {
    if (!CreateDirectory(destDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return false;
    }
    
    TCHAR sourcePath[MAX_PATH];
    _stprintf_s(sourcePath, _T("%s\\ShuTongWenIME.dll"), sourceDir);
    
    TCHAR destPath[MAX_PATH];
    _stprintf_s(destPath, _T("%s\\ShuTongWenIME.dll"), destDir);
    
    return CopyFile(sourcePath, destPath, FALSE);
}

void ShowMessage(const TCHAR* title, const TCHAR* message, UINT type) {
    MessageBox(NULL, message, title, type);
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    bool silent = IsSilentMode(__argc, __argv);
    
    if (!CheckAdmin()) {
        if (silent) {
            std::cerr << "Error: Must run as administrator" << std::endl;
            return 1;
        }
        if (!RunAsAdmin()) {
            ShowMessage(_T("Error"), _T("This installer must be run as Administrator!"), MB_ICONERROR | MB_OK);
        }
        return 0;
    }
    
    TCHAR installerPath[MAX_PATH];
    GetModuleFileName(NULL, installerPath, MAX_PATH);
    PathRemoveFileSpec(installerPath);
    
    TCHAR installDir[MAX_PATH];
    ExpandEnvironmentStrings(INSTALL_DIR, installDir, MAX_PATH);
    
    if (!CopyFiles(installerPath, installDir)) {
        if (!silent) {
            ShowMessage(_T("Error"), _T("Failed to copy files!"), MB_ICONERROR | MB_OK);
        }
        return 1;
    }
    
    TCHAR dllPath[MAX_PATH];
    _stprintf_s(dllPath, _T("%s\\ShuTongWenIME.dll"), installDir);
    
    if (!RegisterCOM(dllPath)) {
        if (!silent) {
            ShowMessage(_T("Error"), _T("Failed to register COM component!"), MB_ICONERROR | MB_OK);
        }
        return 1;
    }
    
    if (!CreateRegistryEntries(installDir)) {
        if (!silent) {
            ShowMessage(_T("Error"), _T("Failed to create registry entries!"), MB_ICONERROR | MB_OK);
        }
        return 1;
    }
    
    if (!silent) {
        TCHAR message[MAX_PATH];
        _stprintf_s(message, _T("%s v%s\n\nInstallation completed successfully!\n\nTo enable:\n1. Settings -> Time & Language -> Language & Region\n2. Add Chinese (China)\n3. Select ShuTongWen IME\n4. Use Win+Space to switch"), APP_NAME, APP_VERSION);
        ShowMessage(_T("Installation Complete"), message, MB_ICONINFORMATION | MB_OK);
    }
    
    return 0;
}
