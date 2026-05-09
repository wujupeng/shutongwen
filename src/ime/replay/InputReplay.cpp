#include "ime/replay/InputReplay.h"
#include "utils/logger.h"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace ShuTongWen
{
    namespace Replay
    {
        InputRecorder::InputRecorder()
            : m_recording(false)
        {}

        InputRecorder& InputRecorder::Instance()
        {
            static InputRecorder instance;
            return instance;
        }

        bool InputRecorder::StartRecording(const std::wstring& appName)
        {
            if (m_recording)
            {
                Logger::Warning("Already recording");
                return false;
            }

            m_currentRecord.appName = appName;
            
            HWND hwnd = GetForegroundWindow();
            wchar_t className[256];
            GetClassNameW(hwnd, className, sizeof(className) / sizeof(wchar_t));
            m_currentRecord.className = className;

            m_currentRecord.startTime = std::chrono::system_clock::now();
            m_currentRecord.id = L"rec_" + std::to_wstring(
                std::chrono::system_clock::to_time_t(m_currentRecord.startTime));
            
            m_currentRecord.events.clear();
            m_startTime = std::chrono::high_resolution_clock::now();

            m_recording = true;
            Logger::Info("Started recording: {}", m_currentRecord.id);
            return true;
        }

        bool InputRecorder::StopRecording()
        {
            if (!m_recording)
                return false;

            m_recording = false;
            Logger::Info("Stopped recording: {} ({} events)", 
                m_currentRecord.id, m_currentRecord.events.size());
            return true;
        }

        void InputRecorder::RecordEvent(const InputEvent& event)
        {
            if (!m_recording)
                return;

            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = now - m_startTime;
            
            InputEvent copy = event;
            copy.timestamp = elapsed;
            m_currentRecord.events.push_back(copy);
        }

        void InputRecorder::RecordKeyDown(WPARAM wParam, LPARAM lParam)
        {
            InputEvent event;
            event.type = InputEvent::Type::KeyDown;
            event.wParam = wParam;
            event.lParam = lParam;
            RecordEvent(event);
        }

        void InputRecorder::RecordKeyUp(WPARAM wParam, LPARAM lParam)
        {
            InputEvent event;
            event.type = InputEvent::Type::KeyUp;
            event.wParam = wParam;
            event.lParam = lParam;
            RecordEvent(event);
        }

        void InputRecorder::RecordChar(wchar_t ch)
        {
            InputEvent event;
            event.type = InputEvent::Type::Char;
            event.text = std::wstring(1, ch);
            RecordEvent(event);
        }

        void InputRecorder::RecordComposition(const std::wstring& text)
        {
            InputEvent event;
            event.type = InputEvent::Type::CompositionUpdate;
            event.text = text;
            RecordEvent(event);
        }

        void InputRecorder::RecordCandidateSelect(int index)
        {
            InputEvent event;
            event.type = InputEvent::Type::CandidateSelect;
            event.wParam = index;
            RecordEvent(event);
        }

        void InputRecorder::RecordDelay(std::chrono::nanoseconds delay)
        {
            InputEvent event;
            event.type = InputEvent::Type::Delay;
            event.delay = delay;
            RecordEvent(event);
        }

        bool InputRecorder::SaveRecording(const std::wstring& filePath)
        {
            json root;
            root["id"] = StringUtils::UTF16ToUTF8(m_currentRecord.id);
            root["appName"] = StringUtils::UTF16ToUTF8(m_currentRecord.appName);
            root["className"] = StringUtils::UTF16ToUTF8(m_currentRecord.className);
            root["startTime"] = std::chrono::system_clock::to_time_t(m_currentRecord.startTime);

            json events = json::array();
            for (const auto& event : m_currentRecord.events)
            {
                json e;
                e["type"] = static_cast<int>(event.type);
                e["wParam"] = event.wParam;
                e["lParam"] = event.lParam;
                e["text"] = StringUtils::UTF16ToUTF8(event.text);
                e["timestamp"] = event.timestamp.count();
                e["delay"] = event.delay.count();
                events.push_back(e);
            }
            root["events"] = events;

            json candidates = json::array();
            for (const auto& c : m_currentRecord.expectedCandidates)
            {
                candidates.push_back(StringUtils::UTF16ToUTF8(c));
            }
            root["expectedCandidates"] = candidates;

            std::wofstream file(filePath);
            if (!file.is_open())
                return false;

            file << StringUtils::UTF8ToUTF16(root.dump(2));
            file.close();

            Logger::Info("Recording saved to: {}", filePath);
            return true;
        }

        bool InputRecorder::LoadRecording(const std::wstring& filePath, ReplayRecord& record)
        {
            std::wifstream file(filePath);
            if (!file.is_open())
                return false;

            std::wstring content((std::istreambuf_iterator<wchar_t>(file)),
                                std::istreambuf_iterator<wchar_t>());

            json root = json::parse(StringUtils::UTF16ToUTF8(content));

            record.id = StringUtils::UTF8ToUTF16(root["id"]);
            record.appName = StringUtils::UTF8ToUTF16(root["appName"]);
            record.className = StringUtils::UTF8ToUTF16(root["className"]);
            record.startTime = std::chrono::system_clock::from_time_t(root["startTime"]);

            record.events.clear();
            for (const auto& e : root["events"])
            {
                InputEvent event;
                event.type = static_cast<InputEvent::Type>(e["type"]);
                event.wParam = e["wParam"];
                event.lParam = e["lParam"];
                event.text = StringUtils::UTF8ToUTF16(e["text"]);
                event.timestamp = std::chrono::nanoseconds(e["timestamp"]);
                event.delay = std::chrono::nanoseconds(e["delay"]);
                record.events.push_back(event);
            }

            record.expectedCandidates.clear();
            for (const auto& c : root["expectedCandidates"])
            {
                record.expectedCandidates.push_back(StringUtils::UTF8ToUTF16(c));
            }

            Logger::Info("Recording loaded: {} ({} events)", record.id, record.events.size());
            return true;
        }

        InputPlayer::InputPlayer()
            : m_playbackSpeed(1.0), m_compareMode(true), m_targetWindow(nullptr)
        {}

        InputPlayer& InputPlayer::Instance()
        {
            static InputPlayer instance;
            return instance;
        }

        bool InputPlayer::PlayRecording(const ReplayRecord& record, ReplayResult& result)
        {
            result.success = true;
            result.recordId = record.id;
            result.actualCandidates.clear();
            result.actualLatencies.clear();
            result.mismatches.clear();
            result.latencyDeviation = 0.0;
            result.candidateAccuracy = 0.0;

            HWND target = m_targetWindow ? m_targetWindow : GetForegroundWindow();
            if (!target)
                return false;

            SetForegroundWindow(target);
            Sleep(200);

            auto startTime = std::chrono::high_resolution_clock::now();
            auto lastEventTime = startTime;

            for (const auto& event : record.events)
            {
                if (event.type == InputEvent::Type::Delay)
                {
                    Sleep(static_cast<DWORD>(event.delay.count() / 1000000));
                    continue;
                }

                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed = now - lastEventTime;
                auto targetDelay = event.delay / m_playbackSpeed;

                if (elapsed < targetDelay)
                {
                    auto sleepTime = targetDelay - elapsed;
                    Sleep(static_cast<DWORD>(sleepTime.count() / 1000000));
                }

                switch (event.type)
                {
                case InputEvent::Type::KeyDown:
                {
                    INPUT input = {0};
                    input.type = INPUT_KEYBOARD;
                    input.ki.wVk = event.wParam;
                    input.ki.dwFlags = 0;
                    SendInput(1, &input, sizeof(INPUT));
                    break;
                }
                case InputEvent::Type::KeyUp:
                {
                    INPUT input = {0};
                    input.type = INPUT_KEYBOARD;
                    input.ki.wVk = event.wParam;
                    input.ki.dwFlags = KEYEVENTF_KEYUP;
                    SendInput(1, &input, sizeof(INPUT));
                    break;
                }
                case InputEvent::Type::Char:
                {
                    for (wchar_t ch : event.text)
                    {
                        INPUT input = {0};
                        input.type = INPUT_KEYBOARD;
                        input.ki.wScan = ch;
                        input.ki.dwFlags = KEYEVENTF_UNICODE;
                        SendInput(1, &input, sizeof(INPUT));

                        input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
                        SendInput(1, &input, sizeof(INPUT));
                    }
                    break;
                }
                }

                lastEventTime = std::chrono::high_resolution_clock::now();
            }

            auto totalTime = std::chrono::high_resolution_clock::now() - startTime;
            result.actualLatencies.push_back(totalTime);

            if (m_compareMode)
            {
                size_t matches = 0;
                for (size_t i = 0; i < std::min(result.actualCandidates.size(), 
                                                 record.expectedCandidates.size()); ++i)
                {
                    if (result.actualCandidates[i] == record.expectedCandidates[i])
                        matches++;
                }
                
                if (!record.expectedCandidates.empty())
                {
                    result.candidateAccuracy = static_cast<double>(matches) / record.expectedCandidates.size();
                }
            }

            return true;
        }

        bool InputPlayer::PlayRecordingFromFile(const std::wstring& filePath, ReplayResult& result)
        {
            ReplayRecord record;
            if (!InputRecorder::Instance().LoadRecording(filePath, record))
                return false;

            return PlayRecording(record, result);
        }

        void InputPlayer::SetPlaybackSpeed(double speed)
        {
            m_playbackSpeed = std::max(0.1, std::min(10.0, speed));
        }

        void InputPlayer::SetCompareMode(bool enabled)
        {
            m_compareMode = enabled;
        }

        void InputPlayer::SetTargetWindow(HWND hwnd)
        {
            m_targetWindow = hwnd;
        }

        ReplayAnalyzer::ReplayAnalyzer()
        {}

        ReplayAnalyzer& ReplayAnalyzer::Instance()
        {
            static ReplayAnalyzer instance;
            return instance;
        }

        ReplayResult ReplayAnalyzer::CompareRecords(const ReplayRecord& expected, const ReplayRecord& actual)
        {
            ReplayResult result;
            result.success = true;
            result.recordId = expected.id;
            result.mismatches.clear();

            if (expected.events.size() != actual.events.size())
            {
                result.success = false;
                result.mismatches.push_back(L"Event count mismatch");
            }

            for (size_t i = 0; i < std::min(expected.events.size(), actual.events.size()); ++i)
            {
                const auto& e1 = expected.events[i];
                const auto& e2 = actual.events[i];

                if (e1.type != e2.type)
                {
                    result.mismatches.push_back(L"Event type mismatch at index " + std::to_wstring(i));
                }
                else if (e1.wParam != e2.wParam)
                {
                    result.mismatches.push_back(L"wParam mismatch at index " + std::to_wstring(i));
                }
                else if (e1.text != e2.text)
                {
                    result.mismatches.push_back(L"Text mismatch at index " + std::to_wstring(i));
                }
            }

            return result;
        }

        void ReplayAnalyzer::AnalyzeLatency(const ReplayResult& result, std::wstring& report)
        {
            report.clear();
            
            if (result.actualLatencies.empty())
            {
                report = L"No latency data";
                return;
            }

            std::chrono::nanoseconds total(0);
            std::chrono::nanoseconds maxLatency(0);
            std::chrono::nanoseconds minLatency = result.actualLatencies[0];

            for (const auto& latency : result.actualLatencies)
            {
                total += latency;
                maxLatency = std::max(maxLatency, latency);
                minLatency = std::min(minLatency, latency);
            }

            double avgLatency = static_cast<double>(total.count()) / result.actualLatencies.size();

            report = L"Latency Analysis:\n";
            report += L"  Count: " + std::to_wstring(result.actualLatencies.size()) + L"\n";
            report += L"  Average: " + std::to_wstring(avgLatency / 1000000.0) + L"ms\n";
            report += L"  Max: " + std::to_wstring(maxLatency.count() / 1000000.0) + L"ms\n";
            report += L"  Min: " + std::to_wstring(minLatency.count() / 1000000.0) + L"ms\n";
            report += L"  Deviation: " + std::to_wstring(result.latencyDeviation) + L"%\n";
        }

        void ReplayAnalyzer::AnalyzeCandidateAccuracy(const ReplayResult& result, std::wstring& report)
        {
            report.clear();
            
            report = L"Candidate Analysis:\n";
            report += L"  Accuracy: " + std::to_wstring(result.candidateAccuracy * 100) + L"%\n";
            report += L"  Mismatches: " + std::to_wstring(result.mismatches.size()) + L"\n";
        }

        bool ReplayAnalyzer::GenerateReport(const ReplayResult& result, const std::wstring& filePath)
        {
            std::wofstream file(filePath);
            if (!file.is_open())
                return false;

            file << L"=== Input Replay Report ===\n";
            file << L"Record ID: " << result.recordId << L"\n";
            file << L"Success: " << (result.success ? L"Yes" : L"No") << L"\n";
            file << L"Candidate Accuracy: " << result.candidateAccuracy * 100 << L"%\n";
            file << L"Latency Deviation: " << result.latencyDeviation << L"%\n";

            file << L"\n--- Mismatches ---\n";
            for (const auto& mismatch : result.mismatches)
            {
                file << L"  - " << mismatch << L"\n";
            }

            file.close();
            return true;
        }
    }
}