#include "ime/tracing/InputTrace.h"
#include "utils/logger.h"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace ShuTongWen
{
    namespace Tracing
    {
        InputTracer::InputTracer()
            : m_enabled(true),
              m_maxSessions(100)
        {}

        InputTracer& InputTracer::Instance()
        {
            static InputTracer instance;
            return instance;
        }

        void InputTracer::StartSession(const std::wstring& sessionId)
        {
            m_currentSession = std::make_unique<TraceSession>();
            m_currentSession->id = sessionId;
            m_currentSession->startTime = std::chrono::system_clock::now();
            m_currentSession->rootSpan = std::make_shared<TraceSpan>(L"InputSession");
            m_currentSession->rootSpan->start = std::chrono::high_resolution_clock::now();
            m_currentSession->allSpans.push_back(m_currentSession->rootSpan);

            Logger::Debug("Trace session started: {}", sessionId);
        }

        void InputTracer::EndSession()
        {
            if (m_currentSession && m_currentSession->rootSpan)
            {
                m_currentSession->rootSpan->end = std::chrono::high_resolution_clock::now();

                m_sessions.push_back(*m_currentSession);
                if (m_sessions.size() > m_maxSessions)
                {
                    m_sessions.erase(m_sessions.begin());
                }

                Logger::Debug("Trace session ended: {}, duration={}ms", 
                    m_currentSession->id,
                    std::chrono::duration<double, std::milli>(
                        m_currentSession->rootSpan->GetDuration()).count());
            }

            m_currentSession.reset();
        }

        std::shared_ptr<TraceSpan> InputTracer::StartSpan(const std::wstring& name)
        {
            if (!m_enabled || !m_currentSession)
                return nullptr;

            auto span = std::make_shared<TraceSpan>(name);
            span->start = std::chrono::high_resolution_clock::now();
            span->parent = m_currentSession->rootSpan;
            
            m_currentSession->rootSpan->children.push_back(span);
            m_currentSession->allSpans.push_back(span);

            return span;
        }

        void InputTracer::EndSpan(std::shared_ptr<TraceSpan> span)
        {
            if (!span)
                return;

            span->end = std::chrono::high_resolution_clock::now();

            Logger::Debug("Span ended: {} - {}ms", 
                span->name,
                std::chrono::duration<double, std::milli>(span->GetDuration()).count());
        }

        void InputTracer::AddEvent(const std::wstring& name, const std::wstring& category)
        {
            if (!m_enabled || !m_currentSession)
                return;

            auto now = std::chrono::high_resolution_clock::now();
            auto ts = std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch());

            TraceEvent event(name, category, ts);
            if (m_currentSession->rootSpan)
            {
                m_currentSession->rootSpan->events.push_back(event);
            }
        }

        void InputTracer::AddEventWithArgs(const std::wstring& name, const std::wstring& category,
                                           const std::unordered_map<std::wstring, std::wstring>& args)
        {
            if (!m_enabled || !m_currentSession)
                return;

            auto now = std::chrono::high_resolution_clock::now();
            auto ts = std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch());

            TraceEvent event(name, category, ts);
            event.args = args;
            
            if (m_currentSession->rootSpan)
            {
                m_currentSession->rootSpan->events.push_back(event);
            }
        }

        void InputTracer::SetSpanTag(std::shared_ptr<TraceSpan> span, const std::wstring& key, const std::wstring& value)
        {
            if (span)
            {
                span->tags[key] = value;
            }
        }

        void InputTracer::Enable(bool enabled)
        {
            m_enabled = enabled;
            Logger::Info("Tracing {}abled", enabled ? "en" : "dis");
        }

        void InputTracer::ExportToFile(const std::wstring& filePath)
        {
            std::wstring jsonOutput;
            ExportToJson(jsonOutput);

            std::wofstream file(filePath);
            if (file.is_open())
            {
                file << jsonOutput;
                file.close();
                Logger::Info("Trace exported to: {}", filePath);
            }
        }

        void InputTracer::ExportToJson(std::wstring& jsonOutput)
        {
            json root = json::array();

            for (const auto& session : m_sessions)
            {
                json sessionJson;
                sessionJson["id"] = StringUtils::UTF16ToUTF8(session.id);
                sessionJson["startTime"] = session.startTime.time_since_epoch().count();

                json spans = json::array();
                for (const auto& span : session.allSpans)
                {
                    json spanJson;
                    spanJson["name"] = StringUtils::UTF16ToUTF8(span->name);
                    spanJson["duration"] = span->GetDuration().count();
                    
                    json tags = json::object();
                    for (const auto& pair : span->tags)
                    {
                        tags[StringUtils::UTF16ToUTF8(pair.first)] = StringUtils::UTF16ToUTF8(pair.second);
                    }
                    spanJson["tags"] = tags;

                    json events = json::array();
                    for (const auto& event : span->events)
                    {
                        json eventJson;
                        eventJson["name"] = StringUtils::UTF16ToUTF8(event.name);
                        eventJson["category"] = StringUtils::UTF16ToUTF8(event.category);
                        eventJson["timestamp"] = event.timestamp.count();
                        
                        json args = json::object();
                        for (const auto& arg : event.args)
                        {
                            args[StringUtils::UTF16ToUTF8(arg.first)] = StringUtils::UTF16ToUTF8(arg.second);
                        }
                        eventJson["args"] = args;

                        events.push_back(eventJson);
                    }
                    spanJson["events"] = events;

                    spans.push_back(spanJson);
                }
                sessionJson["spans"] = spans;

                root.push_back(sessionJson);
            }

            jsonOutput = StringUtils::UTF8ToUTF16(root.dump(2));
        }

        void InputTracer::Clear()
        {
            m_sessions.clear();
            m_currentSession.reset();
        }

        std::vector<TraceSession> InputTracer::GetRecentSessions(size_t limit) const
        {
            std::vector<TraceSession> result;
            size_t count = std::min(limit, m_sessions.size());
            result.resize(count);
            std::reverse_copy(m_sessions.begin(), m_sessions.begin() + count, result.begin());
            return result;
        }
    }
}