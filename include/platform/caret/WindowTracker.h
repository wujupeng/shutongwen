#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <functional>

namespace ShuTongWen
{
    namespace Platform
    {
        enum class WindowEvent
        {
            FocusChanged,
            Move,
            Resize,
            Close,
            DpiChanged,
            ThemeChanged
        };

        struct WindowState
        {
            HWND hwnd;
            RECT rect;
            UINT dpi;
            bool hasFocus;
            std::wstring className;
            std::wstring title;
        };

        class WindowTracker
        {
        public:
            ~WindowTracker() = default;

            static WindowTracker& Instance();

            bool Initialize();
            void Uninitialize();

            void StartTracking(HWND hwnd);
            void StopTracking(HWND hwnd);

            WindowState GetWindowState(HWND hwnd) const;
            std::vector<WindowState> GetAllTrackedWindows() const;

            using WindowCallback = std::function<void(HWND hwnd, WindowEvent event)>;
            void SetWindowCallback(WindowCallback callback);

            HWND GetForegroundWindow();
            bool IsWindowVisible(HWND hwnd) const;

        private:
            WindowTracker();

            static LRESULT CALLBACK MessageHook(int code, WPARAM wParam, LPARAM lParam);

            bool m_hookInstalled;
            HHOOK m_messageHook;
            WindowCallback m_callback;
            std::unordered_map<HWND, WindowState> m_windowStates;
        };
    }
}