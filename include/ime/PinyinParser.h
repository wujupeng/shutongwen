#pragma once

#include <vector>
#include <string>

namespace ShuTongWen
{
    struct PinyinUnit
    {
        std::wstring pinyin;
        std::wstring tone;
        int tone_number;
        bool valid;
    };

    class PinyinParser
    {
    public:
        static bool Parse(const std::wstring& input, std::vector<PinyinUnit>& result);
        static std::vector<std::wstring> GetValidPinyins();
        static bool IsValidPinyin(const std::wstring& pinyin);
        static std::wstring RemoveTone(const std::wstring& pinyin);
        static int GetToneNumber(const std::wstring& pinyin);
    };
}