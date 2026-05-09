#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace ShuTongWen
{
    namespace Language
    {
        struct BehaviorRecord
        {
            std::wstring input;
            std::wstring selected;
            uint64_t timestamp;
            std::wstring appName;
            int inputMode;
        };

        struct UserPreference
        {
            std::wstring word;
            double preference;
            uint64_t lastUsed;
            int usageCount;
        };

        class UserBehaviorModel
        {
        public:
            UserBehaviorModel();
            ~UserBehaviorModel();

            bool Initialize(const std::wstring& dbPath);
            void Uninitialize();

            void RecordBehavior(const BehaviorRecord& record);
            UserPreference GetPreference(const std::wstring& word);
            double PredictPreference(const std::wstring& input, const std::wstring& candidate);

            void LoadFromDatabase();
            void SaveToDatabase();

            bool IsInitialized() const { return m_initialized; }

        private:
            bool m_initialized;
            void* m_dbHandle;
            
            std::unordered_map<std::wstring, UserPreference> m_preferences;
            std::vector<BehaviorRecord> m_recentRecords;
        };
    }
}