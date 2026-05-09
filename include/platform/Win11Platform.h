#pragma once

#include <windows.h>
#include <string>
#include <vector>

namespace ShuTongWen
{
    struct MonitorInfo
    {
        HMONITOR hMonitor;
        RECT rect;
        RECT workArea;
        int dpiX;
        int dpiY;
        double scaleX;
        double scaleY;
        bool isPrimary;
    };

    struct WindowInfo
    {
        HWND hwnd;
        std::wstring className;
        std::wstring windowTitle;
        std::wstring processName;
        RECT rect;
        bool isForeground;
        bool isImmersive;
    };

    class Win11Platform
    {
    public:
        ~Win11Platform() = default;

        static Win11Platform& Instance();

        bool Initialize();
        void Uninitialize();

        bool IsWindows11OrLater() const;
        bool IsARM64() const;
        bool IsHighDpiEnabled() const;

        HRESULT GetCurrentWindowInfo(WindowInfo& info);
        HRESULT GetAllMonitors(std::vector<MonitorInfo>& monitors);
        HRESULT GetMonitorInfoForWindow(HWND hwnd, MonitorInfo& info);

        HRESULT GetDpiForMonitor(HMONITOR hMonitor, int& dpiX, int& dpiY);
        HRESULT GetScaleFactorForMonitor(HMONITOR hMonitor, double& scaleX, double& scaleY);

        HRESULT SetProcessDpiAwareness();
        HRESULT GetCursorPosition(POINT& pt);
        HRESULT GetForegroundWindow(HWND& hwnd);

        bool IsProcessElevated();
        bool IsUWPApp(HWND hwnd);

    private:
        Win11Platform();
        Win11Platform(const Win11Platform&) = delete;
        Win11Platform& operator=(const Win11Platform&) = delete;

        bool m_initialized;
        bool m_isWindows11;
        bool m_isARM64;
        bool m_isHighDpi;
    };
}