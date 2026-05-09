#include "ime/pipeline/InputPipeline.h"
#include "utils/logger.h"
#include "ime/MixedInputProcessor.h"
#include "ime/language/SegmentGraph.h"
#include "ime/language/LanguageModel.h"
#include "ime/language/ContextAnalyzer.h"
#include "ime/language/CandidateRanker.h"
#include "ime/adaptive/AdaptivePipeline.h"
#include "ime/perception/LatencyOptimizer.h"
#include "ime/cache/InputCache.h"

namespace ShuTongWen
{
    namespace Pipeline
    {
        HRESULT KeyEventStage::Process(PipelineContext& context)
        {
            Logger::Debug("Processing KeyEvent stage: wParam={}, lParam={}", 
                context.wParam, context.lParam);
            
            switch (context.wParam)
            {
            case VK_BACK:
                context.handled = true;
                break;
            case VK_RETURN:
                context.handled = true;
                break;
            case VK_ESCAPE:
                context.cancelled = true;
                context.handled = true;
                break;
            case VK_SPACE:
                context.handled = true;
                break;
            }
            
            return S_OK;
        }

        HRESULT TokenizerStage::Process(PipelineContext& context)
        {
            Logger::Debug("Processing Tokenizer stage: input={}", context.inputText);
            
            auto processor = MixedInputProcessor::Create();
            
            for (wchar_t ch : context.inputText)
            {
                processor->ProcessChar(ch);
            }
            
            context.pinyin = processor->GetCurrentPinyin();
            
            return S_OK;
        }

        HRESULT SegmentGraphStage::Process(PipelineContext& context)
        {
            Logger::Debug("Processing SegmentGraph stage: pinyin={}", context.pinyin);
            
            static Language::SegmentGraph segmentGraph;
            segmentGraph.BuildGraph(context.pinyin);
            
            return S_OK;
        }

        HRESULT LanguageModelStage::Process(PipelineContext& context)
        {
            Logger::Debug("Processing LanguageModel stage");
            
            static auto model = Language::CreateLanguageModel();
            
            if (!context.pinyin.empty())
            {
                auto paths = model->Segment(context.pinyin, 5);
                
                for (const auto& path : paths)
                {
                    auto candidates = model->GenerateCandidates(path.segments, 10);
                    for (const auto& c : candidates)
                    {
                        CandidateItem item;
                        item.text = c.text;
                        item.pinyin = c.pinyin;
                        item.score = c.score;
                        item.frequency = c.frequency;
                        context.candidates.push_back(item);
                    }
                }
            }
            
            return S_OK;
        }

        HRESULT ContextAnalyzerStage::Process(PipelineContext& context)
        {
            Logger::Debug("Processing ContextAnalyzer stage: app={}", context.appName);
            
            static Language::ContextAnalyzer analyzer;
            analyzer.Initialize();
            
            Language::ContextInfo info = analyzer.Analyze(L"", context.cursorPosition, context.appName);
            
            if (analyzer.IsCodeContext(context.appName))
            {
                context.candidates.erase(
                    std::remove_if(context.candidates.begin(), context.candidates.end(),
                        [](const CandidateItem& item) {
                            return item.category == L"emoji";
                        }),
                    context.candidates.end());
            }
            
            return S_OK;
        }

        HRESULT CandidateRankerStage::Process(PipelineContext& context)
        {
            Logger::Debug("Processing CandidateRanker stage: {} candidates", context.candidates.size());
            
            static Language::CandidateRanker ranker;
            ranker.Initialize();
            
            if (!context.candidates.empty())
            {
                std::vector<Language::Candidate> langCandidates;
                for (const auto& c : context.candidates)
                {
                    Language::Candidate lc;
                    lc.text = c.text;
                    lc.pinyin = c.pinyin;
                    lc.score = c.score;
                    lc.frequency = c.frequency;
                    langCandidates.push_back(lc);
                }
                
                auto now = std::chrono::high_resolution_clock::now();
                uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()).count();
                
                auto ranked = ranker.Rank(langCandidates, {}, context.inputText, timestamp);
                
                context.candidates.clear();
                for (const auto& c : ranked)
                {
                    CandidateItem item;
                    item.text = c.text;
                    item.pinyin = c.pinyin;
                    item.score = c.score;
                    item.frequency = c.frequency;
                    context.candidates.push_back(item);
                }
            }
            
            return S_OK;
        }

        HRESULT PluginHooksStage::Process(PipelineContext& context)
        {
            Logger::Debug("Processing PluginHooks stage");
            return S_OK;
        }

        InputPipeline::InputPipeline()
        {}

        InputPipeline::~InputPipeline()
        {
            Uninitialize();
        }

