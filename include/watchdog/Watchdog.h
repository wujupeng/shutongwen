#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>

namespace ShuTongWen
{
    namespace Watchdog
    {
        enum class ProcessType
        {
            Unknown,
            IME,
            UI,
            AI,
            Service
        };

        enum class HealthStatus
        {
            Healthy,
            Warning,
            Unresponsive,
            Crashed,
            Restarting
        };

        struct ProcessInfo
        {
            ProcessType type;
            std::wstring name;
            DWORD pid;
            HANDLE handle;
            std::chrono::system_clock::time_point startTime;
            std::chrono::system_clock::time_point lastHeartbeat;
            HealthStatus status;
            int restartCount;
            std::wstring executablePath;
        };

        struct CrashDumpInfo
        {
            std::wstring processName;
            DWORD pid;
            std::wstring dumpPath;
            std::chrono::system_clock::time_point timestamp;
            std::wstring exceptionCode;
            std::wstring exceptionAddress;
        };

        class ProcessMonitor
        {
        public:
            ProcessMonitor(ProcessType type, const std::wstring& name, const std::wstring& path);
            ~ProcessMonitor();

            bool Start();
            bool Stop();
            bool Restart();
            
            HealthStatus GetStatus() const;
            DWORD GetPid() const;
            std::wstring GetName() const;
            ProcessType GetProcessType() const { return m_type; }
            
            void SetHeartbeat();
            bool IsHeartbeatTimedOut(std::chrono::seconds timeout) const;

        private:
            static DWORD WINAPI MonitorThread(LPVOID param);
            void MonitorLoop();

            ProcessType m_type;
            std::wstring m_name;
            std::wstring m_path;
            HANDLE m_process;
            DWORD m_pid;
            HANDLE m_thread;
            bool m_running;
            HealthStatus m_status;
            std::chrono::system_clock::time_point m_lastHeartbeat;
            int m_restartCount;
            CRITICAL_SECTION m_cs;
        };

        class FreezeDetector
        {
        public:
            ~FreezeDetector() = default;

            static FreezeDetector& Instance();

            bool Initialize();
            void Uninitialize();

            void RegisterProcess(ProcessType type, DWORD pid);
            void UnregisterProcess(DWORD pid);

            void SetFreezeThreshold(std::chrono::seconds threshold);
            void OnHeartbeat(DWORD pid);

            std::function<void(DWORD pid)> onFreezeDetected;

        private:
            FreezeDetector();

            static DWORD WINAPI DetectionThread(LPVOID param);
            void DetectionLoop();

            struct ProcessHeartbeat
            {
                ProcessType type;
                std::chrono::system_clock::time_point lastHeartbeat;
            };

            std::unordered_map<DWORD, ProcessHeartbeat> m_heartbeats;
            std::chrono::seconds m_threshold;
            HANDLE m_thread;
            bool m_running;
            CRITICAL_SECTION m_cs;
        };

        class CrashDumper
        {
        public:
            ~CrashDumper() = default;

            static CrashDumper& Instance();

            bool Initialize();
            void Uninitialize();

            bool GenerateDump(DWORD pid, const std::wstring& reason, CrashDumpInfo& info);
            std::vector<CrashDumpInfo> GetRecentDumps(size_t limit = 10) const;
            void CleanOldDumps(int daysToKeep = 7);

            std::function<void(const CrashDumpInfo&)> onDumpGenerated;

        private:
            CrashDumper();

            std::wstring GetDumpDirectory() const;
            std::wstring GenerateDumpFileName() const;

            std::vector<CrashDumpInfo> m_dumps;
        };

        class WatchdogManager
        {
        public:
            ~WatchdogManager() = default;

            static WatchdogManager& Instance();

            bool Initialize();
            void Uninitialize();

            bool StartAllMonitors();
            bool StopAllMonitors();

            std::shared_ptr<ProcessMonitor> GetMonitor(ProcessType type);
            std::vector<ProcessInfo> GetAllProcessInfo() const;

            void Enable(bool enabled);
            bool IsEnabled() const { return m_enabled; }

        private:
            WatchdogManager();

            bool m_enabled;
            std::unordered_map<ProcessType, std::shared_ptr<ProcessMonitor>> m_monitors;
        };
    }
}