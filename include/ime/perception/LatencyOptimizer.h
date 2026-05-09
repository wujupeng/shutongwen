#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <functional>

namespace ShuTongWen
{
    namespace Perception
    {
        enum class LatencyPhase
        {
            KEY_TO_LOGIC,
            LOGIC_TO_CANDIDATE,
            CANDIDATE_TO_UI
        };

        struct LatencyBudget
        {
            std::chrono::microseconds keyToLogic;
            std::chrono::microseconds logicToCandidate;
            std::chrono::microseconds candidateToUI;

            LatencyBudget() 
                : keyToLogic(2000),    // < 2ms
                  logicToCandidate(3000),  // < 3ms
                  candidateToUI(5000) {}   // < 5ms
        };

        struct LatencyMeasurement
        {
            LatencyPhase phase;
            std::chrono::nanoseconds duration;
            std::chrono::system_clock::time_point timestamp;
            std::wstring context;

            LatencyMeasurement() : duration(0) {}
        };

        struct PerceivedLatencyReport
        {
            std::chrono::nanoseconds totalLatency;
            std::chrono::nanoseconds keyToLogic;
            std::chrono::nanoseconds logicToCandidate;
            std::chrono::nanoseconds candidateToUI;
            bool withinBudget;
            std::vector<LatencyMeasurement> measurements;
        };

        class LatencyOptimizer
        {
        public:
            ~LatencyOptimizer() = default;

            static LatencyOptimizer& Instance();

            bool Initialize();
            void Uninitialize();

            void StartPhase(LatencyPhase phase);
            void EndPhase(LatencyPhase phase);

            void SetBudget(const LatencyBudget& budget);
            LatencyBudget GetBudget() const { return m_budget; }

            PerceivedLatencyReport GenerateReport();
            void Reset();

            bool IsWithinBudget(LatencyPhase phase, std::chrono::nanoseconds duration) const;

            std::function<void(LatencyPhase, std::chrono::nanoseconds)> onLatencyWarning;
            std::function<void(const PerceivedLatencyReport&)> onReportGenerated;

        private:
            LatencyOptimizer();

            void CheckBudgetViolation(LatencyPhase phase, std::chrono::nanoseconds duration);

            LatencyBudget m_budget;
            
            std::unordered_map<LatencyPhase, std::chrono::high_resolution_clock::time_point> m_phaseStart;
            std::vector<LatencyMeasurement> m_measurements;
            
            const size_t m_maxMeasurements;
            std::chrono::nanoseconds m_totalKeyToLogic;
            std::chrono::nanoseconds m_totalLogicToCandidate;
            std::chrono::nanoseconds m_totalCandidateToUI;
            size_t m_measurementCount;
        };

        class InputPredictor
        {
        public:
            ~InputPredictor() = default;

            static InputPredictor& Instance();

            bool Initialize();
            void Uninitialize();

            std::vector<std::wstring> PredictNextInputs(const std::wstring& currentInput);
            void UpdatePredictionModel(const std::vector<std::wstring>& recentInputs);
            
            void PrecomputeCandidates(const std::wstring& prefix);
            bool HasCachedCandidates(const std::wstring& prefix);
            std::vector<CandidateItem> GetCachedCandidates(const std::wstring& prefix);

        private:
            InputPredictor();

            std::unordered_map<std::wstring, std::vector<std::wstring>> m_predictions;
            std::unordered_map<std::wstring, std::vector<CandidateItem>> m_precomputed;
        };
    }
}