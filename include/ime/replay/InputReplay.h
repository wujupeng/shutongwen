#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <chrono>
#include <memory>

namespace ShuTongWen
{
    namespace Replay
    {
        struct InputEvent
        {
            enum class Type
            {
                KeyDown,
                KeyUp,
                Char,
                CompositionStart,
                CompositionUpdate,
                CompositionEnd,
                CandidateSelect,
                FocusIn,
                FocusOut,
                CaretMove,
                Delay
            };

            Type type;
            WPARAM wParam;
            LPARAM lParam;
            std::wstring text;
            std::chrono::nanoseconds timestamp;
            std::chrono::nanoseconds delay;

            InputEvent() : type(Type::KeyDown), wParam(0), lParam(0), delay(0) {}
        };

        struct ReplayRecord
        {
            std::wstring id;
            std::wstring appName;
            std::wstring className;
            std::chrono::system_clock::time_point startTime;
            std::vector<InputEvent> events;
            std::vector<std::wstring> expectedCandidates;
            std::vector<std::chrono::nanoseconds> expectedLatencies;
        };

        struct ReplayResult
        {
            bool success;
            std::wstring recordId;
            std::vector<std::wstring> actualCandidates;
            std::vector<std::chrono::nanoseconds> actualLatencies;
            std::vector<std::wstring> mismatches;
            double latencyDeviation;
            double candidateAccuracy;
        };

        class InputRecorder
        {
        public:
            ~InputRecorder() = default;

            static InputRecorder& Instance();

            bool StartRecording(const std::wstring& appName);
            bool StopRecording();
            bool IsRecording() const { return m_recording; }

            void RecordEvent(const InputEvent& event);
            void RecordKeyDown(WPARAM wParam, LPARAM lParam);
            void RecordKeyUp(WPARAM wParam, LPARAM lParam);
            void RecordChar(wchar_t ch);
            void RecordComposition(const std::wstring& text);
            void RecordCandidateSelect(int index);
            void RecordDelay(std::chrono::nanoseconds delay);

            bool SaveRecording(const std::wstring& filePath);
            bool LoadRecording(const std::wstring& filePath, ReplayRecord& record);

            const ReplayRecord& GetCurrentRecord() const { return m_currentRecord; }

        private:
            InputRecorder();

            bool m_recording;
            ReplayRecord m_currentRecord;
            std::chrono::high_resolution_clock::time_point m_startTime;
        };

        class InputPlayer
        {
        public:
            ~InputPlayer() = default;

            static InputPlayer& Instance();

            bool PlayRecording(const ReplayRecord& record, ReplayResult& result);
            bool PlayRecordingFromFile(const std::wstring& filePath, ReplayResult& result);

            void SetPlaybackSpeed(double speed);
            double GetPlaybackSpeed() const { return m_playbackSpeed; }

            void SetCompareMode(bool enabled);
            bool GetCompareMode() const { return m_compareMode; }

            void SetTargetWindow(HWND hwnd);
            HWND GetTargetWindow() const { return m_targetWindow; }

        private:
            InputPlayer();

            double m_playbackSpeed;
            bool m_compareMode;
            HWND m_targetWindow;
        };

        class ReplayAnalyzer
        {
        public:
            ~ReplayAnalyzer() = default;

            static ReplayAnalyzer& Instance();

            ReplayResult CompareRecords(const ReplayRecord& expected, const ReplayRecord& actual);

            void AnalyzeLatency(const ReplayResult& result, std::wstring& report);
            void AnalyzeCandidateAccuracy(const ReplayResult& result, std::wstring& report);

            bool GenerateReport(const ReplayResult& result, const std::wstring& filePath);

        private:
            ReplayAnalyzer();
        };
    }
}