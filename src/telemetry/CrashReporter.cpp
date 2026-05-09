#include "telemetry/CrashReporter.h"
#include "utils/logger.h"
#include <DbgHelp.h>
#include <fstream>

#pragma comment(lib, "DbgHelp.lib")

namespace ShuTongWen
{
    namespace Telemetry
    {
        CrashReporter::CrashReporter()
            : m_enabled(true)
        {}

        CrashReporter& CrashReporter::Instance()
        {
            static CrashReporter instance;
            return instance;
        }

        bool CrashReporter::Initialize()
        {
            Logger::Info("Initializing CrashReporter...");
            return InstallExceptionHandler();
        }

        void CrashReporter::Uninitialize()
        {
            Logger::Info("Uninitializing CrashReporter...");
            UninstallExceptionHandler();
        }

        void CrashReporter::SetReportCallback(std::function<void(const CrashInfo&)> callback)
        {
            m_callback = callback;
        }

        void CrashReporter::ReportCrash(const EXCEPTION_POINTERS* pExceptionInfo)
        {
            if (!m_enabled)
                return;

            CrashInfo info;
            
            info.exceptionCode = std::to_wstring(pExceptionInfo->ExceptionRecord->ExceptionCode);
            info.exceptionAddress = L"0x" + std::to_wstring(
                reinterpret_cast<uintptr_t>(pExceptionInfo->ExceptionRecord->ExceptionAddress));

            HMODULE hModule;
            if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
                                 GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                 reinterpret_cast<LPCSTR>(pExceptionInfo->ExceptionRecord->ExceptionAddress),
                                 &hModule))
            {
                wchar_t modulePath[MAX_PATH];
                if (GetModuleFileNameW(hModule, modulePath, MAX_PATH))
                {
                    info.moduleName = modulePath;
                }
            }

            STACKFRAME64 stackFrame = {0};
            stackFrame.AddrPC.Mode = AddrModeFlat;
            stackFrame.AddrPC.Offset = reinterpret_cast<uint64_t>(pExceptionInfo->ContextRecord->Rip);
            stackFrame.AddrFrame.Mode = AddrModeFlat;
            stackFrame.AddrFrame.Offset = reinterpret_cast<uint64_t>(pExceptionInfo->ContextRecord->Rbp);
            stackFrame.AddrStack.Mode = AddrModeFlat;
            stackFrame.AddrStack.Offset = reinterpret_cast<uint64_t>(pExceptionInfo->ContextRecord->Rsp);

            HANDLE hProcess = GetCurrentProcess();
            HANDLE hThread = GetCurrentThread();
            CONTEXT context = *pExceptionInfo->ContextRecord;

            for (int i = 0; i < 20; ++i)
            {
                if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, hProcess, hThread, &stackFrame, 
                                &context, nullptr, SymFunctionTableAccess64, 
                                SymGetModuleBase64, nullptr))
                {
                    break;
                }

                if (stackFrame.AddrPC.Offset != 0)
                {
                    info.callStack.push_back(L"0x" + std::to_wstring(stackFrame.AddrPC.Offset));
                }
            }

            info.timestamp = L"";
            info.processId = std::to_wstring(GetCurrentProcessId());
            info.threadId = std::to_wstring(GetCurrentThreadId());
            info.version = L"1.0.0";
            info.buildNumber = L"0";

            m_crashHistory.push_back(info);
            if (m_crashHistory.size() > 10)
            {
                m_crashHistory.erase(m_crashHistory.begin());
            }

            if (m_callback)
            {
                m_callback(info);
            }

            Logger::Fatal("Crash reported: Code={}, Address={}", 
                info.exceptionCode, info.exceptionAddress);
        }

        void CrashReporter::ReportException(const std::exception& ex)
        {
            if (!m_enabled)
                return;

            CrashInfo info;
            info.exceptionCode = L"STD_EXCEPTION";
            info.exceptionAddress = L"";
            info.moduleName = L"";
            info.functionName = L"";
            info.callStack.push_back(StringUtils::UTF8ToUTF16(ex.what()));
            info.timestamp = L"";
            info.processId = std::to_wstring(GetCurrentProcessId());
            info.threadId = std::to_wstring(GetCurrentThreadId());
            info.version = L"1.0.0";
            info.buildNumber = L"0";

            m_crashHistory.push_back(info);
            if (m_crashHistory.size() > 10)
            {
                m_crashHistory.erase(m_crashHistory.begin());
            }

            if (m_callback)
            {
                m_callback(info);
            }

            Logger::Error("Exception reported: {}", ex.what());
        }

        void CrashReporter::ReportFatalError(const std::wstring& message)
        {
            if (!m_enabled)
                return;

            CrashInfo info;
            info.exceptionCode = L"FATAL_ERROR";
            info.exceptionAddress = L"";
            info.moduleName = L"";
            info.functionName = L"";
            info.callStack.push_back(message);
            info.timestamp = L"";
            info.processId = std::to_wstring(GetCurrentProcessId());
            info.threadId = std::to_wstring(GetCurrentThreadId());
            info.version = L"1.0.0";
            info.buildNumber = L"0";

            m_crashHistory.push_back(info);
            if (m_crashHistory.size() > 10)
            {
                m_crashHistory.erase(m_crashHistory.begin());
            }

            if (m_callback)
            {
                m_callback(info);
            }

            Logger::Fatal("Fatal error: {}", message);
        }

        bool CrashReporter::IsEnabled() const
        {
            return m_enabled;
        }

        void CrashReporter::SetEnabled(bool enabled)
        {
            m_enabled = enabled;
        }

        std::vector<CrashInfo> CrashReporter::GetCrashHistory(size_t limit) const
        {
            std::vector<CrashInfo> result;
            size_t count = std::min(limit, m_crashHistory.size());
            result.resize(count);
            std::reverse_copy(m_crashHistory.begin(), m_crashHistory.begin() + count, result.begin());
            return result;
        }

        void CrashReporter::ClearCrashHistory()
        {
            m_crashHistory.clear();
        }

        bool CrashReporter::InstallExceptionHandler()
        {
            SetUnhandledExceptionFilter(ExceptionHandler);
            return true;
        }

        bool CrashReporter::UninstallExceptionHandler()
        {
            SetUnhandledExceptionFilter(nullptr);
            return true;
        }

        LONG WINAPI CrashReporter::ExceptionHandler(EXCEPTION_POINTERS* pExceptionInfo)
        {
            Instance().ReportCrash(pExceptionInfo);
            return EXCEPTION_EXECUTE_HANDLER;
        }
    }
}