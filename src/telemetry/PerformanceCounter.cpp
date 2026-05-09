#include "telemetry/PerformanceCounter.h"
#include "utils/logger.h"
#include <fstream>

namespace ShuTongWen
{
    namespace Telemetry
    {
        PerformanceCounter::PerformanceCounter()
        {}

        PerformanceCounter& PerformanceCounter::Instance()
        {
            static PerformanceCounter instance;
            return instance;
        }

        void PerformanceCounter::IncrementCounter(const std::wstring& name)
        {
            m_counters[name]++;
        }

        void PerformanceCounter::DecrementCounter(const std::wstring& name)
        {
            auto it = m_counters.find(name);
            if (it != m_counters.end() && it->second > 0)
            {
                it->second--;
            }
        }

        uint64_t PerformanceCounter::GetCounterValue(const std::wstring& name) const
        {
            auto it = m_counters.find(name);
            return it != m_counters.end() ? it->second : 0;
        }

        void PerformanceCounter::RecordOperation(const std::wstring& name, bool success, std::chrono::nanoseconds latency)
        {
            PerformanceStats& stats = m_stats[name];
            stats.totalCount++;
            
            if (success)
            {
                stats.successCount++;
            }
            else
            {
                stats.failureCount++;
            }

            stats.totalLatency += latency;
            stats.maxLatency = std::max(stats.maxLatency, latency);
            stats.minLatency = std::min(stats.minLatency, latency);
            stats.lastUpdate = std::chrono::system_clock::now();

            Logger::Debug("Performance: {} - Success={}, Latency={}ms", 
                name, success, std::chrono::duration<double, std::milli>(latency).count());
        }

        PerformanceStats PerformanceCounter::GetStats(const std::wstring& name) const
        {
            auto it = m_stats.find(name);
            if (it != m_stats.end())
            {
                return it->second;
            }
            return PerformanceStats();
        }

        void PerformanceCounter::ResetCounter(const std::wstring& name)
        {
            m_counters.erase(name);
            m_stats.erase(name);
        }

        void PerformanceCounter::ResetAllCounters()
        {
            m_counters.clear();
            m_stats.clear();
        }

        void PerformanceCounter::ExportStats(const std::wstring& filePath)
        {
            std::wofstream file(filePath);
            if (!file.is_open())
                return;

            file << L"Name,TotalCount,SuccessCount,FailureCount,AvgLatency(ns),MaxLatency(ns),MinLatency(ns)\n";
            
            for (const auto& pair : m_stats)
            {
                const PerformanceStats& stats = pair.second;
                file << pair.first << L","
                     << stats.totalCount << L","
                     << stats.successCount << L","
                     << stats.failureCount << L","
                     << stats.GetAverageLatency() << L","
                     << stats.maxLatency.count() << L","
                     << stats.minLatency.count() << L"\n";
            }

            file.close();
        }
    }
}