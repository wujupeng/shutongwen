#pragma once

#include <windows.h>
#include <string>
#include <unordered_map>
#include <chrono>

namespace ShuTongWen
{
    namespace Telemetry
    {
        struct PerformanceStats
        {
            uint64_t totalCount;
            uint64_t successCount;
            uint64_t failureCount;
            std::chrono::nanoseconds totalLatency;
            std::chrono::nanoseconds maxLatency;
            std::chrono::nanoseconds minLatency;
            std::chrono::system_clock::time_point lastUpdate;

            PerformanceStats() 
                : totalCount(0), successCount(0), failureCount(0),
                  totalLatency(0), maxLatency(0), minLatency(std::chrono::nanoseconds::max()) {}

            double GetSuccessRate() const { return totalCount > 0 ? (double)successCount / totalCount : 0.0; }
            double GetAverageLatency() const { return totalCount > 0 ? (double)totalLatency.count() / totalCount : 0.0; }
        };

        class PerformanceCounter
        {
        public:
            ~PerformanceCounter() = default;

            static PerformanceCounter& Instance();

            void IncrementCounter(const std::wstring& name);
            void DecrementCounter(const std::wstring& name);
            uint64_t GetCounterValue(const std::wstring& name) const;

            void RecordOperation(const std::wstring& name, bool success, std::chrono::nanoseconds latency);
            PerformanceStats GetStats(const std::wstring& name) const;

            void ResetCounter(const std::wstring& name);
            void ResetAllCounters();

            void ExportStats(const std::wstring& filePath);

        private:
            PerformanceCounter();

            std::unordered_map<std::wstring, uint64_t> m_counters;
            std::unordered_map<std::wstring, PerformanceStats> m_stats;
        };
    }
}