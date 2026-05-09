#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <functional>

namespace ShuTongWen
{
    namespace Tracing
    {
        struct TraceEvent
        {
            std::wstring name;
            std::wstring category;
            std::chrono::nanoseconds timestamp;
            std::chrono::nanoseconds duration;
            std::wstring phase;
            std::unordered_map<std::wstring, std::wstring> args;

            TraceEvent() : duration(0) {}
            TraceEvent(const std::wstring& n, const std::wstring& cat, std::chrono::nanoseconds ts)
                : name(n), category(cat), timestamp(ts), duration(0) {}
        };

        struct TraceSpan
        {
            std::wstring name;
            std::chrono::high_resolution_clock::time_point start;
            std::chrono::high_resolution_clock::time_point end;
            std::vector<TraceEvent> events;
            std::shared_ptr<TraceSpan> parent;
            std::vector<std::shared_ptr<TraceSpan>> children;
            std::unordered_map<std::wstring, std::wstring> tags;

            TraceSpan(const std::wstring& n) : name(n) {}

            std::chrono::nanoseconds GetDuration() const 
            { 
                return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start); 
            }
        };

        struct TraceSession
        {
            std::wstring id;
            std::chrono::system_clock::time_point startTime;
            std::shared_ptr<TraceSpan> rootSpan;
            std::vector<std::shared_ptr<TraceSpan>> allSpans;
        };

        class InputTracer
        {
        public:
            ~InputTracer() = default;

            static InputTracer& Instance();

            void StartSession(const std::wstring& sessionId);
            void EndSession();

            std::shared_ptr<TraceSpan> StartSpan(const std::wstring& name);
            void EndSpan(std::shared_ptr<TraceSpan> span);

            void AddEvent(const std::wstring& name, const std::wstring& category = L"");
            void AddEventWithArgs(const std::wstring& name, const std::wstring& category,
                                 const std::unordered_map<std::wstring, std::wstring>& args);

            void SetSpanTag(std::shared_ptr<TraceSpan> span, const std::wstring& key, const std::wstring& value);

            void Enable(bool enabled);
            bool IsEnabled() const { return m_enabled; }

            void ExportToFile(const std::wstring& filePath);
            void ExportToJson(std::wstring& jsonOutput);

            void Clear();

            std::vector<TraceSession> GetRecentSessions(size_t limit = 10) const;

        private:
            InputTracer();

            bool m_enabled;
            std::unique_ptr<TraceSession> m_currentSession;
            std::vector<TraceSession> m_sessions;
            const size_t m_maxSessions;
        };

        class TraceScope
        {
        public:
            TraceScope(const std::wstring& name)
                : m_name(name), m_span(nullptr)
            {
                if (InputTracer::Instance().IsEnabled())
                {
                    m_span = InputTracer::Instance().StartSpan(name);
                }
            }

            ~TraceScope()
            {
                if (m_span)
                {
                    InputTracer::Instance().EndSpan(m_span);
                }
            }

            void SetTag(const std::wstring& key, const std::wstring& value)
            {
                if (m_span)
                {
                    InputTracer::Instance().SetSpanTag(m_span, key, value);
                }
            }

        private:
            std::wstring m_name;
            std::shared_ptr<TraceSpan> m_span;
        };
    }
}