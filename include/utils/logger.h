#pragma once

#include <memory>
#include <string>
#include <spdlog/spdlog.h>

namespace ShuTongWen
{
    class Logger
    {
    public:
        Logger() = default;
        ~Logger() = default;

        static void Initialize(const std::wstring& logPath = L"");
        static void Shutdown();

        static void Trace(const char* msg);
        static void Debug(const char* msg);
        static void Info(const char* msg);
        static void Warn(const char* msg);
        static void Error(const char* msg);
        static void Critical(const char* msg);

        std::shared_ptr<spdlog::logger> log;

    private:
        static std::unique_ptr<Logger> s_instance;
    };
}
