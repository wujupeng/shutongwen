#include "data/UserPhrase.h"
#include "utils/logger.h"
#include "utils/win32_utils.h"
#include <sqlite3.h>

namespace ShuTongWen
{
    double UserPhrase::CalculateScore(double decay_factor) const
    {
        uint64_t now = GetTickCount64();
        uint64_t hours_since_used = (now - last_used) / 3600000;
        
        double time_decay = pow(decay_factor, static_cast<double>(hours_since_used));
        double freq_score = log(static_cast<double>(freq) + 1);
        
        return (freq_score * time_decay) + language_score;
    }

    UserPhraseManager::UserPhraseManager()
        : m_initialized(false),
          m_dbHandle(nullptr)
    {}

    UserPhraseManager& UserPhraseManager::Instance()
    {
        static UserPhraseManager instance;
        return instance;
    }

    bool UserPhraseManager::Initialize(const std::wstring& dbPath)
    {
        if (m_initialized)
            return true;

        Logger::Info("Initializing UserPhraseManager...");

        std::wstring fullPath = Win32Utils::GetLocalAppDataPath() + L"\\ShuTongWen\\data\\" + dbPath;
        
        sqlite3* db = nullptr;
        int rc = sqlite3_open16(fullPath.c_str(), &db);
        
        if (rc != SQLITE_OK)
        {
            Logger::Warn("Failed to open user phrase database, creating new one");
            
            sqlite3_close(db);
            
            std::wstring dirPath = fullPath.substr(0, fullPath.find_last_of(L'\\'));
            Win32Utils::CreateDirectoryRecursive(dirPath);
            
            rc = sqlite3_open16(fullPath.c_str(), &db);
            if (rc != SQLITE_OK)
            {
                Logger::Error("Failed to create user phrase database");
                return false;
            }

            const char* createTable = 
                "CREATE TABLE IF NOT EXISTS user_phrases ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "text TEXT NOT NULL UNIQUE,"
                "freq INTEGER DEFAULT 1,"
                "last_used INTEGER DEFAULT 0,"
                "language_score REAL DEFAULT 0.0"
                ");"
                "CREATE INDEX IF NOT EXISTS idx_text ON user_phrases(text);"
                "CREATE INDEX IF NOT EXISTS idx_freq ON user_phrases(freq DESC);";

            char* errMsg;
            rc = sqlite3_exec(db, createTable, nullptr, nullptr, &errMsg);
            if (rc != SQLITE_OK)
            {
                Logger::Error("Failed to create user phrases table: {}", errMsg);
                sqlite3_free(errMsg);
                sqlite3_close(db);
                return false;
            }
        }

        m_dbHandle = db;
        m_initialized = true;
        Logger::Info("UserPhraseManager initialized successfully");
        return true;
    }

    void UserPhraseManager::Uninitialize()
    {
        if (m_dbHandle)
        {
            sqlite3_close(reinterpret_cast<sqlite3*>(m_dbHandle));
            m_dbHandle = nullptr;
        }
        m_initialized = false;
    }

    bool UserPhraseManager::AddPhrase(const std::wstring& text)
    {
        if (!m_initialized || !m_dbHandle)
            return false;

        sqlite3* db = reinterpret_cast<sqlite3*>(m_dbHandle);
        const char* sql = "INSERT OR IGNORE INTO user_phrases (text, freq, last_used) VALUES (?, 1, ?);";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return false;

        uint64_t now = GetTickCount64();

        sqlite3_bind_text(stmt, 1, StringUtils::UTF16ToUTF8(text).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(now));

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);

