#pragma once

#include <vector>
#include <string>
#include <memory>
#include <chrono>

namespace ShuTongWen
{
    namespace Language
    {
        struct SegmentPath;
        class LanguageModel;
        class ContextAnalyzer;
        class CandidateRanker;
        struct ContextInfo;
    }

    struct CompositionState
    {
        std::wstring text;
        std::wstring pinyin;
        int cursorPosition;
        bool isComposing;
        std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

        CompositionState() : cursorPosition(0), isComposing(false) {}
    };

    struct ContextState
    {
        std::wstring appName;
        std::wstring inputMode;
        std::vector<std::wstring> precedingWords;
        std::vector<std::wstring> followingWords;
        int cursorPositionInDocument;
        bool isCodeContext;
        bool isEmailContext;
        bool isUrlContext;

        ContextState() : cursorPositionInDocument(0), isCodeContext(false), 
                         isEmailContext(false), isUrlContext(false) {}
    };

    struct CandidateState
    {
        std::vector<CandidateItem> candidates;
        int selectedIndex;
        int pageIndex;
        int totalPages;

        CandidateState() : selectedIndex(0), pageIndex(0), totalPages(1) {}
    };

    struct UserIntentState
    {
        std::wstring detectedIntent;
        double confidence;
        std::vector<std::wstring> entities;

        UserIntentState() : confidence(0.0) {}
    };

    class InputSession
    {
    public:
        InputSession();
        ~InputSession();

        bool Initialize();
        void Uninitialize();

        void Start();
        void End();
        bool IsActive() const { return m_active; }

        HRESULT ProcessKeyDown(WPARAM wParam, LPARAM lParam, bool& handled);
        HRESULT ProcessKeyUp(WPARAM wParam, LPARAM lParam, bool& handled);

        const CompositionState& GetCompositionState() const { return m_composition; }
        const ContextState& GetContextState() const { return m_context; }
        const CandidateState& GetCandidateState() const { return m_candidates; }
        const UserIntentState& GetUserIntentState() const { return m_intent; }

        void UpdateContext(const std::wstring& appName, int cursorPos);
        void UpdateComposition(const std::wstring& text, const std::wstring& pinyin);
        void UpdateCandidates(const std::vector<CandidateItem>& candidates);

        void Commit();
        void Cancel();
        void SelectCandidate(int index);

        void Reset();

        void SetContextAnalyzer(std::shared_ptr<Language::ContextAnalyzer> analyzer);
        void SetLanguageModel(std::shared_ptr<Language::LanguageModel> model);
        void SetCandidateRanker(std::shared_ptr<Language::CandidateRanker> ranker);

    private:
        void UpdateUserIntent();
        void ApplyContextRules();
        void UpdateSegmentation();

        bool m_active;
        CompositionState m_composition;
        ContextState m_context;
        CandidateState m_candidates;
        UserIntentState m_intent;

        std::shared_ptr<Language::ContextAnalyzer> m_contextAnalyzer;
        std::shared_ptr<Language::LanguageModel> m_languageModel;
        std::shared_ptr<Language::CandidateRanker> m_candidateRanker;

        std::vector<Language::SegmentPath> m_segmentPaths;
        std::wstring m_previousInput;
    };
}