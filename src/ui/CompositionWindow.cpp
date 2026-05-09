#include "ui/CompositionWindow.h"
#include "ime/IMEFramework.h"
#include "utils/logger.h"

namespace ShuTongWen
{
    CompositionWindow::CompositionWindow()
        : m_hWnd(nullptr),
          m_hInstance(nullptr),
          m_hParent(nullptr),
          m_visible(false),
          m_hFont(nullptr),
          m_cursorPos(0)
    {
        m_composition.preedit.clear();
        m_composition.committed.clear();
        m_composition.cursor_pos = 0;
    }

    CompositionWindow::~CompositionWindow()
    {
        Uninitialize();
    }

    HRESULT CompositionWindow::Initialize(HINSTANCE hInstance, HWND hParent)
    {
        Logger::Info("Initializing CompositionWindow...");

        m_hInstance = hInstance;
        m_hParent = hParent;

        WNDCLASSEXW wc = {0};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"ShuTongWen_CompositionWindow";
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.style = CS_HREDRAW | CS_VREDRAW;

        if (!RegisterClassExW(&wc))
        {
            Logger::Error("Failed to register composition window class");
            return E_FAIL;
        }

        m_hFont = CreateFontW(
            18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Microsoft YaHei"
        );

        m_textColor = RGB(0, 0, 0);
        m_bgColor = RGB(255, 255, 255);

        Logger::Info("CompositionWindow initialized successfully");
        return S_OK;
    }

    HRESULT CompositionWindow::Uninitialize()
    {
        Logger::Info("Uninitializing CompositionWindow...");

        if (m_hWnd)
        {
            DestroyWindow(m_hWnd);
            m_hWnd = nullptr;
        }

        if (m_hFont)
        {
            DeleteObject(m_hFont);
            m_hFont = nullptr;
        }

        UnregisterClassW(L"ShuTongWen_CompositionWindow", m_hInstance);
        m_visible = false;

        Logger::Info("CompositionWindow uninitialized successfully");
        return S_OK;
    }

    HRESULT CompositionWindow::Show(const CompositionString& composition)
    {
        m_composition = composition;
        m_cursorPos = composition.cursor_pos;

        if (!m_hWnd)
        {
            m_hWnd = CreateWindowExW(
                WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
                L"ShuTongWen_CompositionWindow",
                L"",
                WS_POPUP | WS_VISIBLE,
                0, 0, 200, 30,
                m_hParent,
                nullptr,
                m_hInstance,
                this
            );

            if (!m_hWnd)
            {
                Logger::Error("Failed to create composition window");
                return E_FAIL;
            }
        }

        UpdateWindow(m_hWnd);
        m_visible = true;

        return S_OK;
    }

    HRESULT CompositionWindow::Hide()
    {
        if (m_hWnd)
        {
            ShowWindow(m_hWnd, SW_HIDE);
        }
        m_visible = false;
        return S_OK;
    }

    HRESULT CompositionWindow::Update(const CompositionString& composition)
    {
        m_composition = composition;
        m_cursorPos = composition.cursor_pos;

        if (m_hWnd)
        {
            UpdateWindow(m_hWnd);
        }

        return S_OK;
    }

    HRESULT CompositionWindow::SetPosition(int x, int y)
    {
        if (m_hWnd)
        {
            SetWindowPos(m_hWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
        }
        return S_OK;
    }

    LRESULT CALLBACK CompositionWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CompositionWindow* pThis = nullptr;

        if (uMsg == WM_CREATE)
        {
            CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
            pThis = reinterpret_cast<CompositionWindow*>(pCreate->lpCreateParams);
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        }
        else
        {
            pThis = reinterpret_cast<CompositionWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        }

        if (pThis)
        {
            switch (uMsg)
            {
            case WM_PAINT:
                return pThis->OnPaint();
            case WM_DESTROY:
                pThis->m_hWnd = nullptr;
                break;
            }
        }

        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    LRESULT CompositionWindow::OnPaint()
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(m_hWnd, &ps);

        RECT rect;
        GetClientRect(m_hWnd, &rect);

        HBRUSH bgBrush = CreateSolidBrush(m_bgColor);
        FillRect(hdc, &rect, bgBrush);
        DeleteObject(bgBrush);

        SelectObject(hdc, m_hFont);
        SetTextColor(hdc, m_textColor);
        SetBkMode(hdc, TRANSPARENT);

        RECT textRect = {5, 0, rect.right - 5, rect.bottom};
        DrawTextW(hdc, m_composition.preedit.c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        if (m_cursorPos >= 0 && m_cursorPos <= static_cast<int>(m_composition.preedit.length()))
        {
            SIZE size;
            std::wstring beforeCursor = m_composition.preedit.substr(0, m_cursorPos);
            GetTextExtentPoint32W(hdc, beforeCursor.c_str(), static_cast<int>(beforeCursor.length()), &size);

            int cursorX = 5 + size.cx;
            int cursorY = 5;
            int cursorHeight = rect.bottom - 10;

            HPEN cursorPen = CreatePen(PS_SOLID, 1, m_textColor);
            SelectObject(hdc, cursorPen);
            MoveToEx(hdc, cursorX, cursorY, nullptr);
            LineTo(hdc, cursorX, cursorY + cursorHeight);
            DeleteObject(cursorPen);
        }

        EndPaint(m_hWnd, &ps);
        return 0;
    }
}