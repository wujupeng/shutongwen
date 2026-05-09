#include "ime/DictionaryManager.h"
#include "utils/logger.h"
#include "utils/win32_utils.h"
#include "utils/string_utils.h"
#include <sqlite3.h>

namespace ShuTongWen
{
    DictionaryManager::DictionaryManager()
        : m_initialized(false),
          m_dbHandle(nullptr)
    {}

    DictionaryManager& DictionaryManager::Instance()
    {
        static DictionaryManager instance;
        return instance;
    }

    bool DictionaryManager::Initialize(const std::wstring& dbPath)
    {
        if (m_initialized)
            return true;

        Logger::Info("Initializing dictionary manager...");

        std::wstring fullPath = Win32Utils::GetModuleDirectory() + L"\\" + dbPath;
        
        sqlite3* db = nullptr;
        int rc = sqlite3_open16(fullPath.c_str(), &db);
        
        if (rc != SQLITE_OK)
        {
            Logger::Warn("Failed to open database, creating new one");
            
            sqlite3_close(db);
            
            std::wstring dirPath = fullPath.substr(0, fullPath.find_last_of(L'\\'));
            Win32Utils::CreateDirectoryRecursive(dirPath);
            
            rc = sqlite3_open16(fullPath.c_str(), &db);
            if (rc != SQLITE_OK)
            {
                Logger::Error("Failed to create database");
                return false;
            }

            const char* createTable = 
                "CREATE TABLE IF NOT EXISTS words ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "word TEXT NOT NULL UNIQUE,"
                "pinyins TEXT NOT NULL,"
                "frequency INTEGER DEFAULT 1,"
                "category TEXT DEFAULT 'general'"
                ");"
                "CREATE INDEX IF NOT EXISTS idx_word ON words(word);"
                "CREATE INDEX IF NOT EXISTS idx_pinyin ON words(pinyins);";

            char* errMsg;
            rc = sqlite3_exec(db, createTable, nullptr, nullptr, &errMsg);
            if (rc != SQLITE_OK)
            {
                Logger::Error("Failed to create table");
                sqlite3_free(errMsg);
                sqlite3_close(db);
                return false;
            }
        }

        m_dbHandle = db;
        m_initialized = true;
        Logger::Info("Dictionary manager initialized successfully");
        return true;
    }

    void DictionaryManager::Uninitialize()
    {
        if (m_dbHandle)
        {
            sqlite3_close(reinterpret_cast<sqlite3*>(m_dbHandle));
            m_dbHandle = nullptr;
        }
        m_initialized = false;
    }

    std::vector<DictEntry> DictionaryManager::Lookup(const std::wstring& pinyin)
    {
        std::vector<DictEntry> result;
        if (!m_initialized || !m_dbHandle)
            return result;

        sqlite3* db = reinterpret_cast<sqlite3*>(m_dbHandle);
        std::string sql = "SELECT word, pinyins, frequency, category FROM words WHERE pinyins LIKE ? ORDER BY frequency DESC LIMIT 20;";
        
        sqlite3_stmt* stmt;
        std::string pattern = "%" + StringUtils::UTF16ToUTF8(pinyin) + "%";
        
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            return result;

        sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            DictEntry entry;
            entry.word = StringUtils::UTF8ToUTF16(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            
            std::string pinyinsStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            auto pinyins = StringUtils::Split(StringUtils::UTF8ToUTF16(pinyinsStr), L',');
            entry.pinyins = pinyins;
            
            entry.frequency = sqlite3_column_int(stmt, 2);
            entry.category = StringUtils::UTF8ToUTF16(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
            
            result.push_back(entry);
        }

        sqlite3_finalize(stmt);
        return result;
    }

    std::vector<DictEntry> DictionaryManager::LookupMulti(const std::vector<std::wstring>& pinyins)
    {
        std::vector<DictEntry> result;
        if (pinyins.empty())
            return result;

        for (const auto& pinyin : pinyins)
        {
            auto entries = Lookup(pinyin);
            result.insert(result.end(), entries.begin(), entries.end());
        }

        std::sort(result.begin(), result.end(),
            [](const DictEntry& a, const DictEntry& b) {
                return a.frequency > b.frequency;
            });

        return result;
    }

    bool DictionaryManager::AddWord(const DictEntry& entry)
    {
        if (!m_initialized || !m_dbHandle)
            return false;

        sqlite3* db = reinterpret_cast<sqlite3*>(m_dbHandle);
        const char* sql = "INSERT OR REPLACE INTO words (word, pinyins, frequency, category) VALUES (?, ?, ?, ?);";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return false;

        std::wstring pinyinsStr = StringUtils::Join(entry.pinyins, L',');

        sqlite3_bind_text(stmt, 1, StringUtils::UTF16ToUTF8(entry.word).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, StringUtils::UTF16ToUTF8(pinyinsStr).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, entry.frequency);
        sqlite3_bind_text(stmt, 4, StringUtils::UTF16ToUTF8(entry.category).c_str(), -1, SQLITE_STATIC);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return success;
    }

    bool DictionaryManager::UpdateFrequency(const std::wstring& word, int delta)
    {
        if (!m_initialized || !m_dbHandle)
            return false;

        sqlite3* db = reinterpret_cast<sqlite3*>(m_dbHandle);
        const char* sql = "UPDATE words SET frequency = frequency + ? WHERE word = ?;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return false;

        sqlite3_bind_int(stmt, 1, delta);
        sqlite3_bind_text(stmt, 2, StringUtils::UTF16ToUTF8(word).c_str(), -1, SQLITE_STATIC);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return success;
    }

    bool DictionaryManager::RemoveWord(const std::wstring& word)
    {
        if (!m_initialized || !m_dbHandle)
            return false;

        sqlite3* db = reinterpret_cast<sqlite3*>(m_dbHandle);
        const char* sql = "DELETE FROM words WHERE word = ?;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return false;

        sqlite3_bind_text(stmt, 1, StringUtils::UTF16ToUTF8(word).c_str(), -1, SQLITE_STATIC);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return success;
    }

    size_t DictionaryManager::GetWordCount() const
    {
        if (!m_initialized || !m_dbHandle)
            return 0;

        sqlite3* db = reinterpret_cast<sqlite3*>(m_dbHandle);
        const char* sql = "SELECT COUNT(*) FROM words;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return 0;

        size_t count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            count = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return count;
    }
}