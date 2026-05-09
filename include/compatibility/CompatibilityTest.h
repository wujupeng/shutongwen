#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <chrono>

namespace ShuTongWen
{
    namespace Compatibility
    {
        enum class TestStatus
        {
            NotRun,
            Running,
            Passed,
            Failed,
            Skipped,
            Timeout
        };

        enum class TestCategory
        {
            Chrome,
            Edge,
            VSCode,
            WeChat,
            Office,
            Unreal,
            Unity,
            UWP,
            Game,
            AdminWindow
        };

        struct TestResult
        {
            std::wstring testName;
            TestCategory category;
            TestStatus status;
            std::wstring errorMessage;
            std::chrono::nanoseconds duration;
            std::vector<std::wstring> logs;
        };

        struct TestConfig
        {
            std::wstring appPath;
            std::wstring appArgs;
            std::wstring testInput;
            std::wstring expectedOutput;
            int timeoutMs;
            bool requireAdmin;
        };

        class ICompatibilityTest
        {
        public:
            virtual ~ICompatibilityTest() = default;
            virtual bool Initialize(const TestConfig& config) = 0;
            virtual TestResult Run() = 0;
            virtual std::wstring GetName() const = 0;
            virtual TestCategory GetCategory() const = 0;
            virtual void Cancel() = 0;
        };

        class CompatibilityTestRunner
        {
        public:
            ~CompatibilityTestRunner() = default;

            static CompatibilityTestRunner& Instance();

            bool Initialize();
            void Uninitialize();

            void AddTest(std::unique_ptr<ICompatibilityTest> test);
            void RemoveTest(const std::wstring& testName);
            
            std::vector<TestResult> RunAllTests();
            TestResult RunTest(const std::wstring& testName);
            std::vector<TestResult> RunTestsByCategory(TestCategory category);

            void CancelAllTests();

            std::vector<std::wstring> GetTestNames() const;
            std::vector<ICompatibilityTest*> GetTestsByCategory(TestCategory category) const;

            void ExportResults(const std::wstring& filePath);

        private:
            CompatibilityTestRunner();

            std::vector<std::unique_ptr<ICompatibilityTest>> m_tests;
            bool m_running;
        };

        class ChromeTest : public ICompatibilityTest
        {
        public:
            bool Initialize(const TestConfig& config) override;
            TestResult Run() override;
            std::wstring GetName() const override { return L"ChromeTest"; }
            TestCategory GetCategory() const override { return TestCategory::Chrome; }
            void Cancel() override;

        private:
            TestConfig m_config;
            bool m_cancelled;
        };

        class EdgeTest : public ICompatibilityTest
        {
        public:
            bool Initialize(const TestConfig& config) override;
            TestResult Run() override;
            std::wstring GetName() const override { return L"EdgeTest"; }
            TestCategory GetCategory() const override { return TestCategory::Edge; }
            void Cancel() override;

        private:
            TestConfig m_config;
            bool m_cancelled;
        };

        class VSCodeTest : public ICompatibilityTest
        {
        public:
            bool Initialize(const TestConfig& config) override;
            TestResult Run() override;
            std::wstring GetName() const override { return L"VSCodeTest"; }
            TestCategory GetCategory() const override { return TestCategory::VSCode; }
            void Cancel() override;

        private:
            TestConfig m_config;
            bool m_cancelled;
        };
    }
}