#include "utils/string_utils.h"
#include <codecvt>
#include <locale>

namespace ShuTongWen
{
    namespace StringUtils
    {
        std::wstring UTF8ToUTF16(const std::string& utf8)
        {
            if (utf8.empty())
                return L"";

            try
            {
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                return converter.from_bytes(utf8);
            }
            catch (...)
            {
                return L"";
            }
        }

        std::string UTF16ToUTF8(const std::wstring& utf16)
        {
            if (utf16.empty())
                return "";

            try
            {
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                return converter.to_bytes(utf16);
            }
            catch (...)
            {
                return "";
            }
        }

        std::wstring ToLower(const std::wstring& str)
        {
            std::wstring result = str;
            for (size_t i = 0; i < result.length(); ++i)
            {
                result[i] = towlower(result[i]);
            }
            return result;
        }

        std::wstring ToUpper(const std::wstring& str)
        {
            std::wstring result = str;
            for (size_t i = 0; i < result.length(); ++i)
            {
                result[i] = towupper(result[i]);
            }
            return result;
        }

        bool StartsWith(const std::wstring& str, const std::wstring& prefix)
        {
            if (prefix.length() > str.length())
                return false;
            return str.substr(0, prefix.length()) == prefix;
        }

        bool EndsWith(const std::wstring& str, const std::wstring& suffix)
        {
            if (suffix.length() > str.length())
                return false;
            return str.substr(str.length() - suffix.length()) == suffix;
        }

        std::vector<std::wstring> Split(const std::wstring& str, wchar_t delimiter)
        {
            std::vector<std::wstring> parts;
            std::wstring current;

            for (wchar_t ch : str)
            {
                if (ch == delimiter)
                {
                    if (!current.empty())
                    {
                        parts.push_back(current);
                        current.clear();
                    }
                }
                else
                {
                    current += ch;
                }
            }

            if (!current.empty())
            {
                parts.push_back(current);
            }

            return parts;
        }

        std::wstring Join(const std::vector<std::wstring>& parts, wchar_t delimiter)
        {
            std::wstring result;
            for (size_t i = 0; i < parts.size(); ++i)
            {
                if (i > 0)
                    result += delimiter;
                result += parts[i];
            }
            return result;
        }

        std::wstring Trim(const std::wstring& str)
        {
            return TrimRight(TrimLeft(str));
        }

        std::wstring TrimLeft(const std::wstring& str)
        {
            size_t start = str.find_first_not_of(L" \t\n\r");
            if (start == std::wstring::npos)
                return L"";
            return str.substr(start);
        }

        std::wstring TrimRight(const std::wstring& str)
        {
            size_t end = str.find_last_not_of(L" \t\n\r");
            if (end == std::wstring::npos)
                return L"";
            return str.substr(0, end + 1);
        }

        bool IsDigit(wchar_t ch)
        {
            return ch >= L'0' && ch <= L'9';
        }

        bool IsLetter(wchar_t ch)
        {
            return (ch >= L'A' && ch <= L'Z') || (ch >= L'a' && ch <= L'z');
        }

        bool IsPunctuation(wchar_t ch)
        {
            return iswpunct(ch) != 0;
        }

        int CompareIgnoreCase(const std::wstring& str1, const std::wstring& str2)
        {
            std::wstring lower1 = ToLower(str1);
            std::wstring lower2 = ToLower(str2);
            return lower1.compare(lower2);
        }
    }
}