#include "plugins/PluginInterface.h"
#include "utils/logger.h"
#include <filesystem>

namespace ShuTongWen
{
    namespace Plugins
    {
        PluginManager::PluginManager()
        {}

        PluginManager& PluginManager::Instance()
        {
            static PluginManager instance;
            return instance;
        }

        bool PluginManager::Initialize(const std::wstring& pluginsPath)
        {
            Logger::Info("Initializing PluginManager...");

            if (!pluginsPath.empty())
            {
                m_pluginsPath = pluginsPath;
            }
            else
            {
                m_pluginsPath = L"plugins";
            }

            ScanPlugins();
            
            Logger::Info("PluginManager initialized successfully");
            return true;
        }

        void PluginManager::Uninitialize()
        {
            Logger::Info("Uninitializing PluginManager...");
            
            for (auto& pair : m_modules)
            {
                UnloadPluginModule(pair.second);
            }
            m_modules.clear();
            m_plugins.clear();
        }

        void PluginManager::ScanPlugins()
        {
            try
            {
                std::filesystem::path path(m_pluginsPath);
                if (!std::filesystem::exists(path))
                {
                    std::filesystem::create_directories(path);
                    return;
                }

                for (const auto& entry : std::filesystem::directory_iterator(path))
                {
                    if (entry.is_regular_file() && entry.path().extension() == L".dll")
                    {
                        LoadPlugin(entry.path().wstring());
                    }
                }
            }
            catch (const std::exception& e)
            {
                Logger::Error("Failed to scan plugins: {}", e.what());
            }
        }

        bool PluginManager::LoadPlugin(const std::wstring& filePath)
        {
            HMODULE hModule = LoadPluginModule(filePath);
            if (!hModule)
                return false;

            FARPROC createFunc = GetProcAddress(hModule, "CreatePlugin");
            if (!createFunc)
            {
                UnloadPluginModule(hModule);
                return false;
            }

            using CreatePluginFunc = IPlugin* (*)();
            CreatePluginFunc createPlugin = reinterpret_cast<CreatePluginFunc>(createFunc);
            
            std::shared_ptr<IPlugin> plugin(createPlugin());
            if (!plugin)
            {
                UnloadPluginModule(hModule);
                return false;
            }

            if (!plugin->Initialize())
            {
                UnloadPluginModule(hModule);
                return false;
            }

            std::wstring pluginId = plugin->GetInfo().id;
            m_plugins[pluginId] = plugin;
            m_modules[pluginId] = hModule;

            Logger::Info("Plugin loaded: {}", plugin->GetName());
            return true;
        }

        bool PluginManager::UnloadPlugin(const std::wstring& pluginId)
        {
            auto it = m_plugins.find(pluginId);
            if (it == m_plugins.end())
                return false;

            it->second->Uninitialize();
            m_plugins.erase(it);

            auto moduleIt = m_modules.find(pluginId);
            if (moduleIt != m_modules.end())
            {
                UnloadPluginModule(moduleIt->second);
                m_modules.erase(moduleIt);
            }

            Logger::Info("Plugin unloaded: {}", pluginId);
            return true;
        }

        bool PluginManager::ReloadPlugin(const std::wstring& pluginId)
        {
            UnloadPlugin(pluginId);
            
            auto it = m_plugins.find(pluginId);
            if (it != m_plugins.end())
            {
                std::wstring filePath = it->second->GetInfo().filePath;
                return LoadPlugin(filePath);
            }
            
            return false;
        }

        std::vector<PluginInfo> PluginManager::GetPlugins() const
        {
            std::vector<PluginInfo> infos;
            for (const auto& pair : m_plugins)
            {
                infos.push_back(pair.second->GetInfo());
            }
            return infos;
        }

        std::shared_ptr<IPlugin> PluginManager::GetPlugin(const std::wstring& pluginId) const
        {
            auto it = m_plugins.find(pluginId);
            if (it != m_plugins.end())
            {
                return it->second;
            }
            return nullptr;
        }

        std::vector<std::shared_ptr<IPlugin>> PluginManager::GetPluginsByType(PluginType type) const
        {
            std::vector<std::shared_ptr<IPlugin>> plugins;
            for (const auto& pair : m_plugins)
            {
                if (pair.second->GetType() == type)
                {
                    plugins.push_back(pair.second);
                }
            }
            return plugins;
        }

        bool PluginManager::EnablePlugin(const std::wstring& pluginId)
        {
            auto plugin = GetPlugin(pluginId);
            if (!plugin)
                return false;

            plugin->SetEnabled(true);
            Logger::Info("Plugin enabled: {}", pluginId);
            return true;
        }

        bool PluginManager::DisablePlugin(const std::wstring& pluginId)
        {
            auto plugin = GetPlugin(pluginId);
            if (!plugin)
                return false;

            plugin->SetEnabled(false);
            Logger::Info("Plugin disabled: {}", pluginId);
            return true;
        }

        HMODULE PluginManager::LoadPluginModule(const std::wstring& filePath)
        {
            HMODULE hModule = LoadLibraryExW(filePath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
            if (!hModule)
            {
                DWORD error = GetLastError();
                Logger::Error("Failed to load plugin: {}, error: {}", filePath, error);
            }
            return hModule;
        }

        void PluginManager::UnloadPluginModule(HMODULE hModule)
        {
            if (hModule)
            {
                FreeLibrary(hModule);
            }
        }
    }
}