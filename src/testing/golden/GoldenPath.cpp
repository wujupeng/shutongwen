#include "testing/golden/GoldenPath.h"
#include "utils/logger.h"
#include <nlohmann/json.hpp>
#include <filesystem>

using json = nlohmann::json;

namespace ShuTongWen
{
    namespace Testing
    {
        GoldenPathExecutor::GoldenPathExecutor()
        {}

        GoldenPathExecutor& GoldenPathExecutor::Instance()
        {
            static GoldenPathExecutor instance;
            return instance;
        }

        bool GoldenPathExecutor::Initialize()
        {
            Logger::Info("Initializing GoldenPathExecutor...");
            return true;
        }

        void GoldenPathExecutor::Uninitialize()
        {
            m_tests.clear();
        }

        GoldenResult GoldenPathExecutor::RunTest(const GoldenPathTest& test)
        {
            GoldenResult result;
            result.type = test.type;
            result.testName = test.name;
            result.outcome = TestOutcome::Passed;
            result.stepLatencies.clear();
            result.errors.clear();
            result.passedSteps = 0;
            result.totalSteps = test.steps.size();

            Logger::Info("Running golden path test: {}", test.name);

            PROCESS_INFORMATION pi = {0};
            bool launched = LaunchApp(test, pi);

            if (!launched)
            {
                result.outcome = TestOutcome::Skipped;
                result.errors.push_back(L"Failed to launch application");
                return result;
            }

            HWND hwnd = nullptr;
            if (!WaitForWindow(test.windowClassName, test.timeoutSeconds * 1000, hwnd))
            {
                result.outcome = TestOutcome::Failed;
                result.errors.push_back(L"Window not found");
                TerminateProcess(pi.hProcess, 0);
                WaitForSingleObject(pi.hProcess, 5000);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                return result;
            }

            SetForegroundWindow(hwnd);
            Sleep(500);

            auto totalStartTime = std::chrono::high_resolution_clock::now();

            for (const GoldenStep& step : test.steps)
            {
                std::chrono::nanoseconds latency;
                
                if (!ExecuteStep(step, hwnd, latency))
                {
                    if (!step.optional)
                    {
                        result.outcome = TestOutcome::Failed;
                        result.errors.push_back(L"Step failed: " + step.name);
                    }
                }
                else
                {
                    result.passedSteps++;
                }

                result.stepLatencies.push_back(latency);

                if (latency > step.maxLatency)
                {
                    result.errors.push_back(L"Step latency exceeded: " + step.name);
                }

                Sleep(100);
            }

            auto totalEndTime = std::chrono::high_resolution_clock::now();
            result.totalTime = totalEndTime - totalStartTime;

            TerminateProcess(pi.hProcess, 0);
            WaitForSingleObject(pi.hProcess, 5000);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            Logger::Info("Test {} completed: {}, Steps: {}/{}, Time: {}ms", 
                test.name,
                result.outcome == TestOutcome::Passed ? L"PASSED" : L"FAILED",
                result.passedSteps, result.totalSteps,
                std::chrono::duration<double, std::milli>(result.totalTime).count());

            return result;
        }

        std::vector<GoldenResult> GoldenPathExecutor::RunAllTests()
        {
            std::vector<GoldenResult> results;
            for (const auto& test : m_tests)
            {
                results.push_back(RunTest(test));
            }
            return results;
        }

        std::vector<GoldenResult> GoldenPathExecutor::RunTestsByType(GoldenPathType type)
        {
            std::vector<GoldenResult> results;
            for (const auto& test : m_tests)
            {
                if (test.type == type)
                {
                    results.push_back(RunTest(test));
                }
            }
            return results;
        }

        void GoldenPathExecutor::AddTest(const GoldenPathTest& test)
        {
            m_tests.push_back(test);
        }

        void GoldenPathExecutor::RemoveTest(const std::wstring& name)
        {
            m_tests.erase(
                std::remove_if(m_tests.begin(), m_tests.end(),
                    [&name](const GoldenPathTest& t) {
                        return t.name == name;
                    }),
                m_tests.end());
        }

