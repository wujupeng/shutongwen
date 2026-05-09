#include "utils/logger.h"
#include "utils/config.h"
#include "ime/DictionaryManager.h"
#include <windows.h>

namespace ShuTongWen
{
    bool InitializeCore()
    {
        Logger::Initialize();
        Logger::Info("Initializing ShuTongWen Core...");

        std::wstring configPath = Win32Utils::GetLocalAppDataPath() + L"\\ShuTongWen\\config.json";
        ConfigManager::Instance().Load(configPath);

        std::wstring dbPath = Win32Utils::GetLocalAppDataPath() + L"\\ShuTongWen\\dict\\shutongwen.db";
        if (!DictionaryManager::Instance().Initialize(dbPath))
        {
            Logger::Warn("Failed to initialize dictionary manager");
            return false;
        }

        Logger::Info("ShuTongWen Core initialized successfully");
        return true;
    }

    void UninitializeCore()
    {
        Logger::Info("Uninitializing ShuTongWen Core...");
        
        DictionaryManager::Instance().Uninitialize();
        Logger::Shutdown();
    }
}