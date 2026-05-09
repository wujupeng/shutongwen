#include "ime/language/CandidateRanker.h"
#include "utils/logger.h"
#include <algorithm>

namespace ShuTongWen
{
    namespace Language
    {
        bool CandidateRanker::Initialize()
        {
            m_maxRecentInputs = 100;
            m_weights = RankWeights();
            return true;
        }

        void CandidateRanker::Uninitialize()
        {
            m_userFrequency.clear();
            m_userLastUsed.clear();
            m_contextScores.clear();
            m_appScores.clear();
            m_recentInputs.clear();
        }

        std::vector<Candidate> CandidateRanker::Rank(const std::vector<Candidate>& candidates,
                                                     const std::vector<std::wstring>& context,
                                                     const std::wstring& input,
                                                     uint64_t timestamp)
        {
            std::vector<Candidate> ranked = candidates;

            for (auto& candidate : ranked)
            {
                CandidateFeatures features = ExtractFeatures(candidate, context);
                candidate.score = CalculateScore(candidate, features);
            }

            std::sort(ranked.begin(), ranked.end(),
                [](const Candidate& a, const Candidate& b) {
                    return a.score > b.score;
                });

            return ranked;
        }

        void CandidateRanker::UpdateUserBehavior(const std::wstring& text, uint64_t timestamp)
        {
            m_userFrequency[text]++;
            m_userLastUsed[text] = timestamp;

            auto it = std::find(m_recentInputs.begin(), m_recentInputs.end(), text);
            if (it != m_recentInputs.end())
            {
                m_recentInputs.erase(it);
            }
            
            m_recentInputs.insert(m_recentInputs.begin(), text);
            
            if (m_recentInputs.size() > m_maxRecentInputs)
            {
                m_recentInputs.pop_back();
            }

            OnlineLearn(text, timestamp);
        }

        void CandidateRanker::UpdateContextScore(const std::vector<std::wstring>& context)
        {
            m_contextScores.clear();
            for (size_t i = 0; i < context.size(); ++i)
            {
                float score = 1.0f - static_cast<float>(i) / context.size();
                m_contextScores[context[i]] = score;
            }
        }

        void CandidateRanker::UpdateAppContext(const std::wstring& appName)
        {
            m_currentApp = appName;
        }

        void CandidateRanker::SetWeights(const RankWeights& weights)
        {
            m_weights = weights;
        }

        RankWeights CandidateRanker::GetWeights() const
        {
            return m_weights;
        }

        void CandidateRanker::LoadUserProfile(const std::wstring& profilePath)
        {
        }

        void CandidateRanker::SaveUserProfile(const std::wstring& profilePath)
        {
        }

        CandidateFeatures CandidateRanker::ExtractFeatures(const Candidate& candidate,
                                                          const std::vector<std::wstring>& context)
        {
            CandidateFeatures features;

            features.frequency_score = static_cast<float>(candidate.frequency) / 1000.0f;

            uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            
            auto it = m_userLastUsed.find(candidate.text);
            if (it != m_userLastUsed.end())
            {
                uint64_t age = now - it->second;
                features.recency_score = std::max(0.0f, 1.0f - static_cast<float>(age) / 86400000.0f);
            }
            else
            {
                features.recency_score = 0.5f;
            }

            features.context_score = 0.0f;
            for (const auto& word : context)
            {
                auto ctxIt = m_contextScores.find(word);
                if (ctxIt != m_contextScores.end())
                {
                    features.context_score += ctxIt->second * 0.3f;
                }
            }
            features.context_score = std::min(1.0f, features.context_score);

            auto appIt = m_appScores.find(m_currentApp);
            features.app_context_score = appIt != m_appScores.end() ? appIt->second : 0.5f;

            features.semantic_score = 0.5f;

            features.typo_score = 1.0f;

            return features;
        }

        float CandidateRanker::CalculateScore(const Candidate& candidate,
                                              const CandidateFeatures& features)
        {
            float score = 0.0f;
            
            score += features.frequency_score * m_weights.frequency_weight;
            score += features.context_score * m_weights.context_weight;
            score += features.recency_score * m_weights.user_behavior_weight;
            score += features.app_context_score * m_weights.app_context_weight;
            score += features.semantic_score * m_weights.semantic_weight;

            score += static_cast<float>(candidate.score) * 0.001f;

            return score;
        }

        void CandidateRanker::OnlineLearn(const std::wstring& selectedText, uint64_t timestamp)
        {
            float learningRate = 0.01f;

            auto freqIt = m_userFrequency.find(selectedText);
            if (freqIt != m_userFrequency.end())
            {
                float freqBoost = learningRate * static_cast<float>(freqIt->second);
                m_weights.frequency_weight = std::min(0.5f, m_weights.frequency_weight + freqBoost * 0.001f);
            }

            for (const auto& input : m_recentInputs)
            {
                auto ctxIt = m_contextScores.find(input);
                if (ctxIt != m_contextScores.end())
                {
                    ctxIt->second = std::min(1.0f, ctxIt->second + learningRate);
                }
            }
        }
    }
}