#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace ShuTongWen
{
    struct EmojiItem
    {
        std::wstring emoji;
        std::wstring text;
        std::vector<std::wstring> keywords;
        int frequency;

        EmojiItem() : frequency(0) {}
        EmojiItem(const std::wstring& e, const std::wstring& t, const std::vector<std::wstring>& k, int f = 0)
            : emoji(e), text(t), keywords(k), frequency(f) {}
    };

    class EmojiAssociator
    {
    public:
        ~EmojiAssociator() = default;

        static EmojiAssociator& Instance();

        bool Initialize();
        void Uninitialize();

        std::vector<EmojiItem> Query(const std::wstring& input, size_t limit = 10);
        bool HasEmoji(const std::wstring& input);

        void AddEmoji(const EmojiItem& item);
        void RemoveEmoji(const std::wstring& emoji);
        void UpdateFrequency(const std::wstring& emoji, int delta);

        size_t GetEmojiCount() const;
        bool IsInitialized() const { return m_initialized; }

    private:
        EmojiAssociator();
        EmojiAssociator(const EmojiAssociator&) = delete;
        EmojiAssociator& operator=(const EmojiAssociator&) = delete;

        void LoadDefaultEmojis();

        bool m_initialized;
        std::unordered_map<std::wstring, std::vector<EmojiItem>> m_keywordMap;
        std::unordered_map<std::wstring, EmojiItem> m_emojiMap;
    };
}