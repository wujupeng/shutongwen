#include "ui/CandidateWindow.h"
#include "ime/IMEFramework.h"
#include "utils/logger.h"

namespace ShuTongWen
{
    CandidateWindow::CandidateWindow()
        : m_hWnd(nullptr),
          m_hInstance(nullptr),
          m_hParent(nullptr),
          m_visible(false),
          m_selectedIndex(0),
          m_hFont(nullptr)
    {}

    CandidateWindow::~CandidateWindow()
    {
        Uninitialize();
    }

    HRESULT CandidateWindow::Initialize(HINSTANCE hInstance, HWND hParent)
    {
        Logger::Info("Initializing CandidateWindow...");

        m_hInstance = hInstance;
        m_hParent = hParent;

        WNDCLASSEXW wc = {0};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"ShuTongWen_CandidateWindow";
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.style = CS_HREDRAW | CS_VREDRAW;

        if (!RegisterClassExW(&wc))
        {
            Logger::Error("Failed to register candidate window class");
            return E_FAIL;
        }

        m_hFont = CreateFontW(
            14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Microsoft YaHei"
        );

        m_textColor = RGB(0, 0, 0);
        m_selectedColor = RGB(0, 120, 215);
        m_bgColor = RGB(255, 255, 255);

        Logger::Info("CandidateWindow initialized successfully");
        return S_OK;
    }

    HRESULT CandidateWindow::Uninitialize()
    {
        Logger::Info("Uninitializing CandidateWindow...");

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

        UnregisterClassW(L"ShuTongWen_CandidateWindow", m_hInstance);
        m_visible = false;

        Logger::Info("CandidateWindow uninitialized successfully");
        return S_OK;
    }

    HRESULT CandidateWindow::Show(const std::vector<CandidateItem>& candidates, int selectedIndex)
    {
        m_candidates = candidates;
        m_selectedIndex = selectedIndex;

        if (!m_hWnd)
        {
            m_hWnd = CreateWindowExW(
                WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
                L"ShuTongWen_CandidateWindow",
                L"",
                WS_POPUP | WS_VISIBLE,
                0, 0, 300, 30,
                m_hParent,
                nullptr,
                m_hInstance,
                this
            );

            if (!m_hWnd)
            {
                Logger::Error("Failed to create candidate window");
                return E_FAIL;
            }
        }

        UpdateWindow(m_hWnd);
        m_visible = true;

        return S_OK;
    }

    HRESULT CandidateWindow::Hide()
    {
        if (m_hWnd)
        {
            ShowWindow(m_hWnd, SW_HIDE);
        }
        m_visible = false;
        return S_OK;
    }

    HRESULT CandidateWindow::Update(const std::vector<CandidateItem>& candidates, int selectedIndex)
    {
        m_candidates = candidates;
        m_selectedIndex = selectedIndex;

        if (m_hWnd)
        {
            UpdateWindow(m_hWnd);
        }

        return S_OK;
    }

    HRESULT CandidateWindow::SetPosition(int x, int y)
    {
        if (m_hWnd)
        {
            SetWindowPos(m_hWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
        }
        return S_OK;
    }

    HRESULT CandidateWindow::SetSize(int width, int height)
    {
        if (m_hWnd)
        {
            SetWindowPos(m_hWnd, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOACTIVATE);
        }
        return S_OK;
    }

    void CandidateWindow::SetCandidateSelectedCallback(CandidateSelectedCallback callback)
    {
        m_callback = callback;
    }

    LRESULT CALLBACK CandidateWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CandidateWindow* pThis = nullptr;

        if (uMsg == WM_CREATE)
        {
            CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
            pThis = reinterpret_cast<CandidateWindow*>(pCreate->lpCreateParams);
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        }
        else
        {
            pThis = reinterpret_cast<CandidateWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        }

        if (pThis)
        {
            switch (uMsg)
            {
            case WM_PAINT:
                return pThis->OnPaint();
            case WM_KEYDOWN:
                return pThis->OnKeyDown(wParam, lParam);
            case WM_LBUTTONDOWN:
                return pThis->OnMouseClick(wParam, lParam);
            case WM_DESTROY:
                pThis->m_hWnd = nullptr;
                break;
            }
        }

        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    LRESULT CandidateWindow::OnPaint()
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(m_hWnd, &ps);

        RECT rect;
        GetClientRect(m_hWnd, &rect);

        HBRUSH bgBrush = CreateSolidBrush(m_bgColor);
        FillRect(hdc, &rect, bgBrush);
        DeleteObject(bgBrush);

        if (!m_candidates.empty())
        {
            SelectObject(hdc, m_hFont);

            int itemHeight = 28;
            int x = 10;
            int y = 5;

            for (size_t i = 0; i < m_candidates.size(); ++i)
            {
                RECT itemRect = {x, y, rect.right - 10, y + itemHeight};

                if (static_cast<int>(i) == m_selectedIndex)
                {
                    HBRUSH selBrush = CreateSolidBrush(m_selectedColor);
                    FillRect(hdc, &itemRect, selBrush);
                    DeleteObject(selBrush);
                    SetTextColor(hdc, RGB(255, 255, 255));
                }
                else
                {
                    SetTextColor(hdc, m_textColor);
                }

                SetBkMode(hdc, TRANSPARENT);

                std::wstring display = std::to_wstring(i + 1) + L". " + m_candidates[i].text;
                DrawTextW(hdc, display.c_str(), -1, &itemRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

                y += itemHeight;
            }
        }

        EndPaint(m_hWnd, &ps);
        return 0;
    }

    LRESULT CandidateWindow::OnKeyDown(WPARAM wParam, LPARAM lParam)
    {
        if (wParam >= '1' && wParam <= '9')
        {
            int index = wParam - '1';
            if (index >= 0 && index < static_cast<int>(m_candidates.size()))
            {
                if (m_callback)
                {
                    m_callback(index);
                }
            }
        }
        else if (wParam == VK_UP)
        {
            if (m_selectedIndex > 0)
            {
                m_selectedIndex--;
                UpdateWindow(m_hWnd);
            }
        }
        else if (wParam == VK_DOWN)
        {
            if (m_selectedIndex < static_cast<int>(m_candidates.size()) - 1)
            {
                m_selectedIndex++;
                UpdateWindow(m_hWnd);
            }
        }
        else if (wParam == VK_RETURN)
        {
            if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_candidates.size()))
            {
                if (m_callback)
                {
                    m_callback(m_selectedIndex);
                }
            }
        }

        return 0;
    }

    LRESULT CandidateWindow::OnMouseClick(WPARAM wParam, LPARAM lParam)
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        int itemHeight = 28;
        int index = (y - 5) / itemHeight;

        if (index >= 0 && index < static_cast<int>(m_candidates.size()))
        {
            if (m_callback)
            {
                m_callback(index);
            }
        }

        return 0;
    }
}