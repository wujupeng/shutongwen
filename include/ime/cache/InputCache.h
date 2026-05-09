#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <chrono>

namespace ShuTongWen
{
    namespace Cache
    {
        struct CandidateCacheEntry
        {
            std::wstring key;
            std::vector<CandidateItem> candidates;
            std::chrono::system_clock::time_point timestamp;
            size_t accessCount;

            CandidateCacheEntry() : accessCount(0) {}
        };

        struct ContextCacheEntry
        {
            std::wstring contextKey;
            std::vector<std::wstring> contextWords;
            std::unordered_map<std::wstring, float> scores;
            std::chrono::system_clock::time_point timestamp;
        };

        struct SessionCacheEntry
        {
            std::wstring sessionId;
            std::vector<std::wstring> recentInputs;
            std::vector<std::wstring> recentSelections;
            std::chrono::system_clock::time_point lastAccess;
        };

        class CandidateCache
        {
        public:
            ~CandidateCache() = default;

            static CandidateCache& Instance();

            bool Initialize();
            void Uninitialize();

            bool Get(const std::wstring& key, std::vector<CandidateItem>& candidates);
            bool Set(const std::wstring& key, const std::vector<CandidateItem>& candidates);
            
            void Remove(const std::wstring& key);
            void Clear();
            void Trim();

            size_t GetSize() const;
            size_t GetMaxSize() const;
            void SetMaxSize(size_t maxSize);

            double GetHitRate() const;

        private:
            CandidateCache();

            std::unordered_map<std::wstring, CandidateCacheEntry> m_cache;
            size_t m_maxSize;
            size_t m_hits;
            size_t m_misses;
            std::chrono::minutes m_expireMinutes;
        };

        class ContextCache
        {
        public:
            ~ContextCache() = default;

            static ContextCache& Instance();

            bool Initialize();
            void Uninitialize();

            bool Get(const std::wstring& contextKey, std::unordered_map<std::wstring, float>& scores);
            bool Set(const std::wstring& contextKey, const std::vector<std::wstring>& contextWords);
            
            void UpdateScores(const std::wstring& contextKey, const std::unordered_map<std::wstring, float>& scores);
            
            void Remove(const std::wstring& contextKey);
            void Clear();

        private:
            ContextCache();

            std::unordered_map<std::wstring, ContextCacheEntry> m_cache;
        };

        class SessionCache
        {
        public:
            ~SessionCache() = default;

            static SessionCache& Instance();

            bool Initialize();
            void Uninitialize();

            bool Get(const std::wstring& sessionId, SessionCacheEntry& entry);
            bool Set(const std::wstring& sessionId, const SessionCacheEntry& entry);
            
            void AddInput(const std::wstring& sessionId, const std::wstring& input);
            void AddSelection(const std::wstring& sessionId, const std::wstring& selection);
            
            void Remove(const std::wstring& sessionId);
            void Clear();
            void CleanupExpired();

        private:
            SessionCache();

            std::unordered_map<std::wstring, SessionCacheEntry> m_cache;
            std::chrono::hours m_expireHours;
            const size_t m_maxInputsPerSession;
        };
    }
}