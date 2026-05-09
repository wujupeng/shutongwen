#pragma once

#include <windows.h>
#include <memory>

namespace ShuTongWen
{
    class CandidateWindow;
    class CompositionWindow;

    class IMEWindow
    {
    public:
        IMEWindow();
        ~IMEWindow();

        HRESULT Initialize(HINSTANCE hInstance);
        HRESULT Uninitialize();

        HRESULT ShowCandidateWindow(HWND hParent, const RECT& caretRect);
        HRESULT HideCandidateWindow();

        HRESULT ShowCompositionWindow(HWND hParent, const RECT& caretRect);
        HRESULT HideCompositionWindow();

        HRESULT UpdateCandidates(const std::vector<CandidateItem>& candidates, int selectedIndex = 0);
        HRESULT UpdateComposition(const CompositionString& composition);

        void SetCandidateSelectedCallback(std::function<void(int)> callback);

    private:
        std::unique_ptr<CandidateWindow> m_candidateWindow;
        std::unique_ptr<CompositionWindow> m_compositionWindow;
        HINSTANCE m_hInstance;
    };
}