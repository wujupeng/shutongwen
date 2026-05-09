#pragma once

#include <windows.h>
#include <memory>
#include <string>

namespace ShuTongWen
{
    struct TextLayoutInfo
    {
        HWND hwnd;
        POINT caretPos;
        RECT compositionRect;
        RECT candidateRect;
        int dpi;
        double scaleFactor;
        bool isHighDpi;
        std::wstring appName;
    };

    class TextLayoutTracker
    {
    public:
        TextLayoutTracker();
        ~TextLayoutTracker();

        HRESULT Initialize();
        HRESULT Uninitialize();

        HRESULT TrackContext(ITfContext* pContext);
        HRESULT UpdateLayout(ITfContext* pContext);

        const TextLayoutInfo& GetLayoutInfo() const;
        HRESULT GetCaretPosition(POINT& pt) const;
        HRESULT GetCompositionRect(RECT& rect) const;
        HRESULT GetCandidatePosition(POINT& pt) const;

        HRESULT AdjustForDpi(POINT& pt) const;
        HRESULT AdjustForMultiMonitor(POINT& pt) const;

        static TextLayoutTracker& Instance();

    private:
        TextLayoutInfo m_layoutInfo;
        ITfContext* m_pContext;
        ITfRange* m_pRange;

        HRESULT GetWindowInfo(HWND hwnd);
        HRESULT CalculateCaretPosition(ITfContext* pContext);
        HRESULT CalculateCompositionRect(ITfContext* pContext);
        HRESULT GetDpiForWindow(HWND hwnd);
        HRESULT GetWindowAppName(HWND hwnd);
    };
}