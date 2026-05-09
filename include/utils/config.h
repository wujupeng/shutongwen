#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace ShuTongWen
{
    class ConfigManager
    {
    public:
        ~ConfigManager() = default;

        static ConfigManager& Instance();

        bool Load(const std::wstring& configPath);
        bool Save(const std::wstring& configPath = L"");

        std::wstring GetString(const std::string& key, const std::wstring& defaultValue = L"") const;
        int GetInt(const std::string& key, int defaultValue = 0) const;
        bool GetBool(const std::string& key, bool defaultValue = false) const;
        double GetDouble(const std::string& key, double defaultValue = 0.0) const;

        void SetString(const std::string& key, const std::wstring& value);
        void SetInt(const std::string& key, int value);
        void SetBool(const std::string& key, bool value);
        void SetDouble(const std::string& key, double value);

        bool HasKey(const std::string& key) const;
        void RemoveKey(const std::string& key);

        void ResetToDefaults();

    private:
        ConfigManager();
        ConfigManager(const ConfigManager&) = delete;
        ConfigManager& operator=(const ConfigManager&) = delete;

        void InitializeDefaults();

        nlohmann::json m_config;
        std::wstring m_configPath;
    };
}
