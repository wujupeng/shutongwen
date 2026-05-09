#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <functional>

namespace ShuTongWen
{
    namespace Adaptive
    {
        enum class OperationMode
        {
            NORMAL,
            CPU_PRESSURE,
            GAME,
            FAST_INPUT,
            BATTERY_SAVER
        };

        enum class FeatureSwitch
        {
            AI_RERANKER,
            OCR,
            PLUGINS,
            LM_FULL,
            LM_LIGHT,
            CONTEXT_ANALYZER,
            EMOJI_SUGGESTION,
            CLIPBOARD_HISTORY
        };

        struct SystemMetrics
        {
            float cpuUsage;
            float memoryUsage;
            float batteryLevel;
            bool isBatteryPower;
            bool isGameRunning;
            uint32_t inputSpeed;
            uint32_t recentInputCount;
            std::chrono::milliseconds avgLatency;
        };

        struct ModeConfiguration
        {
            OperationMode mode;
            std::wstring name;
            std::vector<FeatureSwitch> enabledFeatures;
            std::vector<FeatureSwitch> disabledFeatures;
            int latencyBudgetMs;
            bool aggressiveCaching;

            ModeConfiguration() : latencyBudgetMs(50), aggressiveCaching(false) {}
        };

        class FeatureSwitchManager
        {
        public:
            ~FeatureSwitchManager() = default;

            static FeatureSwitchManager& Instance();

            bool Initialize();
            void Uninitialize();

            void SetMode(OperationMode mode);
            OperationMode GetMode() const { return m_currentMode; }

            bool IsFeatureEnabled(FeatureSwitch feature) const;
            void EnableFeature(FeatureSwitch feature);
            void DisableFeature(FeatureSwitch feature);

            void UpdateMetrics(const SystemMetrics& metrics);
            SystemMetrics GetCurrentMetrics() const { return m_metrics; }

            void LoadConfiguration(const std::wstring& path);
            void SaveConfiguration(const std::wstring& path);

            std::function<void(OperationMode)> onModeChanged;

        private:
            FeatureSwitchManager();

            void ApplyModeConfiguration(const ModeConfiguration& config);
            void DetectSystemState();

            OperationMode m_currentMode;
            std::unordered_map<FeatureSwitch, bool> m_features;
            std::unordered_map<OperationMode, ModeConfiguration> m_configurations;
            SystemMetrics m_metrics;

            std::chrono::steady_clock::time_point m_lastDetection;
            const std::chrono::seconds m_detectionInterval;
        };
    }
}