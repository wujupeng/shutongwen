#pragma once

#include <windows.h>
#include <vector>
#include <string>

namespace ShuTongWen
{
    namespace Platform
    {
        struct CaretPosition
        {
            POINT screenPos;
            POINT clientPos;
            HWND hwnd;
            RECT boundingRect;
            int lineHeight;
            int charWidth;
            bool isValid;

            CaretPosition() : lineHeight(0), charWidth(0), isValid(false) {}
        };

        struct WindowInfo
        {
            HWND hwnd;
            std::wstring className;
            std::wstring windowName;
            RECT rect;
            HMONITOR monitor;
            UINT dpi;
            bool isUWP;
            bool isChrome;
            bool isElectron;
        };

        class CaretLocator
        {
        public:
            ~CaretLocator() = default;

            static CaretLocator& Instance();

            CaretPosition GetCaretPosition();
            CaretPosition GetCaretPositionForWindow(HWND hwnd);

            WindowInfo GetActiveWindowInfo();
            WindowInfo GetWindowInfo(HWND hwnd);

            bool IsValidCaretPosition(const CaretPosition& pos) const;

        private:
            CaretLocator();

            bool GetCaretInfo(CaretPosition& pos, HWND hwnd);
            bool GetTextLayoutInfo(CaretPosition& pos, HWND hwnd);
            bool GetChromeCaretPosition(CaretPosition& pos, HWND hwnd);
            bool GetElectronCaretPosition(CaretPosition& pos, HWND hwnd);
        };
    }
}