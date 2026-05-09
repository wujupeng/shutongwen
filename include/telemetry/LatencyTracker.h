#pragma once

#include <windows.h>
#include <string>
#include <chrono>
#include <unordered_map>
#include <vector>

namespace ShuTongWen
{
    namespace Telemetry
    {
        struct LatencyRecord
        {
            std::wstring name;
            std::chrono::nanoseconds duration;
            std::chrono::system_clock::time_point timestamp;
            std::wstring context;

            LatencyRecord(const std::wstring& n, std::chrono::nanoseconds d, const std::wstring& c = L"")
                : name(n), duration(d), timestamp(std::chrono::system_clock::now()), context(c) {}
        };

        class LatencyTracker
        {
        public:
            ~LatencyTracker() = default;

            static LatencyTracker& Instance();

            void StartTrack(const std::wstring& name, const std::wstring& context = L"");
            void EndTrack(const std::wstring& name);
            
            std::chrono::nanoseconds GetLastLatency(const std::wstring& name) const;
            double GetAverageLatency(const std::wstring& name) const;
            std::chrono::nanoseconds GetMaxLatency(const std::wstring& name) const;
            std::chrono::nanoseconds GetMinLatency(const std::wstring& name) const;

            void ClearTracks();
            void ExportRecords(const std::wstring& filePath);

            std::vector<LatencyRecord> GetRecentRecords(size_t count = 100) const;

        private:
            LatencyTracker();
            
            struct TrackInfo
            {
                std::chrono::high_resolution_clock::time_point start;
                std::vector<std::chrono::nanoseconds> history;
            };

            std::unordered_map<std::wstring, TrackInfo> m_tracks;
            std::vector<LatencyRecord> m_recentRecords;
            const size_t m_maxHistorySize;
            const size_t m_maxRecentRecords;
        };
    }
}