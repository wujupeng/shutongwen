#include "ime/language/UserBehaviorModel.h"
#include "utils/logger.h"
#include "data/UserPhrase.h"

namespace ShuTongWen
{
    namespace Language
    {
        UserBehaviorModel::UserBehaviorModel()
            : m_initialized(false),
              m_dbHandle(nullptr)
        {}

        UserBehaviorModel::~UserBehaviorModel()
        {
            Uninitialize();
        }

        bool UserBehaviorModel::Initialize(const std::wstring& dbPath)
        {
            Logger::Info("Initializing UserBehaviorModel...");
            
            m_preferences.clear();
            m_recentRecords.clear();
            
            LoadFromDatabase();
            
            m_initialized = true;
            Logger::Info("UserBehaviorModel initialized successfully");
            return true;
        }

        void UserBehaviorModel::Uninitialize()
        {
            SaveToDatabase();
            m_preferences.clear();
            m_recentRecords.clear();
            m_initialized = false;
        }

        void UserBehaviorModel::RecordBehavior(const BehaviorRecord& record)
        {
            m_recentRecords.push_back(record);
            
            if (m_recentRecords.size() > 1000)
            {
                m_recentRecords.erase(m_recentRecords.begin());
            }

            auto it = m_preferences.find(record.selected);
            if (it != m_preferences.end())
            {
                it->second.usageCount++;
                it->second.lastUsed = record.timestamp;
                it->second.preference = std::min(1.0, it->second.preference + 0.01);
            }
            else
            {
                UserPreference pref;
                pref.word = record.selected;
                pref.preference = 0.5;
                pref.lastUsed = record.timestamp;
                pref.usageCount = 1;
                m_preferences[record.selected] = pref;
            }
        }

        UserPreference UserBehaviorModel::GetPreference(const std::wstring& word)
        {
            auto it = m_preferences.find(word);
            if (it != m_preferences.end())
                return it->second;

            UserPreference empty;
            empty.word = word;
            return empty;
        }

        double UserBehaviorModel::PredictPreference(const std::wstring& input, const std::wstring& candidate)
        {
            auto pref = GetPreference(candidate);
            
            double inputSimilarity = 0.0;
            if (!input.empty() && !candidate.empty())
            {
                size_t minLen = std::min(input.length(), candidate.length());
                size_t matches = 0;
                
                for (size_t i = 0; i < minLen; ++i)
                {
                    if (input[i] == candidate[i])
                        matches++;
                }
                
                inputSimilarity = static_cast<double>(matches) / static_cast<double>(minLen);
            }

            double recencyBonus = 0.0;
            if (pref.lastUsed > 0)
            {
                uint64_t now = GetTickCount64();
                uint64_t diff = now - pref.lastUsed;
                
                if (diff < 3600000)
                    recencyBonus = 0.3;
                else if (diff < 86400000)
                    recencyBonus = 0.1;
            }

            return pref.preference * 0.5 + inputSimilarity * 0.3 + recencyBonus;
        }

        void UserBehaviorModel::LoadFromDatabase()
        {
            UserPhraseManager& phraseManager = UserPhraseManager::Instance();
            
            std::vector<UserPhrase> phrases = phraseManager.GetAllPhrases();
            for (const UserPhrase& phrase : phrases)
            {
                UserPreference pref;
                pref.word = phrase.text;
                pref.preference = std::min(1.0, static_cast<double>(phrase.freq) / 100.0);
                pref.lastUsed = phrase.last_used;
                pref.usageCount = static_cast<int>(phrase.freq);
                m_preferences[phrase.text] = pref;
            }
        }

        void UserBehaviorModel::SaveToDatabase()
        {
            UserPhraseManager& phraseManager = UserPhraseManager::Instance();
            
            for (const auto& pair : m_preferences)
            {
                const UserPreference& pref = pair.second;
                phraseManager.AddPhrase(pref.word);
            }
        }
    }
}