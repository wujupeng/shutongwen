#include "watchdog/Watchdog.h"
#include "utils/logger.h"
#include <DbgHelp.h>
#include <filesystem>

#pragma comment(lib, "DbgHelp.lib")

namespace ShuTongWen
{
    namespace Watchdog
    {
        ProcessMonitor::ProcessMonitor(ProcessType type, const std::wstring& name, const std::wstring& path)
            : m_type(type), m_name(name), m_path(path), m_process(nullptr), 
              m_pid(0), m_thread(nullptr), m_running(false), 
              m_status(HealthStatus::Healthy), m_restartCount(0)
        {
            InitializeCriticalSection(&m_cs);
        }

        ProcessMonitor::~ProcessMonitor()
        {
            Stop();
            DeleteCriticalSection(&m_cs);
        }

        bool ProcessMonitor::Start()
        {
            EnterCriticalSection(&m_cs);
            
            if (m_running)
            {
                LeaveCriticalSection(&m_cs);
                return true;
            }

            STARTUPINFO si = {sizeof(si)};
            PROCESS_INFORMATION pi = {0};

            if (CreateProcessW(m_path.c_str(), nullptr, nullptr, nullptr, 
                              FALSE, 0, nullptr, nullptr, &si, &pi))
            {
                m_process = pi.hProcess;
                m_pid = pi.dwProcessId;
                m_startTime = std::chrono::system_clock::now();
                m_lastHeartbeat = m_startTime;
                m_status = HealthStatus::Healthy;

                m_running = true;
                m_thread = CreateThread(nullptr, 0, MonitorThread, this, 0, nullptr);

                LeaveCriticalSection(&m_cs);
                Logger::Info("Process {} started (PID: {})", m_name, m_pid);
                return true;
            }

            LeaveCriticalSection(&m_cs);
            Logger::Error("Failed to start process: {}", m_name);
            return false;
        }

        bool ProcessMonitor::Stop()
        {
            EnterCriticalSection(&m_cs);

            m_running = false;

            if (m_thread)
            {
                WaitForSingleObject(m_thread, 5000);
                CloseHandle(m_thread);
                m_thread = nullptr;
            }

            if (m_process)
            {
                TerminateProcess(m_process, 0);
                WaitForSingleObject(m_process, 5000);
                CloseHandle(m_process);
                m_process = nullptr;
            }

            m_pid = 0;
            m_status = HealthStatus::Healthy;

            LeaveCriticalSection(&m_cs);
            return true;
        }

        bool ProcessMonitor::Restart()
        {
            Logger::Info("Restarting process: {}", m_name);
            
            Stop();
            m_restartCount++;
            
            if (m_restartCount > 5)
            {
                Logger::Error("Process {} restarted too many times", m_name);
                return false;
            }

            return Start();
        }

        HealthStatus ProcessMonitor::GetStatus() const
        {
            return m_status;
        }

        DWORD ProcessMonitor::GetPid() const
        {
            return m_pid;
        }

        std::wstring ProcessMonitor::GetName() const
        {
            return m_name;
        }

        void ProcessMonitor::SetHeartbeat()
        {
            EnterCriticalSection(&m_cs);
            m_lastHeartbeat = std::chrono::system_clock::now();
            m_status = HealthStatus::Healthy;
            LeaveCriticalSection(&m_cs);
        }

        bool ProcessMonitor::IsHeartbeatTimedOut(std::chrono::seconds timeout) const
        {
            if (m_status == HealthStatus::Crashed)
                return true;

            auto now = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastHeartbeat);
            return elapsed > timeout;
        }

        DWORD WINAPI ProcessMonitor::MonitorThread(LPVOID param)
        {
            ProcessMonitor* monitor = reinterpret_cast<ProcessMonitor*>(param);
            monitor->MonitorLoop();
            return 0;
        }

        void ProcessMonitor::MonitorLoop()
        {
            while (m_running)
            {
                if (m_process)
                {
                    DWORD exitCode = STILL_ACTIVE;
                    GetExitCodeProcess(m_process, &exitCode);

                    if (exitCode != STILL_ACTIVE)
                    {
                        EnterCriticalSection(&m_cs);
                        m_status = HealthStatus::Crashed;
                        LeaveCriticalSection(&m_cs);
                        
                        Logger::Error("Process {} crashed (Exit code: {})", m_name, exitCode);
                        
                        Restart();
                    }
                    else if (IsHeartbeatTimedOut(std::chrono::seconds(30)))
                    {
                        EnterCriticalSection(&m_cs);
                        m_status = HealthStatus::Unresponsive;
                        LeaveCriticalSection(&m_cs);
                        
                        Logger::Warning("Process {} unresponsive, restarting", m_name);
                        
                        Restart();
                    }
                }

                Sleep(1000);
            }
        }

