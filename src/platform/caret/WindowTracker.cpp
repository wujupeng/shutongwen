#include "platform/caret/WindowTracker.h"
#include "utils/logger.h"

namespace ShuTongWen
{
    namespace Platform
    {
        WindowTracker::WindowTracker()
            : m_hookInstalled(false),
              m_messageHook(nullptr)
        {}

        WindowTracker& WindowTracker::Instance()
        {
            static WindowTracker instance;
            return instance;
        }

        bool WindowTracker::Initialize()
        {
            Logger::Info("Initializing WindowTracker...");

            m_messageHook = SetWindowsHookEx(WH_CALLWNDPROC, MessageHook, nullptr, GetCurrentThreadId());
            if (!m_messageHook)
            {
                Logger::Error("Failed to install window hook");
                return false;
            }

            m_hookInstalled = true;
            Logger::Info("WindowTracker initialized successfully");
            return true;
        }

        void WindowTracker::Uninitialize()
        {
            Logger::Info("Uninitializing WindowTracker...");

            if (m_hookInstalled && m_messageHook)
            {
                UnhookWindowsHookEx(m_messageHook);
                m_messageHook = nullptr;
            }

            m_hookInstalled = false;
            m_windowStates.clear();
        }

        void WindowTracker::StartTracking(HWND hwnd)
        {
            WindowState state;
            state.hwnd = hwnd;
            GetWindowRect(hwnd, &state.rect);
            
            HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
            UINT dpi = 96;
            GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpi, &dpi);
            state.dpi = dpi;

            state.hasFocus = hwnd == GetForegroundWindow();

            wchar_t className[256];
            GetClassNameW(hwnd, className, sizeof(className) / sizeof(wchar_t));
            state.className = className;

            wchar_t title[256];
            GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t));
            state.title = title;

            m_windowStates[hwnd] = state;

            Logger::Debug("Started tracking window: {}", state.title);
        }

        void WindowTracker::StopTracking(HWND hwnd)
        {
            m_windowStates.erase(hwnd);
            Logger::Debug("Stopped tracking window");
        }

        WindowState WindowTracker::GetWindowState(HWND hwnd) const
        {
            auto it = m_windowStates.find(hwnd);
            if (it != m_windowStates.end())
            {
                return it->second;
            }

            WindowState state;
            state.hwnd = hwnd;
            return state;
        }

        std::vector<WindowState> WindowTracker::GetAllTrackedWindows() const
        {
            std::vector<WindowState> states;
            for (const auto& pair : m_windowStates)
            {
                states.push_back(pair.second);
            }
            return states;
        }

        void WindowTracker::SetWindowCallback(WindowCallback callback)
        {
            m_callback = callback;
        }

        HWND WindowTracker::GetForegroundWindow()
        {
            return ::GetForegroundWindow();
        }

        bool WindowTracker::IsWindowVisible(HWND hwnd) const
        {
            return IsWindowVisible(hwnd) && !IsIconic(hwnd);
        }

        LRESULT CALLBACK WindowTracker::MessageHook(int code, WPARAM wParam, LPARAM lParam)
        {
            if (code < 0)
                return CallNextHookEx(nullptr, code, wParam, lParam);

            CWPSTRUCT* cwp = reinterpret_cast<CWPSTRUCT*>(lParam);
            
            WindowEvent event = WindowEvent::FocusChanged;
            bool shouldNotify = false;

            switch (cwp->message)
            {
            case WM_SETFOCUS:
                event = WindowEvent::FocusChanged;
                shouldNotify = true;
                break;
            case WM_MOVE:
                event = WindowEvent::Move;
                shouldNotify = true;
                break;
            case WM_SIZE:
                event = WindowEvent::Resize;
                shouldNotify = true;
                break;
            case WM_DESTROY:
                event = WindowEvent::Close;
                shouldNotify = true;
                break;
            case WM_DPICHANGED:
                event = WindowEvent::DpiChanged;
                shouldNotify = true;
                break;
            }

            if (shouldNotify && Instance().m_callback)
            {
                Instance().m_callback(cwp->hwnd, event);
            }

            return CallNextHookEx(nullptr, code, wParam, lParam);
        }
    }
}