#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace ShuTongWen
{
    namespace Pipeline
    {
        struct PipelineContext
        {
            WPARAM wParam;
            LPARAM lParam;
            std::wstring inputText;
            std::wstring pinyin;
            std::vector<CandidateItem> candidates;
            std::wstring appName;
            int cursorPosition;
            bool handled;
            bool cancelled;
        };

        class IPipelineStage
        {
        public:
            virtual ~IPipelineStage() = default;
            virtual HRESULT Process(PipelineContext& context) = 0;
            virtual std::wstring GetName() const = 0;
        };

        class KeyEventStage : public IPipelineStage
        {
        public:
            HRESULT Process(PipelineContext& context) override;
            std::wstring GetName() const override { return L"KeyEvent"; }
        };

        class TokenizerStage : public IPipelineStage
        {
        public:
            HRESULT Process(PipelineContext& context) override;
            std::wstring GetName() const override { return L"Tokenizer"; }
        };

        class SegmentGraphStage : public IPipelineStage
        {
        public:
            HRESULT Process(PipelineContext& context) override;
            std::wstring GetName() const override { return L"SegmentGraph"; }
        };

        class LanguageModelStage : public IPipelineStage
        {
        public:
            HRESULT Process(PipelineContext& context) override;
            std::wstring GetName() const override { return L"LanguageModel"; }
        };

        class ContextAnalyzerStage : public IPipelineStage
        {
        public:
            HRESULT Process(PipelineContext& context) override;
            std::wstring GetName() const override { return L"ContextAnalyzer"; }
        };

        class CandidateRankerStage : public IPipelineStage
        {
        public:
            HRESULT Process(PipelineContext& context) override;
            std::wstring GetName() const override { return L"CandidateRanker"; }
        };

        class PluginHooksStage : public IPipelineStage
        {
        public:
            HRESULT Process(PipelineContext& context) override;
            std::wstring GetName() const override { return L"PluginHooks"; }
        };

        class InputPipeline
        {
        public:
            InputPipeline();
            ~InputPipeline();

            bool Initialize();
            void Uninitialize();

            HRESULT Process(WPARAM wParam, LPARAM lParam, PipelineContext& context);
            
            void AddStage(std::unique_ptr<IPipelineStage> stage);
            void RemoveStage(const std::wstring& stageName);
            void ClearStages();

            size_t GetStageCount() const { return m_stages.size(); }

        private:
            std::vector<std::unique_ptr<IPipelineStage>> m_stages;
        };
    }
}