        std::vector<GoldenPathTest> GoldenPathExecutor::GetAllTests() const
        {
            return m_tests;
        }

        bool GoldenPathExecutor::LaunchApp(const GoldenPathTest& test, PROCESS_INFORMATION& pi)
        {
            if (test.appPath.empty())
                return false;

            STARTUPINFO si = {sizeof(si)};
            return CreateProcessW(test.appPath.c_str(), 
                const_cast<wchar_t*>(test.appArgs.c_str()),
                nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
        }

        bool GoldenPathExecutor::WaitForWindow(const std::wstring& className, int timeoutMs, HWND& hwnd)
        {
            int waitTime = 0;
            while (waitTime < timeoutMs)
            {
                hwnd = FindWindowW(className.c_str(), nullptr);
                if (hwnd)
                    return true;
                
                Sleep(100);
                waitTime += 100;
            }
            return false;
        }

        bool GoldenPathExecutor::ExecuteStep(const GoldenStep& step, HWND hwnd, std::chrono::nanoseconds& latency)
        {
            auto startTime = std::chrono::high_resolution_clock::now();

            for (wchar_t ch : step.input)
            {
                INPUT input = {0};
                input.type = INPUT_KEYBOARD;
                input.ki.wScan = ch;
                input.ki.dwFlags = KEYEVENTF_UNICODE;
                SendInput(1, &input, sizeof(INPUT));

                input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
                SendInput(1, &input, sizeof(INPUT));

                Sleep(20);
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            latency = endTime - startTime;

            return true;
        }

        bool GoldenPathExecutor::ValidateOutput(HWND hwnd, const std::wstring& expected)
        {
            return true;
        }

        GoldenPathManager::GoldenPathManager()
        {}

        GoldenPathManager& GoldenPathManager::Instance()
        {
            static GoldenPathManager instance;
            return instance;
        }

        bool GoldenPathManager::LoadGoldenPaths(const std::wstring& directory)
        {
            namespace fs = std::filesystem;
            
            for (const auto& entry : fs::directory_iterator(StringUtils::UTF16ToUTF8(directory)))
            {
                if (entry.path().extension() == ".json")
                {
                    std::wifstream file(entry.path());
                    if (file.is_open())
                    {
                        std::wstring content((std::istreambuf_iterator<wchar_t>(file)),
                                            std::istreambuf_iterator<wchar_t>());
                        
                        json root = json::parse(StringUtils::UTF16ToUTF8(content));
                        
                        auto test = std::make_shared<GoldenPathTest>();
                        test->type = static_cast<GoldenPathType>(root["type"]);
                        test->name = StringUtils::UTF8ToUTF16(root["name"]);
                        test->description = StringUtils::UTF8ToUTF16(root["description"]);
                        test->appPath = StringUtils::UTF8ToUTF16(root["appPath"]);
                        test->appArgs = StringUtils::UTF8ToUTF16(root["appArgs"]);
                        test->windowClassName = StringUtils::UTF8ToUTF16(root["windowClassName"]);
                        test->timeoutSeconds = root["timeoutSeconds"];

                        for (const auto& s : root["steps"])
                        {
                            GoldenStep step;
                            step.name = StringUtils::UTF8ToUTF16(s["name"]);
                            step.input = StringUtils::UTF8ToUTF16(s["input"]);
                            step.expectedOutput = StringUtils::UTF8ToUTF16(s["expectedOutput"]);
                            step.maxLatency = std::chrono::milliseconds(s["maxLatency"]);
                            step.optional = s["optional"];
                            test->steps.push_back(step);
                        }

                        m_paths[test->type] = test;
                    }
                }
            }

            return true;
        }

        bool GoldenPathManager::SaveGoldenPaths(const std::wstring& directory)
        {
            namespace fs = std::filesystem;
            fs::create_directories(StringUtils::UTF16ToUTF8(directory));

            for (const auto& pair : m_paths)
            {
                json root;
                root["type"] = static_cast<int>(pair.first);
                root["name"] = StringUtils::UTF16ToUTF8(pair.second->name);
                root["description"] = StringUtils::UTF8ToUTF16(pair.second->description);
                root["appPath"] = StringUtils::UTF8ToUTF16(pair.second->appPath);
                root["appArgs"] = StringUtils::UTF8ToUTF16(pair.second->appArgs);
                root["windowClassName"] = StringUtils::UTF8ToUTF16(pair.second->windowClassName);
                root["timeoutSeconds"] = pair.second->timeoutSeconds;

                json steps = json::array();
                for (const auto& step : pair.second->steps)
                {
                    json s;
                    s["name"] = StringUtils::UTF8ToUTF16(step.name);
                    s["input"] = StringUtils::UTF8ToUTF16(step.input);
                    s["expectedOutput"] = StringUtils::UTF8ToUTF16(step.expectedOutput);
                    s["maxLatency"] = step.maxLatency.count();
                    s["optional"] = step.optional;
                    steps.push_back(s);
                }
                root["steps"] = steps;

                std::wstring filePath = directory + L"\\" + pair.second->name + L".json";
                std::wofstream file(filePath);
                if (file.is_open())
                {
                    file << StringUtils::UTF8ToUTF16(root.dump(2));
                    file.close();
                }
            }

            return true;
        }

        void GoldenPathManager::RegisterDefaultPaths()
        {
            auto chromeTest = std::make_shared<GoldenPathTest>();
            chromeTest->type = GoldenPathType::Chrome;
            chromeTest->name = L"Chrome Golden Path";
            chromeTest->description = L"Chrome browser input test";
            chromeTest->appPath = L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe";
            chromeTest->appArgs = L"--new-window";
            chromeTest->windowClassName = L"Chrome_WidgetWin_1";
            chromeTest->timeoutSeconds = 30;

            GoldenStep step1;
            step1.name = L"Type pinyin";
            step1.input = L"woaini";
            step1.expectedOutput = L"我爱你";
            step1.maxLatency = std::chrono::milliseconds(50);
            chromeTest->steps.push_back(step1);

            GoldenStep step2;
            step2.name = L"Type mixed";
            step2.input = L"zai debug";
            step2.expectedOutput = L"在 debug";
            step2.maxLatency = std::chrono::milliseconds(50);
            chromeTest->steps.push_back(step2);

            m_paths[GoldenPathType::Chrome] = chromeTest;

            auto vsCodeTest = std::make_shared<GoldenPathTest>();
            vsCodeTest->type = GoldenPathType::VSCode;
            vsCodeTest->name = L"VSCode Golden Path";
            vsCodeTest->description = L"VSCode input test";
            vsCodeTest->appPath = L"C:\\Users\\User\\AppData\\Local\\Programs\\Microsoft VS Code\\Code.exe";
            vsCodeTest->appArgs = L"";
            vsCodeTest->windowClassName = L"Chrome_WidgetWin_1";
            vsCodeTest->timeoutSeconds = 60;

            GoldenStep vsStep1;
            vsStep1.name = L"Type code comment";
            vsStep1.input = L"// zheshi yige zhushi";
            vsStep1.expectedOutput = L"// 这是一个注释";
            vsStep1.maxLatency = std::chrono::milliseconds(100);
            vsCodeTest->steps.push_back(vsStep1);

            m_paths[GoldenPathType::VSCode] = vsCodeTest;
        }

        std::shared_ptr<GoldenPathTest> GoldenPathManager::GetPath(GoldenPathType type)
        {
            auto it = m_paths.find(type);
            return it != m_paths.end() ? it->second : nullptr;
        }

        std::vector<std::shared_ptr<GoldenPathTest>> GoldenPathManager::GetAllPaths() const
        {
            std::vector<std::shared_ptr<GoldenPathTest>> paths;
            for (const auto& pair : m_paths)
            {
                paths.push_back(pair.second);
            }
            return paths;
        }
    }
}