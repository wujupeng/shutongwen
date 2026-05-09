#include "platform/caret/CaretLocator.h"
#include "utils/logger.h"
#include <windowsx.h>

namespace ShuTongWen
{
    namespace Platform
    {
        CaretLocator::CaretLocator()
        {}

        CaretLocator& CaretLocator::Instance()
        {
            static CaretLocator instance;
            return instance;
        }

        CaretPosition CaretLocator::GetCaretPosition()
        {
            HWND hwnd = GetForegroundWindow();
            return GetCaretPositionForWindow(hwnd);
        }

        CaretPosition CaretLocator::GetCaretPositionForWindow(HWND hwnd)
        {
            CaretPosition pos;

            if (!hwnd)
                return pos;

            pos.hwnd = hwnd;
            
            if (!GetCaretInfo(pos, hwnd))
            {
                GetTextLayoutInfo(pos, hwnd);
            }

            return pos;
        }

        bool CaretLocator::GetCaretInfo(CaretPosition& pos, HWND hwnd)
        {
            POINT pt;
            if (GetCaretPos(&pt))
            {
                pos.clientPos = pt;
                
                if (ClientToScreen(hwnd, &pt))
                {
                    pos.screenPos = pt;
                    pos.isValid = true;
                    return true;
                }
            }
            return false;
        }

        bool CaretLocator::GetTextLayoutInfo(CaretPosition& pos, HWND hwnd)
        {
            TTFINDRESULT findResult = {0};
            findResult.cbStruct = sizeof(TTFINDRESULT);

            if (TF_FindCurrent(hwnd, &findResult) == S_OK)
            {
                pos.screenPos = findResult.rcCaret.leftTop;
                pos.boundingRect = findResult.rcCaret;
                pos.isValid = true;
                return true;
            }

            RECT rect;
            if (GetClientRect(hwnd, &rect))
            {
                POINT pt = {rect.left + 10, rect.top + 10};
                ClientToScreen(hwnd, &pt);
                pos.screenPos = pt;
                pos.isValid = true;
            }

            return pos.isValid;
        }

        bool CaretLocator::GetChromeCaretPosition(CaretPosition& pos, HWND hwnd)
        {
            return false;
        }

        bool CaretLocator::GetElectronCaretPosition(CaretPosition& pos, HWND hwnd)
        {
            return false;
        }

        WindowInfo CaretLocator::GetActiveWindowInfo()
        {
            HWND hwnd = GetForegroundWindow();
            return GetWindowInfo(hwnd);
        }

        WindowInfo CaretLocator::GetWindowInfo(HWND hwnd)
        {
            WindowInfo info;
            info.hwnd = hwnd;

            if (!hwnd)
                return info;

            wchar_t className[256];
            GetClassNameW(hwnd, className, sizeof(className) / sizeof(wchar_t));
            info.className = className;

            wchar_t windowName[256];
            GetWindowTextW(hwnd, windowName, sizeof(windowName) / sizeof(wchar_t));
            info.windowName = windowName;

            GetWindowRect(hwnd, &info.rect);
            info.monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

            HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
            UINT dpi = 96;
            
            HDC hdc = GetDC(nullptr);
            if (hdc)
            {
                dpi = GetDeviceCaps(hdc, LOGPIXELSX);
                ReleaseDC(nullptr, hdc);
            }
            info.dpi = dpi;

            info.isUWP = info.className.find(L"ApplicationFrameWindow") != std::wstring::npos;
            info.isChrome = info.className == L"Chrome_WidgetWin_1";
            info.isElectron = info.className.find(L"Electron") != std::wstring::npos ||
                            info.className.find(L"CEF") != std::wstring::npos;

            return info;
        }

        bool CaretLocator::IsValidCaretPosition(const CaretPosition& pos) const
        {
            return pos.isValid && pos.hwnd != nullptr;
        }
    }
}