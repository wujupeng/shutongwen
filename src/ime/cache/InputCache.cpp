#include "ime/cache/InputCache.h"
#include "utils/logger.h"

namespace ShuTongWen
{
    namespace Cache
    {
        CandidateCache::CandidateCache()
            : m_maxSize(1000), m_hits(0), m_misses(0), m_expireMinutes(30)
        {}

        CandidateCache& CandidateCache::Instance()
        {
            static CandidateCache instance;
            return instance;
        }

        bool CandidateCache::Initialize()
        {
            Logger::Info("Initializing CandidateCache...");
            return true;
        }

        void CandidateCache::Uninitialize()
        {
            Clear();
        }

        bool CandidateCache::Get(const std::wstring& key, std::vector<CandidateItem>& candidates)
        {
            auto it = m_cache.find(key);
            if (it != m_cache.end())
            {
                auto now = std::chrono::system_clock::now();
                auto age = std::chrono::duration_cast<std::chrono::minutes>(now - it->second.timestamp);
                
                if (age < m_expireMinutes)
                {
                    it->second.accessCount++;
                    candidates = it->second.candidates;
                    m_hits++;
                    return true;
                }
                else
                {
                    m_cache.erase(it);
                }
            }
            
            m_misses++;
            return false;
        }

        bool CandidateCache::Set(const std::wstring& key, const std::vector<CandidateItem>& candidates)
        {
            if (m_cache.size() >= m_maxSize)
            {
                Trim();
            }

            CandidateCacheEntry entry;
            entry.key = key;
            entry.candidates = candidates;
            entry.timestamp = std::chrono::system_clock::now();
            entry.accessCount = 1;

            m_cache[key] = entry;
            return true;
        }

        void CandidateCache::Remove(const std::wstring& key)
        {
            m_cache.erase(key);
        }

        void CandidateCache::Clear()
        {
            m_cache.clear();
            m_hits = 0;
            m_misses = 0;
        }

        void CandidateCache::Trim()
        {
            if (m_cache.empty())
                return;

            size_t targetSize = m_maxSize * 0.7;
            
            std::vector<std::pair<std::wstring, CandidateCacheEntry*>> entries;
            for (auto& pair : m_cache)
            {
                entries.emplace_back(pair.first, &pair.second);
            }

            std::sort(entries.begin(), entries.end(),
                [](const auto& a, const auto& b) {
                    return a.second->accessCount < b.second->accessCount;
                });

            for (size_t i = 0; i < entries.size() - targetSize; ++i)
            {
                m_cache.erase(entries[i].first);
            }
        }

        size_t CandidateCache::GetSize() const
        {
            return m_cache.size();
        }

        size_t CandidateCache::GetMaxSize() const
        {
            return m_maxSize;
        }

        void CandidateCache::SetMaxSize(size_t maxSize)
        {
            m_maxSize = maxSize;
        }

        double CandidateCache::GetHitRate() const
        {
            size_t total = m_hits + m_misses;
            return total > 0 ? static_cast<double>(m_hits) / total : 0.0;
        }

        ContextCache::ContextCache()
        {}

        ContextCache& ContextCache::Instance()
        {
            static ContextCache instance;
            return instance;
        }

        bool ContextCache::Initialize()
        {
            Logger::Info("Initializing ContextCache...");
            return true;
        }

        void ContextCache::Uninitialize()
        {
            Clear();
        }

        bool ContextCache::Get(const std::wstring& contextKey, std::unordered_map<std::wstring, float>& scores)
        {
            auto it = m_cache.find(contextKey);
            if (it != m_cache.end())
            {
                scores = it->second.scores;
                return true;
            }
            return false;
        }

        bool ContextCache::Set(const std::wstring& contextKey, const std::vector<std::wstring>& contextWords)
        {
            ContextCacheEntry entry;
            entry.contextKey = contextKey;
            entry.contextWords = contextWords;
            entry.timestamp = std::chrono::system_clock::now();

            float baseScore = 1.0f;
            for (const auto& word : contextWords)
            {
                entry.scores[word] = baseScore;
                baseScore *= 0.7f;
            }

            m_cache[contextKey] = entry;
            return true;
        }

