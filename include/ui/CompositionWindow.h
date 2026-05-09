#pragma once

#include <windows.h>
#include <string>

namespace ShuTongWen
{
    struct CompositionString;

    class CompositionWindow
    {
    public:
        CompositionWindow();
        ~CompositionWindow();

        HRESULT Initialize(HINSTANCE hInstance, HWND hParent);
        HRESULT Uninitialize();

        HRESULT Show(const CompositionString& composition);
        HRESULT Hide();
        HRESULT Update(const CompositionString& composition);

        HRESULT SetPosition(int x, int y);

        bool IsVisible() const { return m_visible; }

    private:
        static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        LRESULT OnPaint();

        HWND m_hWnd;
        HINSTANCE m_hInstance;
        HWND m_hParent;
        bool m_visible;
        CompositionString m_composition;

        HFONT m_hFont;
        COLORREF m_textColor;
        COLORREF m_bgColor;
        int m_cursorPos;
    };
}