        if (!success)
        {
            const char* updateSql = "UPDATE user_phrases SET freq = freq + 1, last_used = ? WHERE text = ?;";
            if (sqlite3_prepare_v2(db, updateSql, -1, &stmt, nullptr) == SQLITE_OK)
            {
                sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(now));
                sqlite3_bind_text(stmt, 2, StringUtils::UTF16ToUTF8(text).c_str(), -1, SQLITE_STATIC);
                success = (sqlite3_step(stmt) == SQLITE_DONE);
                sqlite3_finalize(stmt);
            }
        }

        return success;
    }

    bool UserPhraseManager::RemovePhrase(const std::wstring& text)
    {
        if (!m_initialized || !m_dbHandle)
            return false;

        sqlite3* db = reinterpret_cast<sqlite3*>(m_dbHandle);
        const char* sql = "DELETE FROM user_phrases WHERE text = ?;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return false;

        sqlite3_bind_text(stmt, 1, StringUtils::UTF16ToUTF8(text).c_str(), -1, SQLITE_STATIC);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return success;
    }

    bool UserPhraseManager::UpdatePhrase(const UserPhrase& phrase)
    {
        if (!m_initialized || !m_dbHandle)
            return false;

        sqlite3* db = reinterpret_cast<sqlite3*>(m_dbHandle);
        const char* sql = "UPDATE user_phrases SET freq = ?, last_used = ?, language_score = ? WHERE text = ?;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return false;

        sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(phrase.freq));
        sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(phrase.last_used));
        sqlite3_bind_double(stmt, 3, phrase.language_score);
        sqlite3_bind_text(stmt, 4, StringUtils::UTF16ToUTF8(phrase.text).c_str(), -1, SQLITE_STATIC);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return success;
    }

    std::vector<UserPhrase> UserPhraseManager::QueryPhrases(const std::wstring& prefix, size_t limit)
    {
        std::vector<UserPhrase> result;
        if (!m_initialized || !m_dbHandle)
            return result;

        sqlite3* db = reinterpret_cast<sqlite3*>(m_dbHandle);
        std::string sql = "SELECT text, freq, last_used, language_score FROM user_phrases WHERE text LIKE ? ORDER BY freq DESC LIMIT " + std::to_string(limit) + ";";

        sqlite3_stmt* stmt;
        std::string pattern = StringUtils::UTF16ToUTF8(prefix) + "%";

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            return result;

        sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            UserPhrase phrase;
            phrase.text = StringUtils::UTF8ToUTF16(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            phrase.freq = static_cast<uint64_t>(sqlite3_column_int64(stmt, 1));
            phrase.last_used = static_cast<uint64_t>(sqlite3_column_int64(stmt, 2));
            phrase.language_score = sqlite3_column_double(stmt, 3);
            result.push_back(phrase);
        }

        sqlite3_finalize(stmt);
        return result;
    }

    std::vector<UserPhrase> UserPhraseManager::GetTopPhrases(size_t limit)
    {
        std::vector<UserPhrase> result;
        if (!m_initialized || !m_dbHandle)
            return result;

        sqlite3* db = reinterpret_cast<sqlite3*>(m_dbHandle);
        std::string sql = "SELECT text, freq, last_used, language_score FROM user_phrases ORDER BY freq DESC LIMIT " + std::to_string(limit) + ";";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            return result;

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            UserPhrase phrase;
            phrase.text = StringUtils::UTF8ToUTF16(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            phrase.freq = static_cast<uint64_t>(sqlite3_column_int64(stmt, 1));
            phrase.last_used = static_cast<uint64_t>(sqlite3_column_int64(stmt, 2));
            phrase.language_score = sqlite3_column_double(stmt, 3);
            result.push_back(phrase);
        }

        sqlite3_finalize(stmt);
        return result;
    }

    bool UserPhraseManager::GetPhrase(const std::wstring& text, UserPhrase& phrase)
    {
        if (!m_initialized || !m_dbHandle)
            return false;

        sqlite3* db = reinterpret_cast<sqlite3*>(m_dbHandle);
        const char* sql = "SELECT text, freq, last_used, language_score FROM user_phrases WHERE text = ?;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return false;

        sqlite3_bind_text(stmt, 1, StringUtils::UTF16ToUTF8(text).c_str(), -1, SQLITE_STATIC);

        bool found = false;
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            phrase.text = StringUtils::UTF8ToUTF16(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            phrase.freq = static_cast<uint64_t>(sqlite3_column_int64(stmt, 1));
            phrase.last_used = static_cast<uint64_t>(sqlite3_column_int64(stmt, 2));
            phrase.language_score = sqlite3_column_double(stmt, 3);
            found = true;
        }

        sqlite3_finalize(stmt);
        return found;
    }

    bool UserPhraseManager::HasPhrase(const std::wstring& text)
    {
        UserPhrase phrase;
        return GetPhrase(text, phrase);
    }

    size_t UserPhraseManager::GetPhraseCount() const
    {
        if (!m_initialized || !m_dbHandle)
            return 0;

        sqlite3* db = reinterpret_cast<sqlite3*>(m_dbHandle);
        const char* sql = "SELECT COUNT(*) FROM user_phrases;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return 0;

        size_t count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            count = static_cast<size_t>(sqlite3_column_int(stmt, 0));
        }

        sqlite3_finalize(stmt);
        return count;
    }

    void UserPhraseManager::GarbageCollect(uint64_t min_timestamp)
    {
        if (!m_initialized || !m_dbHandle)
            return;

        sqlite3* db = reinterpret_cast<sqlite3*>(m_dbHandle);
        const char* sql = "DELETE FROM user_phrases WHERE last_used < ? AND freq < 5;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return;

        sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(min_timestamp));
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}