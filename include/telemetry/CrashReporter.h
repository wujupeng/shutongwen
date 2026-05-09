#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <memory>

namespace ShuTongWen
{
    namespace Telemetry
    {
        struct CrashInfo
        {
            std::wstring exceptionCode;
            std::wstring exceptionAddress;
            std::wstring moduleName;
            std::wstring functionName;
            std::vector<std::wstring> callStack;
            std::wstring timestamp;
            std::wstring processId;
            std::wstring threadId;
            std::wstring version;
            std::wstring buildNumber;
        };

        class CrashReporter
        {
        public:
            ~CrashReporter() = default;

            static CrashReporter& Instance();

            bool Initialize();
            void Uninitialize();

            void SetReportCallback(std::function<void(const CrashInfo&)> callback);

            void ReportCrash(const EXCEPTION_POINTERS* pExceptionInfo);
            void ReportException(const std::exception& ex);
            void ReportFatalError(const std::wstring& message);

            bool IsEnabled() const { return m_enabled; }
            void SetEnabled(bool enabled);

            std::vector<CrashInfo> GetCrashHistory(size_t limit = 10) const;
            void ClearCrashHistory();

        private:
            CrashReporter();

            bool InstallExceptionHandler();
            bool UninstallExceptionHandler();

            static LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* pExceptionInfo);
            
            bool m_enabled;
            std::function<void(const CrashInfo&)> m_callback;
            std::vector<CrashInfo> m_crashHistory;
        };
    }
}