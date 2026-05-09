#include "utils/config.h"
#include "utils/logger.h"
#include "utils/win32_utils.h"
#include "utils/string_utils.h"
#include <fstream>

namespace ShuTongWen
{
    ConfigManager::ConfigManager()
    {
        InitializeDefaults();
    }

    ConfigManager& ConfigManager::Instance()
    {
        static ConfigManager instance;
        return instance;
    }

    bool ConfigManager::Load(const std::wstring& configPath)
    {
        m_configPath = configPath;

        if (!Win32Utils::FileExists(configPath))
        {
            Logger::Warn("Config file not found, using defaults");
            return false;
        }

        try
        {
            std::ifstream file(StringUtils::UTF16ToUTF8(configPath));
            if (file.is_open())
            {
                file >> m_config;
                file.close();
                Logger::Info("Config loaded");
                return true;
            }
        }
        catch (const std::exception& e)
        {
            Logger::Error("Failed to load config");
        }

        return false;
    }

    bool ConfigManager::Save(const std::wstring& configPath)
    {
        std::wstring path = configPath.empty() ? m_configPath : configPath;
        
        if (path.empty())
        {
            Logger::Error("Config path not set");
            return false;
        }

        try
        {
            std::wstring dirPath = path.substr(0, path.find_last_of(L'\\'));
            Win32Utils::CreateDirectoryRecursive(dirPath);

            std::ofstream file(StringUtils::UTF16ToUTF8(path));
            if (file.is_open())
            {
                file << m_config.dump(4);
                file.close();
                Logger::Info("Config saved");
                return true;
            }
        }
        catch (const std::exception& e)
        {
            Logger::Error("Failed to save config");
        }

        return false;
    }

    std::wstring ConfigManager::GetString(const std::string& key, const std::wstring& defaultValue) const
    {
        if (m_config.contains(key))
        {
            return StringUtils::UTF8ToUTF16(m_config[key].get<std::string>());
        }
        return defaultValue;
    }

    int ConfigManager::GetInt(const std::string& key, int defaultValue) const
    {
        if (m_config.contains(key))
        {
            return m_config[key].get<int>();
        }
        return defaultValue;
    }

    bool ConfigManager::GetBool(const std::string& key, bool defaultValue) const
    {
        if (m_config.contains(key))
        {
            return m_config[key].get<bool>();
        }
        return defaultValue;
    }

    double ConfigManager::GetDouble(const std::string& key, double defaultValue) const
    {
        if (m_config.contains(key))
        {
            return m_config[key].get<double>();
        }
        return defaultValue;
    }

    void ConfigManager::SetString(const std::string& key, const std::wstring& value)
    {
        m_config[key] = StringUtils::UTF16ToUTF8(value);
    }

    void ConfigManager::SetInt(const std::string& key, int value)
    {
        m_config[key] = value;
    }

    void ConfigManager::SetBool(const std::string& key, bool value)
    {
        m_config[key] = value;
    }

    void ConfigManager::SetDouble(const std::string& key, double value)
    {
        m_config[key] = value;
    }

    bool ConfigManager::HasKey(const std::string& key) const
    {
        return m_config.contains(key);
    }

    void ConfigManager::RemoveKey(const std::string& key)
    {
        m_config.erase(key);
    }

    void ConfigManager::InitializeDefaults()
    {
        m_config["ime.enabled"] = true;
        m_config["ime.mode"] = "pinyin";
        m_config["ui.theme"] = "dark";
        m_config["ui.font_size"] = 14;
        m_config["candidate.count"] = 9;
        m_config["hotkey.switch"] = "Shift";
        m_config["hotkey.full_width"] = "Ctrl+Shift+A";
        m_config["prediction.enabled"] = true;
        m_config["user_dictionary.enabled"] = true;
        m_config["telemetry.enabled"] = false;
    }
}
