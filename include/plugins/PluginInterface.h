#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <memory>

namespace ShuTongWen
{
    namespace Plugins
    {
        enum class PluginType
        {
            Unknown,
            Emoji,
            Clipboard,
            Translator,
            OCR,
            AIAssistant,
            EnterpriseDict
        };

        enum class PluginStatus
        {
            Unloaded,
            Loading,
            Loaded,
            Error
        };

        struct PluginInfo
        {
            std::wstring id;
            std::wstring name;
            std::wstring description;
            std::wstring version;
            PluginType type;
            PluginStatus status;
            std::wstring filePath;
            bool enabled;
        };

        class IPlugin
        {
        public:
            virtual ~IPlugin() = default;

            virtual bool Initialize() = 0;
            virtual void Uninitialize() = 0;
            
            virtual PluginInfo GetInfo() const = 0;
            virtual PluginStatus GetStatus() const = 0;

            virtual std::wstring GetName() const = 0;
            virtual std::wstring GetDescription() const = 0;
            virtual PluginType GetType() const = 0;

            virtual bool IsEnabled() const = 0;
            virtual void SetEnabled(bool enabled) = 0;
        };

        class PluginManager
        {
        public:
            ~PluginManager() = default;

            static PluginManager& Instance();

            bool Initialize(const std::wstring& pluginsPath = L"");
            void Uninitialize();

            bool LoadPlugin(const std::wstring& filePath);
            bool UnloadPlugin(const std::wstring& pluginId);
            bool ReloadPlugin(const std::wstring& pluginId);

            std::vector<PluginInfo> GetPlugins() const;
            std::shared_ptr<IPlugin> GetPlugin(const std::wstring& pluginId) const;

            std::vector<std::shared_ptr<IPlugin>> GetPluginsByType(PluginType type) const;

            bool EnablePlugin(const std::wstring& pluginId);
            bool DisablePlugin(const std::wstring& pluginId);

            void ScanPlugins();

        private:
            PluginManager();
            PluginManager(const PluginManager&) = delete;
            PluginManager& operator=(const PluginManager&) = delete;

            bool LoadPluginsFromDirectory(const std::wstring& directory);
            HMODULE LoadPluginModule(const std::wstring& filePath);
            void UnloadPluginModule(HMODULE hModule);

            std::wstring m_pluginsPath;
            std::unordered_map<std::wstring, std::shared_ptr<IPlugin>> m_plugins;
            std::unordered_map<std::wstring, HMODULE> m_modules;
        };
    }
}