        FreezeDetector::FreezeDetector()
            : m_threshold(std::chrono::seconds(10)), m_thread(nullptr), m_running(false)
        {
            InitializeCriticalSection(&m_cs);
        }

        FreezeDetector& FreezeDetector::Instance()
        {
            static FreezeDetector instance;
            return instance;
        }

        bool FreezeDetector::Initialize()
        {
            Logger::Info("Initializing FreezeDetector...");

            m_running = true;
            m_thread = CreateThread(nullptr, 0, DetectionThread, this, 0, nullptr);

            return m_thread != nullptr;
        }

        void FreezeDetector::Uninitialize()
        {
            m_running = false;
            
            if (m_thread)
            {
                WaitForSingleObject(m_thread, 5000);
                CloseHandle(m_thread);
                m_thread = nullptr;
            }

            DeleteCriticalSection(&m_cs);
        }

        void FreezeDetector::RegisterProcess(ProcessType type, DWORD pid)
        {
            EnterCriticalSection(&m_cs);
            m_heartbeats[pid] = {type, std::chrono::system_clock::now()};
            LeaveCriticalSection(&m_cs);
        }

        void FreezeDetector::UnregisterProcess(DWORD pid)
        {
            EnterCriticalSection(&m_cs);
            m_heartbeats.erase(pid);
            LeaveCriticalSection(&m_cs);
        }

        void FreezeDetector::SetFreezeThreshold(std::chrono::seconds threshold)
        {
            m_threshold = threshold;
        }

        void FreezeDetector::OnHeartbeat(DWORD pid)
        {
            EnterCriticalSection(&m_cs);
            auto it = m_heartbeats.find(pid);
            if (it != m_heartbeats.end())
            {
                it->second.lastHeartbeat = std::chrono::system_clock::now();
            }
            LeaveCriticalSection(&m_cs);
        }

        DWORD WINAPI FreezeDetector::DetectionThread(LPVOID param)
        {
            FreezeDetector* detector = reinterpret_cast<FreezeDetector*>(param);
            detector->DetectionLoop();
            return 0;
        }