        bool InputPipeline::Initialize()
        {
            Logger::Info("Initializing InputPipeline...");
            
            m_stages.clear();
            m_stages.push_back(std::make_unique<KeyEventStage>());
            m_stages.push_back(std::make_unique<TokenizerStage>());
            m_stages.push_back(std::make_unique<SegmentGraphStage>());
            m_stages.push_back(std::make_unique<LanguageModelStage>());
            m_stages.push_back(std::make_unique<ContextAnalyzerStage>());
            m_stages.push_back(std::make_unique<CandidateRankerStage>());
            m_stages.push_back(std::make_unique<PluginHooksStage>());
            
            Logger::Info("InputPipeline initialized with {} stages", m_stages.size());
            return true;
        }

        void InputPipeline::Uninitialize()
        {
            m_stages.clear();
        }

        HRESULT InputPipeline::Process(WPARAM wParam, LPARAM lParam, PipelineContext& context)
        {
            context.wParam = wParam;
            context.lParam = lParam;
            context.handled = false;
            context.cancelled = false;
            
            auto& latencyOptimizer = Perception::LatencyOptimizer::Instance();
            auto& featureManager = Adaptive::FeatureSwitchManager::Instance();
            auto& candidateCache = Cache::CandidateCache::Instance();

            latencyOptimizer.StartPhase(Perception::LatencyPhase::KEY_TO_LOGIC);

            if (!context.inputText.empty())
            {
                std::vector<CandidateItem> cachedCandidates;
                if (featureManager.IsFeatureEnabled(Adaptive::FeatureSwitch::AI_RERANKER) &&
                    candidateCache.Get(context.inputText, cachedCandidates))
                {
                    context.candidates = cachedCandidates;
                    latencyOptimizer.EndPhase(Perception::LatencyPhase::KEY_TO_LOGIC);
                    latencyOptimizer.StartPhase(Perception::LatencyPhase::LOGIC_TO_CANDIDATE);
                    latencyOptimizer.EndPhase(Perception::LatencyPhase::LOGIC_TO_CANDIDATE);
                    latencyOptimizer.StartPhase(Perception::LatencyPhase::CANDIDATE_TO_UI);
                    latencyOptimizer.EndPhase(Perception::LatencyPhase::CANDIDATE_TO_UI);
                    return S_OK;
                }
            }

            latencyOptimizer.EndPhase(Perception::LatencyPhase::KEY_TO_LOGIC);
            latencyOptimizer.StartPhase(Perception::LatencyPhase::LOGIC_TO_CANDIDATE);

            for (const auto& stage : m_stages)
            {
                if (context.cancelled)
                    break;

                if (stage->GetName() == L"ContextAnalyzer" && 
                    !featureManager.IsFeatureEnabled(Adaptive::FeatureSwitch::CONTEXT_ANALYZER))
                {
                    continue;
                }

                if (stage->GetName() == L"CandidateRanker" && 
                    !featureManager.IsFeatureEnabled(Adaptive::FeatureSwitch::AI_RERANKER))
                {
                    continue;
                }

                if (stage->GetName() == L"PluginHooks" && 
                    !featureManager.IsFeatureEnabled(Adaptive::FeatureSwitch::PLUGINS))
                {
                    continue;
                }

                HRESULT hr = stage->Process(context);
                if (FAILED(hr))
                {
                    Logger::Error("Pipeline stage {} failed: {}", stage->GetName(), hr);
                    latencyOptimizer.EndPhase(Perception::LatencyPhase::LOGIC_TO_CANDIDATE);
                    latencyOptimizer.StartPhase(Perception::LatencyPhase::CANDIDATE_TO_UI);
                    latencyOptimizer.EndPhase(Perception::LatencyPhase::CANDIDATE_TO_UI);
                    return hr;
                }
            }

            if (!context.inputText.empty() && !context.candidates.empty())
            {
                candidateCache.Set(context.inputText, context.candidates);
            }

            latencyOptimizer.EndPhase(Perception::LatencyPhase::LOGIC_TO_CANDIDATE);
            latencyOptimizer.StartPhase(Perception::LatencyPhase::CANDIDATE_TO_UI);
            latencyOptimizer.EndPhase(Perception::LatencyPhase::CANDIDATE_TO_UI);

            return S_OK;
        }

        void InputPipeline::AddStage(std::unique_ptr<IPipelineStage> stage)
        {
            m_stages.push_back(std::move(stage));
        }

        void InputPipeline::RemoveStage(const std::wstring& stageName)
        {
            m_stages.erase(
                std::remove_if(m_stages.begin(), m_stages.end(),
                    [&stageName](const std::unique_ptr<IPipelineStage>& stage) {
                        return stage->GetName() == stageName;
                    }),
                m_stages.end());
        }

        void InputPipeline::ClearStages()
        {
            m_stages.clear();
        }
    }
}