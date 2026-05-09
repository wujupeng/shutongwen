#include "telemetry/LatencyTracker.h"
#include "utils/logger.h"
#include <fstream>
#include <algorithm>

namespace ShuTongWen
{
    namespace Telemetry
    {
        LatencyTracker::LatencyTracker()
            : m_maxHistorySize(100),
              m_maxRecentRecords(1000)
        {}

        LatencyTracker& LatencyTracker::Instance()
        {
            static LatencyTracker instance;
            return instance;
        }

        void LatencyTracker::StartTrack(const std::wstring& name, const std::wstring& context)
        {
            auto it = m_tracks.find(name);
            if (it != m_tracks.end())
            {
                Logger::Warning("Track {} already started", name);
                return;
            }

            TrackInfo info;
            info.start = std::chrono::high_resolution_clock::now();
            m_tracks[name] = info;
        }

        void LatencyTracker::EndTrack(const std::wstring& name)
        {
            auto it = m_tracks.find(name);
            if (it == m_tracks.end())
            {
                Logger::Warning("Track {} not found", name);
                return;
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = end - it->second.start;

            it->second.history.push_back(duration);
            if (it->second.history.size() > m_maxHistorySize)
            {
                it->second.history.erase(it->second.history.begin());
            }

            m_recentRecords.emplace_back(name, duration);
            if (m_recentRecords.size() > m_maxRecentRecords)
            {
                m_recentRecords.erase(m_recentRecords.begin());
            }

            m_tracks.erase(it);

            Logger::Debug("Latency track {} completed: {}ms", name, 
                std::chrono::duration<double, std::milli>(duration).count());
        }

        std::chrono::nanoseconds LatencyTracker::GetLastLatency(const std::wstring& name) const
        {
            auto it = m_tracks.find(name);
            if (it != m_tracks.end() && !it->second.history.empty())
            {
                return it->second.history.back();
            }
            return std::chrono::nanoseconds(0);
        }

        double LatencyTracker::GetAverageLatency(const std::wstring& name) const
        {
            auto it = m_tracks.find(name);
            if (it != m_tracks.end() && !it->second.history.empty())
            {
                std::chrono::nanoseconds total(0);
                for (const auto& d : it->second.history)
                {
                    total += d;
                }
                return static_cast<double>(total.count()) / it->second.history.size();
            }
            return 0.0;
        }

        std::chrono::nanoseconds LatencyTracker::GetMaxLatency(const std::wstring& name) const
        {
            auto it = m_tracks.find(name);
            if (it != m_tracks.end() && !it->second.history.empty())
            {
                return *std::max_element(it->second.history.begin(), it->second.history.end());
            }
            return std::chrono::nanoseconds(0);
        }

        std::chrono::nanoseconds LatencyTracker::GetMinLatency(const std::wstring& name) const
        {
            auto it = m_tracks.find(name);
            if (it != m_tracks.end() && !it->second.history.empty())
            {
                return *std::min_element(it->second.history.begin(), it->second.history.end());
            }
            return std::chrono::nanoseconds(0);
        }

        void LatencyTracker::ClearTracks()
        {
            m_tracks.clear();
            m_recentRecords.clear();
        }

        void LatencyTracker::ExportRecords(const std::wstring& filePath)
        {
            std::wofstream file(filePath);
            if (!file.is_open())
                return;

            file << L"Timestamp,Name,Duration(ns),Context\n";
            for (const auto& record : m_recentRecords)
            {
                file << record.timestamp.time_since_epoch().count() << L","
                     << record.name << L","
                     << record.duration.count() << L","
                     << record.context << L"\n";
            }

            file.close();
        }

        std::vector<LatencyRecord> LatencyTracker::GetRecentRecords(size_t count) const
        {
            std::vector<LatencyRecord> result;
            size_t takeCount = std::min(count, m_recentRecords.size());
            result.resize(takeCount);
            std::reverse_copy(m_recentRecords.begin(), m_recentRecords.begin() + takeCount, result.begin());
            return result;
        }
    }
}