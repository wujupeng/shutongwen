#pragma once

#include <windows.h>
#include <vector>
#include <string>

namespace ShuTongWen
{
    struct CandidateItem;

    class CandidateWindow
    {
    public:
        CandidateWindow();
        ~CandidateWindow();

        HRESULT Initialize(HINSTANCE hInstance, HWND hParent);
        HRESULT Uninitialize();

        HRESULT Show(const std::vector<CandidateItem>& candidates, int selectedIndex = 0);
        HRESULT Hide();
        HRESULT Update(const std::vector<CandidateItem>& candidates, int selectedIndex = 0);

        HRESULT SetPosition(int x, int y);
        HRESULT SetSize(int width, int height);

        bool IsVisible() const { return m_visible; }

        using CandidateSelectedCallback = std::function<void(int index)>;
        void SetCandidateSelectedCallback(CandidateSelectedCallback callback);

    private:
        static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        LRESULT OnPaint();
        LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
        LRESULT OnMouseClick(WPARAM wParam, LPARAM lParam);

        HWND m_hWnd;
        HINSTANCE m_hInstance;
        HWND m_hParent;
        bool m_visible;
        std::vector<CandidateItem> m_candidates;
        int m_selectedIndex;
        CandidateSelectedCallback m_callback;

        HFONT m_hFont;
        COLORREF m_textColor;
        COLORREF m_selectedColor;
        COLORREF m_bgColor;
    };
}