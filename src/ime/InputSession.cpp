#include "ime/InputSession.h"
#include "ime/language/ContextAnalyzer.h"
#include "ime/language/LanguageModel.h"
#include "ime/language/CandidateRanker.h"
#include "utils/logger.h"
#include <chrono>

namespace ShuTongWen
{
    InputSession::InputSession()
        : m_active(false)
    {}

    InputSession::~InputSession()
    {
        Uninitialize();
    }

    bool InputSession::Initialize()
    {
        Logger::Info("Initializing InputSession...");
        m_contextAnalyzer = std::make_shared<Language::ContextAnalyzer>();
        m_languageModel = Language::CreateLanguageModel();
        m_candidateRanker = std::make_shared<Language::CandidateRanker>();

        m_contextAnalyzer->Initialize();
        m_candidateRanker->Initialize();
        
        Logger::Info("InputSession initialized successfully");
        return true;
    }

    void InputSession::Uninitialize()
    {
        Reset();
        m_contextAnalyzer.reset();
        m_languageModel.reset();
        m_candidateRanker.reset();
    }

    void InputSession::Start()
    {
        m_active = true;
        m_composition.startTime = std::chrono::high_resolution_clock::now();
        Logger::Debug("InputSession started");
    }

    void InputSession::End()
    {
        m_active = false;
        Logger::Debug("InputSession ended");
    }

    HRESULT InputSession::ProcessKeyDown(WPARAM wParam, LPARAM lParam, bool& handled)
    {
        if (!m_active)
            return S_FALSE;

        handled = false;
        return S_OK;
    }

    HRESULT InputSession::ProcessKeyUp(WPARAM wParam, LPARAM lParam, bool& handled)
    {
        if (!m_active)
            return S_FALSE;

        handled = false;
        return S_OK;
    }

    void InputSession::UpdateContext(const std::wstring& appName, int cursorPos)
    {
        m_context.appName = appName;
        m_context.cursorPositionInDocument = cursorPos;

        if (m_contextAnalyzer)
        {
            Language::ContextInfo info = m_contextAnalyzer->Analyze(L"", cursorPos, appName);
            m_context.inputMode = info.inputMode;
            m_context.precedingWords = info.precedingWords;
            m_context.isCodeContext = m_contextAnalyzer->IsCodeContext(appName);
            m_context.isEmailContext = m_contextAnalyzer->IsEmailContext(L"");
            m_context.isUrlContext = m_contextAnalyzer->IsUrlContext(L"");
        }

        ApplyContextRules();
    }

    void InputSession::UpdateComposition(const std::wstring& text, const std::wstring& pinyin)
    {
        m_composition.text = text;
        m_composition.pinyin = pinyin;
        m_composition.isComposing = !text.empty();

        UpdateSegmentation();
        UpdateUserIntent();
    }

    void InputSession::UpdateCandidates(const std::vector<CandidateItem>& candidates)
    {
        m_candidates.candidates = candidates;
        m_candidates.selectedIndex = 0;
        m_candidates.totalPages = (candidates.size() + 9) / 10;
    }

    void InputSession::Commit()
    {
        m_composition.isComposing = false;
        m_candidates.candidates.clear();
        m_previousInput = m_composition.text;
        
        Logger::Debug("InputSession committed: {}", m_composition.text);
    }

    void InputSession::Cancel()
    {
        Reset();
        Logger::Debug("InputSession cancelled");
    }

    void InputSession::SelectCandidate(int index)
    {
        if (index >= 0 && index < static_cast<int>(m_candidates.candidates.size()))
        {
            m_candidates.selectedIndex = index;
            
            if (m_candidateRanker)
            {
                auto now = std::chrono::high_resolution_clock::now();
                uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()).count();
                m_candidateRanker->UpdateUserBehavior(m_candidates.candidates[index].text, timestamp);
            }
        }
    }

    void InputSession::Reset()
    {
        m_composition = CompositionState();
        m_context = ContextState();
        m_candidates = CandidateState();
        m_intent = UserIntentState();
        m_segmentPaths.clear();
        m_previousInput.clear();
    }

    void InputSession::SetContextAnalyzer(std::shared_ptr<Language::ContextAnalyzer> analyzer)
    {
        m_contextAnalyzer = analyzer;
    }

    void InputSession::SetLanguageModel(std::shared_ptr<Language::LanguageModel> model)
    {
        m_languageModel = model;
    }

    void InputSession::SetCandidateRanker(std::shared_ptr<Language::CandidateRanker> ranker)
    {
        m_candidateRanker = ranker;
    }

    void InputSession::UpdateUserIntent()
    {
        m_intent.detectedIntent = L"";
        m_intent.confidence = 0.0;
        m_intent.entities.clear();

        if (m_context.isEmailContext)
        {
            m_intent.detectedIntent = L"email";
            m_intent.confidence = 0.9;
        }
        else if (m_context.isUrlContext)
        {
            m_intent.detectedIntent = L"url";
            m_intent.confidence = 0.9;
        }
        else if (m_context.isCodeContext)
        {
            m_intent.detectedIntent = L"code";
            m_intent.confidence = 0.8;
        }
        else if (m_composition.text.find(L"@") != std::wstring::npos)
        {
            m_intent.detectedIntent = L"mention";
            m_intent.confidence = 0.7;
        }
    }

    void InputSession::ApplyContextRules()
    {
        if (m_context.isEmailContext || m_context.isUrlContext)
        {
            m_context.inputMode = L"english";
        }
        else if (m_context.isCodeContext)
        {
            m_context.inputMode = L"mixed";
        }
    }

    void InputSession::UpdateSegmentation()
    {
        if (m_languageModel && !m_composition.pinyin.empty())
        {
            if (m_composition.pinyin == m_previousInput)
            {
                return;
            }

            m_segmentPaths = m_languageModel->Segment(m_composition.pinyin, 5);
            m_previousInput = m_composition.pinyin;

            if (!m_segmentPaths.empty() && m_candidateRanker)
            {
                std::vector<Language::Candidate> candidates;
                for (const auto& path : m_segmentPaths)
                {
                    auto pathCandidates = m_languageModel->GenerateCandidates(path.segments, 20);
                    candidates.insert(candidates.end(), pathCandidates.begin(), pathCandidates.end());
                }

                std::vector<CandidateItem> result;
                for (const auto& c : candidates)
                {
                    CandidateItem item;
                    item.text = c.text;
                    item.pinyin = c.pinyin;
                    item.score = c.score;
                    item.frequency = c.frequency;
                    item.category = c.category;
                    result.push_back(item);
                }

                UpdateCandidates(result);
            }
        }
    }
}