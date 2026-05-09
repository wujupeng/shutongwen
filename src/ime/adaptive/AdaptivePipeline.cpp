#include "ime/adaptive/AdaptivePipeline.h"
#include "utils/logger.h"
#include <windows.h>
#include <psapi.h>

namespace ShuTongWen
{
    namespace Adaptive
    {
        FeatureSwitchManager::FeatureSwitchManager()
            : m_currentMode(OperationMode::NORMAL),
              m_detectionInterval(1)
        {}

        FeatureSwitchManager& FeatureSwitchManager::Instance()
        {
            static FeatureSwitchManager instance;
            return instance;
        }

        bool FeatureSwitchManager::Initialize()
        {
            Logger::Info("Initializing FeatureSwitchManager...");

            m_features[FeatureSwitch::AI_RERANKER] = true;
            m_features[FeatureSwitch::OCR] = true;
            m_features[FeatureSwitch::PLUGINS] = true;
            m_features[FeatureSwitch::LM_FULL] = true;
            m_features[FeatureSwitch::LM_LIGHT] = false;
            m_features[FeatureSwitch::CONTEXT_ANALYZER] = true;
            m_features[FeatureSwitch::EMOJI_SUGGESTION] = true;
            m_features[FeatureSwitch::CLIPBOARD_HISTORY] = true;

            ModeConfiguration normalConfig;
            normalConfig.mode = OperationMode::NORMAL;
            normalConfig.name = L"Normal";
            normalConfig.latencyBudgetMs = 50;
            normalConfig.aggressiveCaching = false;
            m_configurations[OperationMode::NORMAL] = normalConfig;

            ModeConfiguration cpuPressureConfig;
            cpuPressureConfig.mode = OperationMode::CPU_PRESSURE;
            cpuPressureConfig.name = L"CPU Pressure";
            cpuPressureConfig.disabledFeatures = {
                FeatureSwitch::AI_RERANKER,
                FeatureSwitch::OCR,
                FeatureSwitch::PLUGINS,
                FeatureSwitch::LM_FULL
            };
            cpuPressureConfig.enabledFeatures = {
                FeatureSwitch::LM_LIGHT
            };
            cpuPressureConfig.latencyBudgetMs = 10;
            cpuPressureConfig.aggressiveCaching = true;
            m_configurations[OperationMode::CPU_PRESSURE] = cpuPressureConfig;

            ModeConfiguration gameConfig;
            gameConfig.mode = OperationMode::GAME;
            gameConfig.name = L"Game";
            gameConfig.disabledFeatures = {
                FeatureSwitch::AI_RERANKER,
                FeatureSwitch::OCR,
                FeatureSwitch::PLUGINS,
                FeatureSwitch::LM_FULL,
                FeatureSwitch::CONTEXT_ANALYZER,
                FeatureSwitch::EMOJI_SUGGESTION,
                FeatureSwitch::CLIPBOARD_HISTORY
            };
            gameConfig.enabledFeatures = {
                FeatureSwitch::LM_LIGHT
            };
            gameConfig.latencyBudgetMs = 5;
            gameConfig.aggressiveCaching = true;
            m_configurations[OperationMode::GAME] = gameConfig;

            ModeConfiguration fastInputConfig;
            fastInputConfig.mode = OperationMode::FAST_INPUT;
            fastInputConfig.name = L"Fast Input";
            fastInputConfig.disabledFeatures = {
                FeatureSwitch::AI_RERANKER,
                FeatureSwitch::LM_FULL
            };
            fastInputConfig.enabledFeatures = {
                FeatureSwitch::LM_LIGHT
            };
            fastInputConfig.latencyBudgetMs = 8;
            fastInputConfig.aggressiveCaching = true;
            m_configurations[OperationMode::FAST_INPUT] = fastInputConfig;

            ModeConfiguration batteryConfig;
            batteryConfig.mode = OperationMode::BATTERY_SAVER;
            batteryConfig.name = L"Battery Saver";
            batteryConfig.disabledFeatures = {
                FeatureSwitch::AI_RERANKER,
                FeatureSwitch::OCR,
                FeatureSwitch::PLUGINS,
                FeatureSwitch::LM_FULL
            };
            batteryConfig.enabledFeatures = {
                FeatureSwitch::LM_LIGHT
            };
            batteryConfig.latencyBudgetMs = 20;
            batteryConfig.aggressiveCaching = true;
            m_configurations[OperationMode::BATTERY_SAVER] = batteryConfig;

            Logger::Info("FeatureSwitchManager initialized");
            return true;
        }

        void FeatureSwitchManager::Uninitialize()
        {
            m_features.clear();
            m_configurations.clear();
        }

        void FeatureSwitchManager::SetMode(OperationMode mode)
        {
            if (m_currentMode == mode)
                return;

            Logger::Info("Switching operation mode: {} -> {}", 
                static_cast<int>(m_currentMode), static_cast<int>(mode));

            m_currentMode = mode;

            auto it = m_configurations.find(mode);
            if (it != m_configurations.end())
            {
                ApplyModeConfiguration(it->second);
            }

            if (onModeChanged)
            {
                onModeChanged(mode);
            }
        }

        bool FeatureSwitchManager::IsFeatureEnabled(FeatureSwitch feature) const
        {
            auto it = m_features.find(feature);
            return it != m_features.end() && it->second;
        }

        void FeatureSwitchManager::EnableFeature(FeatureSwitch feature)
        {
            m_features[feature] = true;
        }

        void FeatureSwitchManager::DisableFeature(FeatureSwitch feature)
        {
            m_features[feature] = false;
        }

        void FeatureSwitchManager::UpdateMetrics(const SystemMetrics& metrics)
        {
            m_metrics = metrics;
            DetectSystemState();
        }

        void FeatureSwitchManager::LoadConfiguration(const std::wstring& path)
        {
        }

        void FeatureSwitchManager::SaveConfiguration(const std::wstring& path)
        {
        }

        void FeatureSwitchManager::ApplyModeConfiguration(const ModeConfiguration& config)
        {
            for (const auto& feature : config.disabledFeatures)
            {
                m_features[feature] = false;
                Logger::Debug("Feature disabled: {}", static_cast<int>(feature));
            }

            for (const auto& feature : config.enabledFeatures)
            {
                m_features[feature] = true;
                Logger::Debug("Feature enabled: {}", static_cast<int>(feature));
            }
        }

        void FeatureSwitchManager::DetectSystemState()
        {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = now - m_lastDetection;

            if (elapsed < m_detectionInterval)
                return;

            m_lastDetection = now;

            bool shouldSwitch = false;
            OperationMode newMode = m_currentMode;

            if (m_metrics.cpuUsage > 80.0f)
            {
                newMode = OperationMode::CPU_PRESSURE;
                shouldSwitch = true;
            }
            else if (m_metrics.isGameRunning)
            {
                newMode = OperationMode::GAME;
                shouldSwitch = true;
            }
            else if (m_metrics.inputSpeed > 10)
            {
                newMode = OperationMode::FAST_INPUT;
                shouldSwitch = true;
            }
            else if (m_metrics.isBatteryPower && m_metrics.batteryLevel < 20.0f)
            {
                newMode = OperationMode::BATTERY_SAVER;
                shouldSwitch = true;
            }
            else
            {
                newMode = OperationMode::NORMAL;
                shouldSwitch = true;
            }

            if (shouldSwitch && newMode != m_currentMode)
            {
                SetMode(newMode);
            }
        }
    }
}