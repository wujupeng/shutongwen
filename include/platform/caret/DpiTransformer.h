#pragma once

#include <windows.h>

namespace ShuTongWen
{
    namespace Platform
    {
        class DpiTransformer
        {
        public:
            ~DpiTransformer() = default;

            static DpiTransformer& Instance();

            UINT GetSystemDpi();
            UINT GetWindowDpi(HWND hwnd);
            UINT GetMonitorDpi(HMONITOR monitor);

            POINT LogicalToPhysical(HWND hwnd, const POINT& logical);
            POINT PhysicalToLogical(HWND hwnd, const POINT& physical);

            RECT LogicalToPhysicalRect(HWND hwnd, const RECT& logical);
            RECT PhysicalToLogicalRect(HWND hwnd, const RECT& physical);

            int ScaleForDpi(int value, UINT dpi);
            int UnscaleForDpi(int value, UINT dpi);

            void GetMonitorInfo(HMONITOR monitor, MONITORINFOEX& info);
            bool IsHighDpiWindow(HWND hwnd) const;

        private:
            DpiTransformer();

            float GetScaleFactor(UINT dpi) const;
        };
    }
}