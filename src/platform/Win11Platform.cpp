#include "platform/Win11Platform.h"
#include "utils/logger.h"
#include <psapi.h>
#include <sddl.h>

namespace ShuTongWen
{
    Win11Platform::Win11Platform()
        : m_initialized(false),
          m_isWindows11(false),
          m_isARM64(false),
          m_isHighDpi(false)
    {}

    Win11Platform& Win11Platform::Instance()
    {
        static Win11Platform instance;
        return instance;
    }

    bool Win11Platform::Initialize()
    {
        if (m_initialized)
            return true;

        Logger::Info("Initializing Win11Platform...");

        OSVERSIONINFOEXW osvi = {0};
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

        if (GetVersionExW(reinterpret_cast<OSVERSIONINFOW*>(&osvi)))
        {
            m_isWindows11 = (osvi.dwMajorVersion >= 10 && osvi.dwBuildNumber >= 22000);
        }

        SYSTEM_INFO si = {0};
        GetNativeSystemInfo(&si);
        m_isARM64 = (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64);

        m_isHighDpi = SetProcessDpiAwareness() == S_OK;

        m_initialized = true;
        Logger::Info("Win11Platform initialized: Win11={}, ARM64={}, HighDPI={}", 
            m_isWindows11, m_isARM64, m_isHighDpi);
        
        return true;
    }

    void Win11Platform::Uninitialize()
    {
        m_initialized = false;
    }

    bool Win11Platform::IsWindows11OrLater() const
    {
        return m_isWindows11;
    }

    bool Win11Platform::IsARM64() const
    {
        return m_isARM64;
    }

    bool Win11Platform::IsHighDpiEnabled() const
    {
        return m_isHighDpi;
    }

    HRESULT Win11Platform::GetCurrentWindowInfo(WindowInfo& info)
    {
        ZeroMemory(&info, sizeof(info));

        info.hwnd = GetForegroundWindow();
        if (!info.hwnd)
            return E_FAIL;

        WCHAR className[MAX_PATH] = {0};
        GetClassNameW(info.hwnd, className, MAX_PATH);
        info.className = className;

        WCHAR windowTitle[MAX_PATH] = {0};
        GetWindowTextW(info.hwnd, windowTitle, MAX_PATH);
        info.windowTitle = windowTitle;

        GetWindowRect(info.hwnd, &info.rect);
        info.isForeground = true;

        DWORD pid = 0;
        GetWindowThreadProcessId(info.hwnd, &pid);

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess)
        {
            WCHAR processName[MAX_PATH] = {0};
            GetModuleFileNameExW(hProcess, nullptr, processName, MAX_PATH);
            
            size_t pos = std::wstring(processName).find_last_of(L'\\');
            if (pos != std::wstring::npos)
            {
                info.processName = std::wstring(processName).substr(pos + 1);
            }

            CloseHandle(hProcess);
        }

        info.isImmersive = IsUWPApp(info.hwnd);

        return S_OK;
    }

    HRESULT Win11Platform::GetAllMonitors(std::vector<MonitorInfo>& monitors)
    {
        monitors.clear();

        auto callback = [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) -> BOOL {
            std::vector<MonitorInfo>* pMonitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
            
            MonitorInfo info;
            info.hMonitor = hMonitor;
            info.rect = *lprcMonitor;

            MONITORINFOEXW mi = {0};
            mi.cbSize = sizeof(MONITORINFOEXW);
            if (GetMonitorInfoW(hMonitor, &mi))
            {
                info.workArea = mi.rcWork;
                info.isPrimary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;
            }

            int dpiX, dpiY;
            GetDpiForMonitor(hMonitor, dpiX, dpiY);
            info.dpiX = dpiX;
            info.dpiY = dpiY;
            info.scaleX = static_cast<double>(dpiX) / 96.0;
            info.scaleY = static_cast<double>(dpiY) / 96.0;

            pMonitors->push_back(info);
            return TRUE;
        };

        EnumDisplayMonitors(nullptr, nullptr, callback, reinterpret_cast<LPARAM>(&monitors));
        return S_OK;
    }

    HRESULT Win11Platform::GetMonitorInfoForWindow(HWND hwnd, MonitorInfo& info)
    {
        HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        if (!hMonitor)
            return E_FAIL;

        MONITORINFOEXW mi = {0};
        mi.cbSize = sizeof(MONITORINFOEXW);
        if (!GetMonitorInfoW(hMonitor, &mi))
            return E_FAIL;

        info.hMonitor = hMonitor;
        info.rect = mi.rcMonitor;
        info.workArea = mi.rcWork;
        info.isPrimary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;

        return GetDpiForMonitor(hMonitor, info.dpiX, info.dpiY);
    }

    HRESULT Win11Platform::GetDpiForMonitor(HMONITOR hMonitor, int& dpiX, int& dpiY)
    {
        dpiX = 96;
        dpiY = 96;

        if (m_isWindows11)
        {
            if (FAILED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)))
            {
                dpiX = 96;
                dpiY = 96;
            }
        }
        else
        {
            HDC hdc = GetDC(nullptr);
            dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
            dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(nullptr, hdc);
        }

        return S_OK;
    }

    HRESULT Win11Platform::GetScaleFactorForMonitor(HMONITOR hMonitor, double& scaleX, double& scaleY)
    {
        int dpiX, dpiY;
        HRESULT hr = GetDpiForMonitor(hMonitor, dpiX, dpiY);
        if (SUCCEEDED(hr))
        {
            scaleX = static_cast<double>(dpiX) / 96.0;
            scaleY = static_cast<double>(dpiY) / 96.0;
        }
        return hr;
    }

    HRESULT Win11Platform::SetProcessDpiAwareness()
    {
        if (m_isWindows11)
        {
            return SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }
        else
        {
            return SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
        }
    }

    HRESULT Win11Platform::GetCursorPosition(POINT& pt)
    {
        if (GetCursorPos(&pt))
            return S_OK;
        return E_FAIL;
    }

    HRESULT Win11Platform::GetForegroundWindow(HWND& hwnd)
    {
        hwnd = ::GetForegroundWindow();
        return hwnd ? S_OK : E_FAIL;
    }

    bool Win11Platform::IsProcessElevated()
    {
        HANDLE hToken = nullptr;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
            return false;

        TOKEN_ELEVATION elevation = {0};
        DWORD size = sizeof(TOKEN_ELEVATION);

        if (!GetTokenInformation(hToken, TokenElevation, &elevation, size, &size))
        {
            CloseHandle(hToken);
            return false;
        }

        CloseHandle(hToken);
        return elevation.TokenIsElevated != 0;
    }

    bool Win11Platform::IsUWPApp(HWND hwnd)
    {
        DWORD style = GetWindowLongPtrW(hwnd, GWL_STYLE);
        return (style & WS_EX_NOREDIRECTIONBITMAP) != 0;
    }
}