        void ContextCache::UpdateScores(const std::wstring& contextKey, const std::unordered_map<std::wstring, float>& scores)
        {
            auto it = m_cache.find(contextKey);
            if (it != m_cache.end())
            {
                for (const auto& pair : scores)
                {
                    it->second.scores[pair.first] = pair.second;
                }
                it->second.timestamp = std::chrono::system_clock::now();
            }
        }

        void ContextCache::Remove(const std::wstring& contextKey)
        {
            m_cache.erase(contextKey);
        }

        void ContextCache::Clear()
        {
            m_cache.clear();
        }

        SessionCache::SessionCache()
            : m_expireHours(24), m_maxInputsPerSession(50)
        {}

        SessionCache& SessionCache::Instance()
        {
            static SessionCache instance;
            return instance;
        }

        bool SessionCache::Initialize()
        {
            Logger::Info("Initializing SessionCache...");
            return true;
        }

        void SessionCache::Uninitialize()
        {
            Clear();
        }

        bool SessionCache::Get(const std::wstring& sessionId, SessionCacheEntry& entry)
        {
            auto it = m_cache.find(sessionId);
            if (it != m_cache.end())
            {
                auto now = std::chrono::system_clock::now();
                auto age = std::chrono::duration_cast<std::chrono::hours>(now - it->second.lastAccess);
                
                if (age < m_expireHours)
                {
                    entry = it->second;
                    return true;
                }
                else
                {
                    m_cache.erase(it);
                }
            }
            return false;
        }

        bool SessionCache::Set(const std::wstring& sessionId, const SessionCacheEntry& entry)
        {
            SessionCacheEntry newEntry = entry;
            newEntry.lastAccess = std::chrono::system_clock::now();
            m_cache[sessionId] = newEntry;
            return true;
        }

        void SessionCache::AddInput(const std::wstring& sessionId, const std::wstring& input)
        {
            SessionCacheEntry entry;
            if (!Get(sessionId, entry))
            {
                entry.sessionId = sessionId;
            }

            auto it = std::find(entry.recentInputs.begin(), entry.recentInputs.end(), input);
            if (it != entry.recentInputs.end())
            {
                entry.recentInputs.erase(it);
            }
            
            entry.recentInputs.insert(entry.recentInputs.begin(), input);
            
            if (entry.recentInputs.size() > m_maxInputsPerSession)
            {
                entry.recentInputs.pop_back();
            }

            Set(sessionId, entry);
        }

        void SessionCache::AddSelection(const std::wstring& sessionId, const std::wstring& selection)
        {
            SessionCacheEntry entry;
            if (!Get(sessionId, entry))
            {
                entry.sessionId = sessionId;
            }

            auto it = std::find(entry.recentSelections.begin(), entry.recentSelections.end(), selection);
            if (it != entry.recentSelections.end())
            {
                entry.recentSelections.erase(it);
            }
            
            entry.recentSelections.insert(entry.recentSelections.begin(), selection);
            
            if (entry.recentSelections.size() > m_maxInputsPerSession)
            {
                entry.recentSelections.pop_back();
            }

            Set(sessionId, entry);
        }

        void SessionCache::Remove(const std::wstring& sessionId)
        {
            m_cache.erase(sessionId);
        }

        void SessionCache::Clear()
        {
            m_cache.clear();
        }

        void SessionCache::CleanupExpired()
        {
            auto now = std::chrono::system_clock::now();
            
            auto it = m_cache.begin();
            while (it != m_cache.end())
            {
                auto age = std::chrono::duration_cast<std::chrono::hours>(now - it->second.lastAccess);
                if (age >= m_expireHours)
                {
                    it = m_cache.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }
}