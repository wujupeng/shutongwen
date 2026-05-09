#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <chrono>

namespace ShuTongWen
{
    namespace Language
    {
        struct CandidateFeatures
        {
            float frequency_score;
            float recency_score;
            float context_score;
            float app_context_score;
            float semantic_score;
            float typo_score;

            CandidateFeatures() 
                : frequency_score(0.0f), recency_score(0.0f), context_score(0.0f),
                  app_context_score(0.0f), semantic_score(0.0f), typo_score(0.0f) {}
        };

        struct RankWeights
        {
            float frequency_weight;
            float context_weight;
            float user_behavior_weight;
            float app_context_weight;
            float semantic_weight;

            RankWeights() 
                : frequency_weight(0.35f), context_weight(0.25f), 
                  user_behavior_weight(0.20f), app_context_weight(0.10f), 
                  semantic_weight(0.10f) {}
        };

        class CandidateRanker
        {
        public:
            ~CandidateRanker() = default;

            bool Initialize();
            void Uninitialize();

            std::vector<Candidate> Rank(const std::vector<Candidate>& candidates,
                                        const std::vector<std::wstring>& context,
                                        const std::wstring& input,
                                        uint64_t timestamp);

            void UpdateUserBehavior(const std::wstring& text, uint64_t timestamp);
            void UpdateContextScore(const std::vector<std::wstring>& context);
            void UpdateAppContext(const std::wstring& appName);

            void SetWeights(const RankWeights& weights);
            RankWeights GetWeights() const;

            void LoadUserProfile(const std::wstring& profilePath);
            void SaveUserProfile(const std::wstring& profilePath);

        private:
            CandidateFeatures ExtractFeatures(const Candidate& candidate, 
                                            const std::vector<std::wstring>& context);
            
            float CalculateScore(const Candidate& candidate, 
                               const CandidateFeatures& features);

            void OnlineLearn(const std::wstring& selectedText, uint64_t timestamp);

            RankWeights m_weights;
            
            std::unordered_map<std::wstring, uint64_t> m_userFrequency;
            std::unordered_map<std::wstring, uint64_t> m_userLastUsed;
            std::unordered_map<std::wstring, float> m_contextScores;
            std::unordered_map<std::wstring, float> m_appScores;
            
            std::vector<std::wstring> m_recentInputs;
            const size_t m_maxRecentInputs;
            
            std::wstring m_currentApp;
        };
    }
}