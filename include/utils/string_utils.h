#pragma once

#include <string>
#include <vector>

namespace ShuTongWen
{
    namespace StringUtils
    {
        std::wstring UTF8ToUTF16(const std::string& utf8);
        std::string UTF16ToUTF8(const std::wstring& utf16);

        std::wstring ToLower(const std::wstring& str);
        std::wstring ToUpper(const std::wstring& str);

        bool StartsWith(const std::wstring& str, const std::wstring& prefix);
        bool EndsWith(const std::wstring& str, const std::wstring& suffix);

        std::vector<std::wstring> Split(const std::wstring& str, wchar_t delimiter);
        std::wstring Join(const std::vector<std::wstring>& parts, wchar_t delimiter);

        std::wstring Trim(const std::wstring& str);
        std::wstring TrimLeft(const std::wstring& str);
        std::wstring TrimRight(const std::wstring& str);

        bool IsDigit(wchar_t ch);
        bool IsLetter(wchar_t ch);
        bool IsPunctuation(wchar_t ch);

        int CompareIgnoreCase(const std::wstring& str1, const std::wstring& str2);
    }
}