#include "platform/caret/DpiTransformer.h"
#include "utils/logger.h"

namespace ShuTongWen
{
    namespace Platform
    {
        DpiTransformer::DpiTransformer()
        {}

        DpiTransformer& DpiTransformer::Instance()
        {
            static DpiTransformer instance;
            return instance;
        }

        UINT DpiTransformer::GetSystemDpi()
        {
            HDC hdc = GetDC(nullptr);
            if (!hdc)
                return 96;

            UINT dpi = GetDeviceCaps(hdc, LOGPIXELSX);
            ReleaseDC(nullptr, hdc);
            return dpi;
        }

        UINT DpiTransformer::GetWindowDpi(HWND hwnd)
        {
            if (!hwnd)
                return GetSystemDpi();

            HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
            return GetMonitorDpi(hMonitor);
        }

        UINT DpiTransformer::GetMonitorDpi(HMONITOR monitor)
        {
            UINT dpiX = 96, dpiY = 96;
            
            if (monitor)
            {
                GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
            }
            
            return dpiX;
        }

        POINT DpiTransformer::LogicalToPhysical(HWND hwnd, const POINT& logical)
        {
            UINT dpi = GetWindowDpi(hwnd);
            float scale = GetScaleFactor(dpi);

            POINT physical;
            physical.x = static_cast<int>(logical.x * scale);
            physical.y = static_cast<int>(logical.y * scale);
            return physical;
        }

        POINT DpiTransformer::PhysicalToLogical(HWND hwnd, const POINT& physical)
        {
            UINT dpi = GetWindowDpi(hwnd);
            float scale = GetScaleFactor(dpi);

            POINT logical;
            logical.x = static_cast<int>(physical.x / scale);
            logical.y = static_cast<int>(physical.y / scale);
            return logical;
        }

        RECT DpiTransformer::LogicalToPhysicalRect(HWND hwnd, const RECT& logical)
        {
            UINT dpi = GetWindowDpi(hwnd);
            float scale = GetScaleFactor(dpi);

            RECT physical;
            physical.left = static_cast<int>(logical.left * scale);
            physical.top = static_cast<int>(logical.top * scale);
            physical.right = static_cast<int>(logical.right * scale);
            physical.bottom = static_cast<int>(logical.bottom * scale);
            return physical;
        }

        RECT DpiTransformer::PhysicalToLogicalRect(HWND hwnd, const RECT& physical)
        {
            UINT dpi = GetWindowDpi(hwnd);
            float scale = GetScaleFactor(dpi);

            RECT logical;
            logical.left = static_cast<int>(physical.left / scale);
            logical.top = static_cast<int>(physical.top / scale);
            logical.right = static_cast<int>(physical.right / scale);
            logical.bottom = static_cast<int>(physical.bottom / scale);
            return logical;
        }

        int DpiTransformer::ScaleForDpi(int value, UINT dpi)
        {
            float scale = GetScaleFactor(dpi);
            return static_cast<int>(value * scale);
        }

        int DpiTransformer::UnscaleForDpi(int value, UINT dpi)
        {
            float scale = GetScaleFactor(dpi);
            return static_cast<int>(value / scale);
        }

        void DpiTransformer::GetMonitorInfo(HMONITOR monitor, MONITORINFOEX& info)
        {
            info.cbSize = sizeof(MONITORINFOEX);
            GetMonitorInfoW(monitor, &info);
        }

        bool DpiTransformer::IsHighDpiWindow(HWND hwnd) const
        {
            UINT dpi = GetWindowDpi(hwnd);
            return dpi > 96;
        }

        float DpiTransformer::GetScaleFactor(UINT dpi) const
        {
            return static_cast<float>(dpi) / 96.0f;
        }
    }
}