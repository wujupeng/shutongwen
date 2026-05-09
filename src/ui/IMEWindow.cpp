#include "ui/IMEWindow.h"
#include "ui/CandidateWindow.h"
#include "ui/CompositionWindow.h"
#include "ime/IMEFramework.h"
#include "utils/logger.h"

namespace ShuTongWen
{
    IMEWindow::IMEWindow()
        : m_hInstance(nullptr)
    {}

    IMEWindow::~IMEWindow()
    {
        Uninitialize();
    }

    HRESULT IMEWindow::Initialize(HINSTANCE hInstance)
    {
        Logger::Info("Initializing IMEWindow...");

        m_hInstance = hInstance;

        m_candidateWindow = std::make_unique<CandidateWindow>();
        if (FAILED(m_candidateWindow->Initialize(hInstance, nullptr)))
        {
            Logger::Error("Failed to initialize candidate window");
            return E_FAIL;
        }

        m_compositionWindow = std::make_unique<CompositionWindow>();
        if (FAILED(m_compositionWindow->Initialize(hInstance, nullptr)))
        {
            Logger::Error("Failed to initialize composition window");
            return E_FAIL;
        }

        Logger::Info("IMEWindow initialized successfully");
        return S_OK;
    }

    HRESULT IMEWindow::Uninitialize()
    {
        Logger::Info("Uninitializing IMEWindow...");

        m_candidateWindow.reset();
        m_compositionWindow.reset();

        Logger::Info("IMEWindow uninitialized successfully");
        return S_OK;
    }

    HRESULT IMEWindow::ShowCandidateWindow(HWND hParent, const RECT& caretRect)
    {
        if (!m_candidateWindow)
            return E_FAIL;

        int x = caretRect.left;
        int y = caretRect.bottom + 5;

        m_candidateWindow->SetPosition(x, y);

        if (!m_candidateWindow->IsVisible())
        {
            std::vector<CandidateItem> empty;
            m_candidateWindow->Show(empty);
        }

        return S_OK;
    }

    HRESULT IMEWindow::HideCandidateWindow()
    {
        if (m_candidateWindow)
        {
            m_candidateWindow->Hide();
        }
        return S_OK;
    }

    HRESULT IMEWindow::ShowCompositionWindow(HWND hParent, const RECT& caretRect)
    {
        if (!m_compositionWindow)
            return E_FAIL;

        int x = caretRect.left;
        int y = caretRect.top - 5;

        m_compositionWindow->SetPosition(x, y);

        if (!m_compositionWindow->IsVisible())
        {
            CompositionString empty;
            m_compositionWindow->Show(empty);
        }

        return S_OK;
    }

    HRESULT IMEWindow::HideCompositionWindow()
    {
        if (m_compositionWindow)
        {
            m_compositionWindow->Hide();
        }
        return S_OK;
    }

    HRESULT IMEWindow::UpdateCandidates(const std::vector<CandidateItem>& candidates, int selectedIndex)
    {
        if (m_candidateWindow)
        {
            if (candidates.empty())
            {
                m_candidateWindow->Hide();
            }
            else
            {
                if (!m_candidateWindow->IsVisible())
                {
                    m_candidateWindow->Show(candidates, selectedIndex);
                }
                else
                {
                    m_candidateWindow->Update(candidates, selectedIndex);
                }
            }
        }
        return S_OK;
    }

    HRESULT IMEWindow::UpdateComposition(const CompositionString& composition)
    {
        if (m_compositionWindow)
        {
            if (composition.preedit.empty())
            {
                m_compositionWindow->Hide();
            }
            else
            {
                if (!m_compositionWindow->IsVisible())
                {
                    m_compositionWindow->Show(composition);
                }
                else
                {
                    m_compositionWindow->Update(composition);
                }
            }
        }
        return S_OK;
    }

    void IMEWindow::SetCandidateSelectedCallback(std::function<void(int)> callback)
    {
        if (m_candidateWindow)
        {
            m_candidateWindow->SetCandidateSelectedCallback(callback);
        }
    }
}