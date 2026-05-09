#include "compatibility/CompatibilityTest.h"
#include "utils/logger.h"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace ShuTongWen
{
    namespace Compatibility
    {
        CompatibilityTestRunner::CompatibilityTestRunner()
            : m_running(false)
        {}

        CompatibilityTestRunner& CompatibilityTestRunner::Instance()
        {
            static CompatibilityTestRunner instance;
            return instance;
        }

        bool CompatibilityTestRunner::Initialize()
        {
            Logger::Info("Initializing CompatibilityTestRunner...");
            return true;
        }

        void CompatibilityTestRunner::Uninitialize()
        {
            CancelAllTests();
            m_tests.clear();
        }

        void CompatibilityTestRunner::AddTest(std::unique_ptr<ICompatibilityTest> test)
        {
            m_tests.push_back(std::move(test));
        }

        void CompatibilityTestRunner::RemoveTest(const std::wstring& testName)
        {
            m_tests.erase(
                std::remove_if(m_tests.begin(), m_tests.end(),
                    [&testName](const std::unique_ptr<ICompatibilityTest>& test) {
                        return test->GetName() == testName;
                    }),
                m_tests.end());
        }

        std::vector<TestResult> CompatibilityTestRunner::RunAllTests()
        {
            m_running = true;
            std::vector<TestResult> results;

            for (const auto& test : m_tests)
            {
                if (!m_running)
                    break;

                Logger::Info("Running test: {}", test->GetName());
                TestResult result = test->Run();
                results.push_back(result);

                Logger::Info("Test {} completed: {}", 
                    test->GetName(), 
                    result.status == TestStatus::Passed ? L"PASSED" : L"FAILED");
            }

            m_running = false;
            return results;
        }

        TestResult CompatibilityTestRunner::RunTest(const std::wstring& testName)
        {
            for (const auto& test : m_tests)
            {
                if (test->GetName() == testName)
                {
                    return test->Run();
                }
            }

            TestResult result;
            result.testName = testName;
            result.status = TestStatus::Failed;
            result.errorMessage = L"Test not found";
            return result;
        }

        std::vector<TestResult> CompatibilityTestRunner::RunTestsByCategory(TestCategory category)
        {
            m_running = true;
            std::vector<TestResult> results;

            for (const auto& test : m_tests)
            {
                if (!m_running)
                    break;

                if (test->GetCategory() == category)
                {
                    Logger::Info("Running test: {}", test->GetName());
                    TestResult result = test->Run();
                    results.push_back(result);
                }
            }

            m_running = false;
            return results;
        }

        void CompatibilityTestRunner::CancelAllTests()
        {
            m_running = false;
            for (const auto& test : m_tests)
            {
                test->Cancel();
            }
        }

        std::vector<std::wstring> CompatibilityTestRunner::GetTestNames() const
        {
            std::vector<std::wstring> names;
            for (const auto& test : m_tests)
            {
                names.push_back(test->GetName());
            }
            return names;
        }

        std::vector<ICompatibilityTest*> CompatibilityTestRunner::GetTestsByCategory(TestCategory category) const
        {
            std::vector<ICompatibilityTest*> tests;
            for (const auto& test : m_tests)
            {
                if (test->GetCategory() == category)
                {
                    tests.push_back(test.get());
                }
            }
            return tests;
        }

        void CompatibilityTestRunner::ExportResults(const std::wstring& filePath)
        {
            json root = json::array();

            for (const auto& test : m_tests)
            {
                TestResult result = test->Run();

                json testJson;
                testJson["name"] = StringUtils::UTF16ToUTF8(result.testName);
                testJson["status"] = static_cast<int>(result.status);
                testJson["duration"] = result.duration.count();
                testJson["error"] = StringUtils::UTF16ToUTF8(result.errorMessage);

                json logs = json::array();
                for (const auto& log : result.logs)
                {
                    logs.push_back(StringUtils::UTF16ToUTF8(log));
                }
                testJson["logs"] = logs;

                root.push_back(testJson);
            }

            std::wofstream file(filePath);
            if (file.is_open())
            {
                file << StringUtils::UTF8ToUTF16(root.dump(2));
                file.close();
            }
        }

        bool ChromeTest::Initialize(const TestConfig& config)
        {
            m_config = config;
            m_cancelled = false;
            return true;
        }

        TestResult ChromeTest::Run()
        {
            TestResult result;
            result.testName = GetName();
            result.category = GetCategory();
            result.status = TestStatus::Running;

            auto startTime = std::chrono::high_resolution_clock::now();

            try
            {
                if (m_cancelled)
                {
                    result.status = TestStatus::Skipped;
                    return result;
                }

                STARTUPINFO si = {sizeof(si)};
                PROCESS_INFORMATION pi = {0};

                if (CreateProcessW(m_config.appPath.c_str(), 
                    const_cast<wchar_t*>(m_config.appArgs.c_str()),
                    nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
                {
                    WaitForInputIdle(pi.hProcess, 5000);

                    HWND hwnd = nullptr;
                    for (int i = 0; i < 50; ++i)
                    {
                        hwnd = FindWindowW(L"Chrome_WidgetWin_1", nullptr);
                        if (hwnd)
                            break;
                        Sleep(100);
                    }

                    if (hwnd)
                    {
                        SetForegroundWindow(hwnd);
                        Sleep(200);

                        for (wchar_t ch : m_config.testInput)
                        {
                            if (m_cancelled)
                                break;
                            
                            INPUT input = {0};
                            input.type = INPUT_KEYBOARD;
                            input.ki.wVk = VkKeyScanW(ch) & 0xFF;
                            input.ki.dwFlags = 0;
                            SendInput(1, &input, sizeof(INPUT));

                            input.ki.dwFlags = KEYEVENTF_KEYUP;
                            SendInput(1, &input, sizeof(INPUT));

                            Sleep(50);
                        }
                    }

                    WaitForSingleObject(pi.hProcess, m_config.timeoutMs);
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }

                result.status = TestStatus::Passed;
            }
            catch (const std::exception& ex)
            {
                result.status = TestStatus::Failed;
                result.errorMessage = StringUtils::UTF8ToUTF16(ex.what());
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            result.duration = endTime - startTime;

            return result;
        }

        void ChromeTest::Cancel()
        {
            m_cancelled = true;
        }

        bool EdgeTest::Initialize(const TestConfig& config)
        {
            m_config = config;
            m_cancelled = false;
            return true;
        }

        TestResult EdgeTest::Run()
        {
            TestResult result;
            result.testName = GetName();
            result.category = GetCategory();
            result.status = TestStatus::Running;

            auto startTime = std::chrono::high_resolution_clock::now();

            try
            {
                STARTUPINFO si = {sizeof(si)};
                PROCESS_INFORMATION pi = {0};

                if (CreateProcessW(m_config.appPath.c_str(),
                    const_cast<wchar_t*>(m_config.appArgs.c_str()),
                    nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
                {
                    WaitForInputIdle(pi.hProcess, 5000);
                    WaitForSingleObject(pi.hProcess, m_config.timeoutMs);
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }

                result.status = TestStatus::Passed;
            }
            catch (const std::exception& ex)
            {
                result.status = TestStatus::Failed;
                result.errorMessage = StringUtils::UTF8ToUTF16(ex.what());
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            result.duration = endTime - startTime;

            return result;
        }

        void EdgeTest::Cancel()
        {
            m_cancelled = true;
        }

        bool VSCodeTest::Initialize(const TestConfig& config)
        {
            m_config = config;
            m_cancelled = false;
            return true;
        }

        TestResult VSCodeTest::Run()
        {
            TestResult result;
            result.testName = GetName();
            result.category = GetCategory();
            result.status = TestStatus::Running;

            auto startTime = std::chrono::high_resolution_clock::now();

            try
            {
                STARTUPINFO si = {sizeof(si)};
                PROCESS_INFORMATION pi = {0};

                if (CreateProcessW(m_config.appPath.c_str(),
                    const_cast<wchar_t*>(m_config.appArgs.c_str()),
                    nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
                {
                    WaitForInputIdle(pi.hProcess, 5000);
                    WaitForSingleObject(pi.hProcess, m_config.timeoutMs);
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }

                result.status = TestStatus::Passed;
            }
            catch (const std::exception& ex)
            {
                result.status = TestStatus::Failed;
                result.errorMessage = StringUtils::UTF8ToUTF16(ex.what());
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            result.duration = endTime - startTime;

            return result;
        }

        void VSCodeTest::Cancel()
        {
            m_cancelled = true;
        }
    }
}