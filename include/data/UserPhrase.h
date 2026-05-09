#pragma once

#include <string>
#include <vector>
#include <memory>

namespace ShuTongWen
{
    struct UserPhrase
    {
        std::wstring text;
        uint64_t freq;
        uint64_t last_used;
        double language_score;

        UserPhrase() : freq(0), last_used(0), language_score(0.0) {}

        UserPhrase(const std::wstring& t, uint64_t f, uint64_t lu, double ls)
            : text(t), freq(f), last_used(lu), language_score(ls) {}

        double CalculateScore(double decay_factor = 0.99) const;
    };

    class UserPhraseManager
    {
    public:
        ~UserPhraseManager() = default;

        static UserPhraseManager& Instance();

        bool Initialize(const std::wstring& dbPath);
        void Uninitialize();

        bool AddPhrase(const std::wstring& text);
        bool RemovePhrase(const std::wstring& text);
        bool UpdatePhrase(const UserPhrase& phrase);
        
        std::vector<UserPhrase> QueryPhrases(const std::wstring& prefix, size_t limit = 10);
        std::vector<UserPhrase> GetTopPhrases(size_t limit = 100);
        
        bool GetPhrase(const std::wstring& text, UserPhrase& phrase);
        bool HasPhrase(const std::wstring& text);

        size_t GetPhraseCount() const;
        bool IsInitialized() const { return m_initialized; }

        void GarbageCollect(uint64_t min_timestamp);

    private:
        UserPhraseManager();
        UserPhraseManager(const UserPhraseManager&) = delete;
        UserPhraseManager& operator=(const UserPhraseManager&) = delete;

        bool m_initialized;
        void* m_dbHandle;
    };
}