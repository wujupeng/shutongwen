#pragma once

#include <vector>
#include <string>
#include <memory>

namespace ShuTongWen
{
    struct DictEntry
    {
        std::wstring word;
        std::vector<std::wstring> pinyins;
        int frequency;
        std::wstring category;
    };

    class DictionaryManager
    {
    public:
        ~DictionaryManager() = default;

        static DictionaryManager& Instance();

        bool Initialize(const std::wstring& dbPath);
        void Uninitialize();

        std::vector<DictEntry> Lookup(const std::wstring& pinyin);
        std::vector<DictEntry> LookupMulti(const std::vector<std::wstring>& pinyins);

        bool AddWord(const DictEntry& entry);
        bool UpdateFrequency(const std::wstring& word, int delta);
        bool RemoveWord(const std::wstring& word);

        size_t GetWordCount() const;
        bool IsInitialized() const { return m_initialized; }

    private:
        DictionaryManager();
        DictionaryManager(const DictionaryManager&) = delete;
        DictionaryManager& operator=(const DictionaryManager&) = delete;

        bool m_initialized;
        void* m_dbHandle;
    };
}