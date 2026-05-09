#include "tsf/TextLayoutTracker.h"
#include "utils/logger.h"
#include "utils/win32_utils.h"

namespace ShuTongWen
{
    TextLayoutTracker::TextLayoutTracker()
        : m_pContext(nullptr),
          m_pRange(nullptr)
    {
        ZeroMemory(&m_layoutInfo, sizeof(m_layoutInfo));
    }

    TextLayoutTracker::~TextLayoutTracker()
    {
        Uninitialize();
    }

    HRESULT TextLayoutTracker::Initialize()
    {
        Logger::Info("Initializing TextLayoutTracker...");
        return S_OK;
    }

    HRESULT TextLayoutTracker::Uninitialize()
    {
        Logger::Info("Uninitializing TextLayoutTracker...");

        if (m_pRange)
        {
            m_pRange->Release();
            m_pRange = nullptr;
        }

        if (m_pContext)
        {
            m_pContext->Release();
            m_pContext = nullptr;
        }

        return S_OK;
    }

    HRESULT TextLayoutTracker::TrackContext(ITfContext* pContext)
    {
        if (m_pContext)
        {
            m_pContext->Release();
        }

        m_pContext = pContext;
        if (m_pContext)
        {
            m_pContext->AddRef();
        }

        return UpdateLayout(pContext);
    }

    HRESULT TextLayoutTracker::UpdateLayout(ITfContext* pContext)
    {
        if (!pContext)
            return E_INVALIDARG;

        ITfView* pView = nullptr;
        HRESULT hr = pContext->GetActiveView(&pView);
        if (FAILED(hr))
            return hr;

        HWND hwnd = nullptr;
        hr = pView->GetHWnd(&hwnd);
        if (FAILED(hr))
        {
            pView->Release();
            return hr;
        }

        GetWindowInfo(hwnd);
        CalculateCaretPosition(pContext);
        CalculateCompositionRect(pContext);

        pView->Release();
        return S_OK;
    }

    const TextLayoutInfo& TextLayoutTracker::GetLayoutInfo() const
    {
        return m_layoutInfo;
    }

    HRESULT TextLayoutTracker::GetCaretPosition(POINT& pt) const
    {
        pt = m_layoutInfo.caretPos;
        return S_OK;
    }

    HRESULT TextLayoutTracker::GetCompositionRect(RECT& rect) const
    {
        rect = m_layoutInfo.compositionRect;
        return S_OK;
    }

    HRESULT TextLayoutTracker::GetCandidatePosition(POINT& pt) const
    {
        pt.x = m_layoutInfo.compositionRect.left;
        pt.y = m_layoutInfo.compositionRect.bottom + 5;
        return S_OK;
    }

    HRESULT TextLayoutTracker::AdjustForDpi(POINT& pt) const
    {
        if (m_layoutInfo.isHighDpi && m_layoutInfo.scaleFactor != 1.0)
        {
            pt.x = static_cast<int>(pt.x * m_layoutInfo.scaleFactor);
            pt.y = static_cast<int>(pt.y * m_layoutInfo.scaleFactor);
        }
        return S_OK;
    }

    HRESULT TextLayoutTracker::AdjustForMultiMonitor(POINT& pt) const
    {
        HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
        MONITORINFO info = { sizeof(MONITORINFO) };
        
        if (GetMonitorInfoW(hMonitor, &info))
        {
            pt.x = max(info.rcWork.left, min(pt.x, info.rcWork.right - 1));
            pt.y = max(info.rcWork.top, min(pt.y, info.rcWork.bottom - 1));
        }

        return S_OK;
    }

    TextLayoutTracker& TextLayoutTracker::Instance()
    {
        static TextLayoutTracker instance;
        return instance;
    }

    HRESULT TextLayoutTracker::GetWindowInfo(HWND hwnd)
    {
        m_layoutInfo.hwnd = hwnd;
        GetDpiForWindow(hwnd);
        GetWindowAppName(hwnd);

        RECT rect;
        GetClientRect(hwnd, &rect);
        MapWindowPoints(hwnd, nullptr, reinterpret_cast<POINT*>(&rect), 2);

        return S_OK;
    }

    HRESULT TextLayoutTracker::CalculateCaretPosition(ITfContext* pContext)
    {
        if (!pContext)
            return E_INVALIDARG;

        ITfRange* pRange = nullptr;
        HRESULT hr = pContext->GetSelection(0, TF_DEFAULT_SELECTION, &pRange);
        if (FAILED(hr))
            return hr;

        ITfContextView* pView = nullptr;
        hr = pContext->GetActiveView(&pView);
        if (FAILED(hr))
        {
            pRange->Release();
            return hr;
        }

        TfEditCookie ec;
        hr = pContext->RequestEditSession(0, &ec, TF_ES_READ, nullptr);
        if (FAILED(hr))
        {
            pView->Release();
            pRange->Release();
            return hr;
        }

        RECT rect;
        hr = pView->GetTextExt(ec, pRange, &rect);
        if (SUCCEEDED(hr))
        {
            m_layoutInfo.caretPos.x = rect.left;
            m_layoutInfo.caretPos.y = rect.top;
        }

        pView->Release();
        pRange->Release();
        return hr;
    }

    HRESULT TextLayoutTracker::CalculateCompositionRect(ITfContext* pContext)
    {
        if (!pContext)
            return E_INVALIDARG;

        ITfContextView* pView = nullptr;
        HRESULT hr = pContext->GetActiveView(&pView);
        if (FAILED(hr))
            return hr;

        m_layoutInfo.compositionRect = {
            m_layoutInfo.caretPos.x,
            m_layoutInfo.caretPos.y,
            m_layoutInfo.caretPos.x + 200,
            m_layoutInfo.caretPos.y + 24
        };

        pView->Release();
        return S_OK;
    }

    HRESULT TextLayoutTracker::GetDpiForWindow(HWND hwnd)
    {
        if (Win32Utils::IsWindows11OrLater())
        {
            m_layoutInfo.dpi = GetDpiForWindow(hwnd);
        }
        else
        {
            HDC hdc = GetDC(hwnd);
            m_layoutInfo.dpi = GetDeviceCaps(hdc, LOGPIXELSX);
            ReleaseDC(hwnd, hdc);
        }

        m_layoutInfo.scaleFactor = m_layoutInfo.dpi / 96.0;
        m_layoutInfo.isHighDpi = m_layoutInfo.dpi > 96;

        return S_OK;
    }

    HRESULT TextLayoutTracker::GetWindowAppName(HWND hwnd)
    {
        WCHAR szPath[MAX_PATH];
        GetModuleFileNameW(GetWindowThreadProcessId(hwnd, nullptr), szPath, MAX_PATH);
        
        std::wstring path = szPath;
        size_t pos = path.find_last_of(L'\\');
        if (pos != std::wstring::npos)
        {
            m_layoutInfo.appName = path.substr(pos + 1);
        }

        return S_OK;
    }
}