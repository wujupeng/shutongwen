#include "ime/perception/LatencyOptimizer.h"
#include "utils/logger.h"
#include "ime/cache/InputCache.h"

namespace ShuTongWen
{
    namespace Perception
    {
        LatencyOptimizer::LatencyOptimizer()
            : m_maxMeasurements(1000),
              m_totalKeyToLogic(0),
              m_totalLogicToCandidate(0),
              m_totalCandidateToUI(0),
              m_measurementCount(0)
        {}

        LatencyOptimizer& LatencyOptimizer::Instance()
        {
            static LatencyOptimizer instance;
            return instance;
        }

        bool LatencyOptimizer::Initialize()
        {
            Logger::Info("Initializing LatencyOptimizer...");
            m_budget = LatencyBudget();
            return true;
        }

        void LatencyOptimizer::Uninitialize()
        {
            m_measurements.clear();
            m_phaseStart.clear();
        }

        void LatencyOptimizer::StartPhase(LatencyPhase phase)
        {
            m_phaseStart[phase] = std::chrono::high_resolution_clock::now();
        }

        void LatencyOptimizer::EndPhase(LatencyPhase phase)
        {
            auto it = m_phaseStart.find(phase);
            if (it == m_phaseStart.end())
                return;

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = end - it->second;

            m_phaseStart.erase(it);

            LatencyMeasurement measurement;
            measurement.phase = phase;
            measurement.duration = duration;
            measurement.timestamp = std::chrono::system_clock::now();

            m_measurements.push_back(measurement);
            if (m_measurements.size() > m_maxMeasurements)
            {
                m_measurements.erase(m_measurements.begin());
            }

            switch (phase)
            {
            case LatencyPhase::KEY_TO_LOGIC:
                m_totalKeyToLogic += duration;
                break;
            case LatencyPhase::LOGIC_TO_CANDIDATE:
                m_totalLogicToCandidate += duration;
                break;
            case LatencyPhase::CANDIDATE_TO_UI:
                m_totalCandidateToUI += duration;
                break;
            }

            m_measurementCount++;

            CheckBudgetViolation(phase, duration);
        }

        void LatencyOptimizer::SetBudget(const LatencyBudget& budget)
        {
            m_budget = budget;
        }

        PerceivedLatencyReport LatencyOptimizer::GenerateReport()
        {
            PerceivedLatencyReport report;
            report.totalLatency = m_totalKeyToLogic + m_totalLogicToCandidate + m_totalCandidateToUI;
            report.keyToLogic = m_totalKeyToLogic;
            report.logicToCandidate = m_totalLogicToCandidate;
            report.candidateToUI = m_totalCandidateToUI;
            report.measurements = m_measurements;

            bool withinBudget = true;
            
            if (m_measurementCount > 0)
            {
                auto avgKeyToLogic = m_totalKeyToLogic / m_measurementCount;
                auto avgLogicToCandidate = m_totalLogicToCandidate / m_measurementCount;
                auto avgCandidateToUI = m_totalCandidateToUI / m_measurementCount;

                withinBudget = avgKeyToLogic < m_budget.keyToLogic &&
                              avgLogicToCandidate < m_budget.logicToCandidate &&
                              avgCandidateToUI < m_budget.candidateToUI;
            }

            report.withinBudget = withinBudget;

            if (onReportGenerated)
            {
                onReportGenerated(report);
            }

            return report;
        }

        void LatencyOptimizer::Reset()
        {
            m_measurements.clear();
            m_totalKeyToLogic = 0;
            m_totalLogicToCandidate = 0;
            m_totalCandidateToUI = 0;
            m_measurementCount = 0;
        }

        bool LatencyOptimizer::IsWithinBudget(LatencyPhase phase, std::chrono::nanoseconds duration) const
        {
            switch (phase)
            {
            case LatencyPhase::KEY_TO_LOGIC:
                return duration < m_budget.keyToLogic;
            case LatencyPhase::LOGIC_TO_CANDIDATE:
                return duration < m_budget.logicToCandidate;
            case LatencyPhase::CANDIDATE_TO_UI:
                return duration < m_budget.candidateToUI;
            }
            return true;
        }

        void LatencyOptimizer::CheckBudgetViolation(LatencyPhase phase, std::chrono::nanoseconds duration)
        {
            if (!IsWithinBudget(phase, duration))
            {
                Logger::Warning("Latency budget violation for phase {}: {}ms", 
                    static_cast<int>(phase),
                    std::chrono::duration<double, std::milli>(duration).count());

                if (onLatencyWarning)
                {
                    onLatencyWarning(phase, duration);
                }
            }
        }

        InputPredictor::InputPredictor()
        {}

        InputPredictor& InputPredictor::Instance()
        {
            static InputPredictor instance;
            return instance;
        }

        bool InputPredictor::Initialize()
        {
            Logger::Info("Initializing InputPredictor...");
            return true;
        }

        void InputPredictor::Uninitialize()
        {
            m_predictions.clear();
            m_precomputed.clear();
        }

        std::vector<std::wstring> InputPredictor::PredictNextInputs(const std::wstring& currentInput)
        {
            std::vector<std::wstring> predictions;

            if (currentInput.empty())
                return predictions;

            auto it = m_predictions.find(currentInput);
            if (it != m_predictions.end())
            {
                predictions = it->second;
            }
            else
            {
                std::vector<std::wstring> commonPrefixes = {
                    currentInput + L"a",
                    currentInput + L"i",
                    currentInput + L"e",
                    currentInput + L"o",
                    currentInput + L"u"
                };
                predictions = commonPrefixes;
            }

            return predictions;
        }

        void InputPredictor::UpdatePredictionModel(const std::vector<std::wstring>& recentInputs)
        {
            for (size_t i = 0; i < recentInputs.size() - 1; ++i)
            {
                const std::wstring& current = recentInputs[i];
                const std::wstring& next = recentInputs[i + 1];

                auto it = m_predictions.find(current);
                if (it != m_predictions.end())
                {
                    auto& preds = it->second;
                    auto predIt = std::find(preds.begin(), preds.end(), next);
                    if (predIt == preds.end())
                    {
                        preds.insert(preds.begin(), next);
                        if (preds.size() > 5)
                        {
                            preds.pop_back();
                        }
                    }
                }
                else
                {
                    m_predictions[current] = {next};
                }
            }
        }

        void InputPredictor::PrecomputeCandidates(const std::wstring& prefix)
        {
            if (prefix.empty())
                return;

            auto& cache = Cache::CandidateCache::Instance();
            
            std::vector<CandidateItem> candidates;
            if (cache.Get(prefix, candidates))
            {
                m_precomputed[prefix] = candidates;
            }
        }

        bool InputPredictor::HasCachedCandidates(const std::wstring& prefix)
        {
            return m_precomputed.find(prefix) != m_precomputed.end();
        }

        std::vector<CandidateItem> InputPredictor::GetCachedCandidates(const std::wstring& prefix)
        {
            auto it = m_precomputed.find(prefix);
            if (it != m_precomputed.end())
            {
                return it->second;
            }
            return {};
        }
    }
}