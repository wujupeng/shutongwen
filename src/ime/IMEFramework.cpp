#include "ime/IMEFramework.h"
#include "ime/InputProcessor.h"
#include "ime/DictionaryManager.h"
#include "utils/logger.h"
#include <algorithm>

namespace ShuTongWen
{
    class IMEFrameworkImpl : public IMEFramework
    {
    public:
        IMEFrameworkImpl() 
            : m_status(IMEStatus::Disabled), 
              m_inputProcessor(nullptr)
        {}

        HRESULT Initialize(HINSTANCE hInstance) override
        {
            Logger::Info("Initializing IME Framework...");
            
            m_inputProcessor = InputProcessor::Create();
            if (!m_inputProcessor)
            {
                Logger::Error("Failed to create input processor");
                return E_FAIL;
            }

            std::wstring dbPath = L"dict/shutongwen.db";
            if (!DictionaryManager::Instance().Initialize(dbPath))
            {
                Logger::Warn("Failed to initialize dictionary, using fallback");
            }

            m_status = IMEStatus::Enabled;
            Logger::Info("IME Framework initialized successfully");
            return S_OK;
        }

        HRESULT Uninitialize() override
        {
            Logger::Info("Uninitializing IME Framework...");
            m_inputProcessor.reset();
            DictionaryManager::Instance().Uninitialize();
            m_status = IMEStatus::Disabled;
            return S_OK;
        }

        HRESULT SetStatus(IMEStatus status) override
        {
            m_status = status;
            if (m_statusChangedCallback)
            {
                m_statusChangedCallback(status);
            }
            return S_OK;
        }

        IMEStatus GetStatus() const override
        {
            return m_status;
        }

        HRESULT ProcessKeyDown(WPARAM wParam, LPARAM lParam, bool& handled) override
        {
            handled = false;
            
            if (m_status != IMEStatus::Enabled && m_status != IMEStatus::Composing)
                return S_OK;

            if (wParam == VK_BACK)
            {
                if (m_inputProcessor->ProcessBackspace())
                {
                    handled = true;
                    UpdateComposition();
                }
            }
            else if (wParam == VK_SPACE)
            {
                if (m_inputProcessor->ProcessSpace())
                {
                    handled = true;
                    UpdateComposition();
                }
            }
            else if (wParam == VK_RETURN)
            {
                if (m_inputProcessor->ProcessEnter())
                {
                    handled = true;
                    CommitCurrentText();
                }
            }
            else if (wParam == VK_ESCAPE)
            {
                if (m_inputProcessor->ProcessEscape())
                {
                    handled = true;
                    ClearComposition();
                }
            }
            else if (wParam >= 'A' && wParam <= 'Z')
            {
                wchar_t ch = static_cast<wchar_t>(wParam);
                if (m_inputProcessor->ProcessChar(ch))
                {
                    handled = true;
                    UpdateComposition();
                    UpdateCandidates();
                }
            }

            return S_OK;
        }

        HRESULT ProcessKeyUp(WPARAM wParam, LPARAM lParam, bool& handled) override
        {
            handled = false;
            return S_OK;
        }

        HRESULT Compose(const std::wstring& input, CompositionString& result) override
        {
            for (wchar_t ch : input)
            {
                m_inputProcessor->ProcessChar(ch);
            }
            UpdateComposition();
            result = m_composition;
            return S_OK;
        }

        HRESULT GetCandidates(std::vector<CandidateItem>& candidates) override
        {
            candidates = m_candidates;
            return S_OK;
        }

        HRESULT SelectCandidate(int index) override
        {
            if (index >= 0 && index < static_cast<int>(m_candidates.size()))
            {
                const CandidateItem& item = m_candidates[index];
                m_composition.committed += item.text;
                m_composition.preedit.clear();
                m_inputProcessor->Reset();
                UpdateComposition();
                CommitString(item.text);
            }
            return S_OK;
        }

        HRESULT CommitString(const std::wstring& str) override
        {
            if (m_statusChangedCallback)
            {
                m_statusChangedCallback(IMEStatus::Committed);
            }
            return S_OK;
        }

        HRESULT ClearComposition() override
        {
            m_inputProcessor->Reset();
            m_composition.preedit.clear();
            m_composition.cursor_pos = 0;
            m_candidates.clear();
            
            if (m_compositionChangedCallback)
            {
                m_compositionChangedCallback(m_composition);
            }
            if (m_candidateChangedCallback)
            {
                m_candidateChangedCallback(m_candidates);
            }
            
            SetStatus(IMEStatus::Enabled);
            return S_OK;
        }

        void SetCandidateChangedCallback(CandidateChangedCallback callback) override
        {
            m_candidateChangedCallback = callback;
        }

        void SetCompositionChangedCallback(CompositionChangedCallback callback) override
        {
            m_compositionChangedCallback = callback;
        }

        void SetStatusChangedCallback(StatusChangedCallback callback) override
        {
            m_statusChangedCallback = callback;
        }

    private:
        void UpdateComposition()
        {
            m_composition.preedit = m_inputProcessor->GetComposedText();
            m_composition.cursor_pos = static_cast<int>(m_composition.preedit.length());
            
            if (m_inputProcessor->IsComposing())
            {
                SetStatus(IMEStatus::Composing);
            }
            
            if (m_compositionChangedCallback)
            {
                m_compositionChangedCallback(m_composition);
            }
        }

        void UpdateCandidates()
        {
            std::wstring currentPinyin = m_inputProcessor->GetCurrentPinyin();
            if (!currentPinyin.empty())
            {
                auto entries = DictionaryManager::Instance().Lookup(currentPinyin);
                m_candidates.clear();
                for (const auto& entry : entries)
                {
                    CandidateItem item;
                    item.text = entry.word;
                    item.pinyin = !entry.pinyins.empty() ? entry.pinyins[0] : L"";
                    item.frequency = entry.frequency;
                    m_candidates.push_back(item);
                }
                
                std::sort(m_candidates.begin(), m_candidates.end(),
                    [](const CandidateItem& a, const CandidateItem& b) {
                        return a.frequency > b.frequency;
                    });
                
                if (m_candidateChangedCallback)
                {
                    m_candidateChangedCallback(m_candidates);
                }
            }
        }

        void CommitCurrentText()
        {
            std::wstring text = m_composition.preedit;
            if (!text.empty())
            {
                m_composition.committed += text;
                CommitString(text);
            }
            ClearComposition();
        }

        IMEStatus m_status;
        std::unique_ptr<InputProcessor> m_inputProcessor;
        CompositionString m_composition;
        std::vector<CandidateItem> m_candidates;

        CandidateChangedCallback m_candidateChangedCallback;
        CompositionChangedCallback m_compositionChangedCallback;
        StatusChangedCallback m_statusChangedCallback;
    };

    std::unique_ptr<IMEFramework> IMEFramework::Create()
    {
        return std::make_unique<IMEFrameworkImpl>();
    }
}