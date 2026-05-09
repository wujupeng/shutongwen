#pragma once

#include <windows.h>
#include <string>

namespace ShuTongWen
{
    namespace Win32Utils
    {
        std::wstring GetModulePath(HMODULE hModule = nullptr);
        std::wstring GetModuleDirectory(HMODULE hModule = nullptr);
        
        std::wstring GetAppDataPath();
        std::wstring GetLocalAppDataPath();
        std::wstring GetProgramFilesPath();

        bool FileExists(const std::wstring& path);
        bool DirectoryExists(const std::wstring& path);
        
        bool CreateDirectoryRecursive(const std::wstring& path);
        bool DeleteFileSafe(const std::wstring& path);
        bool DeleteDirectoryRecursive(const std::wstring& path);

        std::wstring GetLastErrorMessage(DWORD errorCode = 0);
        
        bool SetRegistryValue(HKEY hKey, const std::wstring& subKey, const std::wstring& valueName, const std::wstring& value);
        bool GetRegistryValue(HKEY hKey, const std::wstring& subKey, const std::wstring& valueName, std::wstring& value);
        bool DeleteRegistryValue(HKEY hKey, const std::wstring& subKey, const std::wstring& valueName);

        bool IsWindows11OrLater();
        bool IsARM64Architecture();
    }
}