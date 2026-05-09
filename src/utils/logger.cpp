#include "utils/logger.h"
#include "utils/win32_utils.h"
#include "utils/string_utils.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace ShuTongWen
{
    std::unique_ptr<Logger> Logger::s_instance = nullptr;

    void Logger::Initialize(const std::wstring& logPath)
    {
        if (s_instance)
            return;

        std::wstring path = logPath.empty() 
            ? Win32Utils::GetLocalAppDataPath() + L"\\ShuTongWen\\logs\\ime.log" 
            : logPath;

        std::wstring dirPath = path.substr(0, path.find_last_of(L'\\'));
        Win32Utils::CreateDirectoryRecursive(dirPath);

        s_instance = std::make_unique<Logger>();
        
        auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            StringUtils::UTF16ToUTF8(path), true);
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        std::vector<spdlog::sink_ptr> sinks = {fileSink, consoleSink};
        s_instance->log = std::make_shared<spdlog::logger>("ShuTongWen", sinks.begin(), sinks.end());

        s_instance->log->set_level(spdlog::level::trace);
        s_instance->log->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

        s_instance->log->info("Logger initialized");
    }

    void Logger::Shutdown()
    {
        if (s_instance)
        {
            s_instance->log->info("Logger shutting down");
            s_instance.reset();
        }
    }

    void Logger::Trace(const char* msg)
    {
        if (s_instance) s_instance->log->trace(msg);
    }

    void Logger::Debug(const char* msg)
    {
        if (s_instance) s_instance->log->debug(msg);
    }

    void Logger::Info(const char* msg)
    {
        if (s_instance) s_instance->log->info(msg);
    }

    void Logger::Warn(const char* msg)
    {
        if (s_instance) s_instance->log->warn(msg);
    }

    void Logger::Error(const char* msg)
    {
        if (s_instance) s_instance->log->error(msg);
    }

    void Logger::Critical(const char* msg)
    {
        if (s_instance) s_instance->log->critical(msg);
    }
}