        void FreezeDetector::DetectionLoop()
        {
            while (m_running)
            {
                EnterCriticalSection(&m_cs);
                
                auto now = std::chrono::system_clock::now();
                std::vector<DWORD> frozenPids;

                for (const auto& pair : m_heartbeats)
                {
                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                        now - pair.second.lastHeartbeat);

                    if (elapsed > m_threshold)
                    {
                        frozenPids.push_back(pair.first);
                    }
                }

                LeaveCriticalSection(&m_cs);

                for (DWORD pid : frozenPids)
                {
                    Logger::Warning("Freeze detected for PID: {}", pid);
                    
                    if (onFreezeDetected)
                    {
                        onFreezeDetected(pid);
                    }
                }

                Sleep(1000);
            }
        }

        CrashDumper::CrashDumper()
        {}

        CrashDumper& CrashDumper::Instance()
        {
            static CrashDumper instance;
            return instance;
        }

        bool CrashDumper::Initialize()
        {
            Logger::Info("Initializing CrashDumper...");
            return true;
        }

        void CrashDumper::Uninitialize()
        {
            m_dumps.clear();
        }

        bool CrashDumper::GenerateDump(DWORD pid, const std::wstring& reason, CrashDumpInfo& info)
        {
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
            if (!hProcess)
            {
                Logger::Error("Failed to open process: {}", pid);
                return false;
            }

            std::wstring dumpPath = GetDumpDirectory() + L"\\" + GenerateDumpFileName();

            HANDLE hFile = CreateFileW(dumpPath.c_str(), GENERIC_WRITE, 0, nullptr,
                                      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

            if (hFile == INVALID_HANDLE_VALUE)
            {
                CloseHandle(hProcess);
                Logger::Error("Failed to create dump file: {}", dumpPath);
                return false;
            }

            MINIDUMP_EXCEPTION_INFORMATION exceptionInfo = {0};
            exceptionInfo.ThreadId = GetCurrentThreadId();
            exceptionInfo.ExceptionPointers = nullptr;
            exceptionInfo.ClientPointers = FALSE;

            BOOL success = MiniDumpWriteDump(hProcess, pid, hFile,
                                            MiniDumpNormal, &exceptionInfo, nullptr, nullptr);

            CloseHandle(hFile);
            CloseHandle(hProcess);

            if (success)
            {
                info.processName = L"";
                info.pid = pid;
                info.dumpPath = dumpPath;
                info.timestamp = std::chrono::system_clock::now();
                info.exceptionCode = L"";
                info.exceptionAddress = L"";

                m_dumps.push_back(info);
                if (m_dumps.size() > 100)
                {
                    m_dumps.erase(m_dumps.begin());
                }

                Logger::Info("Crash dump generated: {}", dumpPath);

                if (onDumpGenerated)
                {
                    onDumpGenerated(info);
                }

                return true;
            }

            Logger::Error("Failed to generate dump for PID: {}", pid);
            return false;
        }

        std::vector<CrashDumpInfo> CrashDumper::GetRecentDumps(size_t limit) const
        {
            std::vector<CrashDumpInfo> result;
            size_t count = std::min(limit, m_dumps.size());
            result.resize(count);
            std::reverse_copy(m_dumps.begin(), m_dumps.begin() + count, result.begin());
            return result;
        }

        void CrashDumper::CleanOldDumps(int daysToKeep)
        {
            auto now = std::chrono::system_clock::now();
            auto threshold = std::chrono::hours(daysToKeep * 24);

            std::erase_if(m_dumps, [&](const CrashDumpInfo& dump) {
                auto age = std::chrono::duration_cast<std::chrono::hours>(now - dump.timestamp);
                return age > threshold;
            });
        }

        std::wstring CrashDumper::GetDumpDirectory() const
        {
            wchar_t path[MAX_PATH];
            GetTempPathW(MAX_PATH, path);
            std::wstring dir = std::wstring(path) + L"ShuTongWen\\dumps";
            
            CreateDirectoryW(dir.c_str(), nullptr);
            return dir;
        }

        std::wstring CrashDumper::GenerateDumpFileName() const
        {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            
            wchar_t timestamp[64];
            wcsftime(timestamp, sizeof(timestamp), L"%Y%m%d_%H%M%S", localtime(&time));
            
            return L"dump_" + std::wstring(timestamp) + L".dmp";
        }

        WatchdogManager::WatchdogManager()
            : m_enabled(true)
        {}

        WatchdogManager& WatchdogManager::Instance()
        {
            static WatchdogManager instance;
            return instance;
        }

        bool WatchdogManager::Initialize()
        {
            Logger::Info("Initializing WatchdogManager...");

            m_monitors[ProcessType::IME] = std::make_shared<ProcessMonitor>(
                ProcessType::IME, L"IME Core", L"ime_core.dll");
            
            m_monitors[ProcessType::UI] = std::make_shared<ProcessMonitor>(
                ProcessType::UI, L"WinUI", L"ime_ui.exe");
            
            m_monitors[ProcessType::AI] = std::make_shared<ProcessMonitor>(
                ProcessType::AI, L"AI Runtime", L"ime_ai.exe");

            FreezeDetector::Instance().Initialize();
            CrashDumper::Instance().Initialize();

            Logger::Info("WatchdogManager initialized");
            return true;
        }

        void WatchdogManager::Uninitialize()
        {
            StopAllMonitors();
            m_monitors.clear();
            
            FreezeDetector::Instance().Uninitialize();
            CrashDumper::Instance().Uninitialize();
        }

        bool WatchdogManager::StartAllMonitors()
        {
            bool allStarted = true;
            
            for (const auto& pair : m_monitors)
            {
                if (!pair.second->Start())
                {
                    allStarted = false;
                }
            }

            return allStarted;
        }

        bool WatchdogManager::StopAllMonitors()
        {
            for (const auto& pair : m_monitors)
            {
                pair.second->Stop();
            }
            return true;
        }

        std::shared_ptr<ProcessMonitor> WatchdogManager::GetMonitor(ProcessType type)
        {
            auto it = m_monitors.find(type);
            return it != m_monitors.end() ? it->second : nullptr;
        }

        std::vector<ProcessInfo> WatchdogManager::GetAllProcessInfo() const
        {
            std::vector<ProcessInfo> infos;
            
            for (const auto& pair : m_monitors)
            {
                ProcessInfo info;
                info.type = pair.second->GetProcessType();
                info.name = pair.second->GetName();
                info.pid = pair.second->GetPid();
                info.status = pair.second->GetStatus();
                infos.push_back(info);
            }

            return infos;
        }

        void WatchdogManager::Enable(bool enabled)
        {
            m_enabled = enabled;
        }
    }
}