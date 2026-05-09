#include "utils/win32_utils.h"
#include "utils/string_utils.h"
#include <shlobj.h>

namespace ShuTongWen
{
    namespace Win32Utils
    {
        std::wstring GetModulePath(HMODULE hModule)
        {
            wchar_t path[MAX_PATH] = {0};
            GetModuleFileNameW(hModule, path, MAX_PATH);
            return path;
        }

        std::wstring GetModuleDirectory(HMODULE hModule)
        {
            std::wstring path = GetModulePath(hModule);
            size_t pos = path.find_last_of(L'\\');
            if (pos != std::wstring::npos)
                return path.substr(0, pos);
            return L"";
        }

        std::wstring GetAppDataPath()
        {
            wchar_t path[MAX_PATH] = {0};
            SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path);
            return path;
        }

        std::wstring GetLocalAppDataPath()
        {
            wchar_t path[MAX_PATH] = {0};
            SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path);
            return path;
        }

        std::wstring GetProgramFilesPath()
        {
            wchar_t path[MAX_PATH] = {0};
            SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILES, nullptr, 0, path);
            return path;
        }

        bool FileExists(const std::wstring& path)
        {
            DWORD attr = GetFileAttributesW(path.c_str());
            return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
        }

        bool DirectoryExists(const std::wstring& path)
        {
            DWORD attr = GetFileAttributesW(path.c_str());
            return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
        }

        bool CreateDirectoryRecursive(const std::wstring& path)
        {
            if (DirectoryExists(path))
                return true;

            std::wstring parent = path.substr(0, path.find_last_of(L'\\'));
            if (!parent.empty() && parent != path)
            {
                if (!CreateDirectoryRecursive(parent))
                    return false;
            }

            return CreateDirectoryW(path.c_str(), nullptr) != 0;
        }

        bool DeleteFileSafe(const std::wstring& path)
        {
            return DeleteFileW(path.c_str()) != 0;
        }

        bool DeleteDirectoryRecursive(const std::wstring& path)
        {
            if (!DirectoryExists(path))
                return true;

            std::wstring searchPath = path + L"\\*";
            WIN32_FIND_DATAW findData;
            HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);

            if (hFind == INVALID_HANDLE_VALUE)
                return false;

            do
            {
                std::wstring name = findData.cFileName;
                if (name == L"." || name == L"..")
                    continue;

                std::wstring fullPath = path + L"\\" + name;

                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (!DeleteDirectoryRecursive(fullPath))
                    {
                        FindClose(hFind);
                        return false;
                    }
                }
                else
                {
                    if (!DeleteFileSafe(fullPath))
                    {
                        FindClose(hFind);
                        return false;
                    }
                }
            } while (FindNextFileW(hFind, &findData) != 0);

            FindClose(hFind);
            return RemoveDirectoryW(path.c_str()) != 0;
        }

        std::wstring GetLastErrorMessage(DWORD errorCode)
        {
            if (errorCode == 0)
                errorCode = GetLastError();

            wchar_t* message = nullptr;
            FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                errorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<wchar_t*>(&message),
                0,
                nullptr
            );

            std::wstring result = message ? message : L"Unknown error";
            LocalFree(message);
            return result;
        }

        bool SetRegistryValue(HKEY hKey, const std::wstring& subKey, const std::wstring& valueName, const std::wstring& value)
        {
            HKEY hSubKey = nullptr;
            LONG result = RegCreateKeyExW(hKey, subKey.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hSubKey, nullptr);

            if (result != ERROR_SUCCESS)
                return false;

            result = RegSetValueExW(hSubKey, valueName.c_str(), 0, REG_SZ,
                reinterpret_cast<const BYTE*>(value.c_str()),
                static_cast<DWORD>((value.length() + 1) * sizeof(wchar_t)));

            RegCloseKey(hSubKey);
            return result == ERROR_SUCCESS;
        }

        bool GetRegistryValue(HKEY hKey, const std::wstring& subKey, const std::wstring& valueName, std::wstring& value)
        {
            HKEY hSubKey = nullptr;
            LONG result = RegOpenKeyExW(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey);

            if (result != ERROR_SUCCESS)
                return false;

            wchar_t buffer[MAX_PATH] = {0};
            DWORD bufferSize = MAX_PATH * sizeof(wchar_t);

            result = RegQueryValueExW(hSubKey, valueName.c_str(), nullptr, nullptr,
                reinterpret_cast<BYTE*>(buffer), &bufferSize);

            RegCloseKey(hSubKey);

            if (result == ERROR_SUCCESS)
                value = buffer;

            return result == ERROR_SUCCESS;
        }

        bool DeleteRegistryValue(HKEY hKey, const std::wstring& subKey, const std::wstring& valueName)
        {
            HKEY hSubKey = nullptr;
            LONG result = RegOpenKeyExW(hKey, subKey.c_str(), 0, KEY_WRITE, &hSubKey);

            if (result != ERROR_SUCCESS)
                return false;

            result = RegDeleteValueW(hSubKey, valueName.c_str());
            RegCloseKey(hSubKey);

            return result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND;
        }

        bool IsWindows11OrLater()
        {
            OSVERSIONINFOEXW osvi = {0};
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

            if (!GetVersionExW(reinterpret_cast<OSVERSIONINFOW*>(&osvi)))
                return false;

            if (osvi.dwMajorVersion >= 10)
            {
                if (osvi.dwBuildNumber >= 22000)
                    return true;
            }

            return false;
        }

        bool IsARM64Architecture()
        {
            SYSTEM_INFO si = {0};
            GetNativeSystemInfo(&si);
            return si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64;
        }
    }
}