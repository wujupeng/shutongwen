#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <memory>
#include <chrono>

namespace ShuTongWen
{
    namespace Testing
    {
        enum class GoldenPathType
        {
            Chrome,
            VSCode,
            Office,
            Game,
            Edge,
            WeChat
        };

        enum class TestOutcome
        {
            Passed,
            Failed,
            Skipped,
            Flaky
        };

        struct GoldenStep
        {
            std::wstring name;
            std::wstring input;
            std::wstring expectedOutput;
            std::chrono::milliseconds maxLatency;
            bool optional;

            GoldenStep() : maxLatency(50), optional(false) {}
        };

        struct GoldenPathTest
        {
            GoldenPathType type;
            std::wstring name;
            std::wstring description;
            std::vector<GoldenStep> steps;
            std::wstring appPath;
            std::wstring appArgs;
            std::wstring windowClassName;
            int timeoutSeconds;

            GoldenPathTest() : timeoutSeconds(60) {}
        };

        struct GoldenResult
        {
            GoldenPathType type;
            std::wstring testName;
            TestOutcome outcome;
            std::vector<std::chrono::nanoseconds> stepLatencies;
            std::vector<std::wstring> errors;
            std::chrono::nanoseconds totalTime;
            int passedSteps;
            int totalSteps;
        };

        class GoldenPathExecutor
        {
        public:
            ~GoldenPathExecutor() = default;

            static GoldenPathExecutor& Instance();

            bool Initialize();
            void Uninitialize();

            GoldenResult RunTest(const GoldenPathTest& test);
            std::vector<GoldenResult> RunAllTests();
            std::vector<GoldenResult> RunTestsByType(GoldenPathType type);

            void AddTest(const GoldenPathTest& test);
            void RemoveTest(const std::wstring& name);

            std::vector<GoldenPathTest> GetAllTests() const;

        private:
            GoldenPathExecutor();

            bool LaunchApp(const GoldenPathTest& test, PROCESS_INFORMATION& pi);
            bool WaitForWindow(const std::wstring& className, int timeoutMs, HWND& hwnd);
            bool ExecuteStep(const GoldenStep& step, HWND hwnd, std::chrono::nanoseconds& latency);
            bool ValidateOutput(HWND hwnd, const std::wstring& expected);

            std::vector<GoldenPathTest> m_tests;
        };

        class GoldenPathManager
        {
        public:
            ~GoldenPathManager() = default;

            static GoldenPathManager& Instance();

            bool LoadGoldenPaths(const std::wstring& directory);
            bool SaveGoldenPaths(const std::wstring& directory);

            void RegisterDefaultPaths();

            std::shared_ptr<GoldenPathTest> GetPath(GoldenPathType type);
            std::vector<std::shared_ptr<GoldenPathTest>> GetAllPaths() const;

        private:
            GoldenPathManager();

            std::unordered_map<GoldenPathType, std::shared_ptr<GoldenPathTest>> m_paths;
        };
    }
}