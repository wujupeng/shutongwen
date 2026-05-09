#pragma once

#include <vector>
#include <string>
#include <memory>

namespace ShuTongWen
{
    struct InputSegment
    {
        std::wstring pinyin;
        std::wstring hanzi;
        bool completed;
    };

    class InputProcessor
    {
    public:
        virtual ~InputProcessor() = default;

        virtual bool ProcessChar(wchar_t ch) = 0;
        virtual bool ProcessBackspace() = 0;
        virtual bool ProcessSpace() = 0;
        virtual bool ProcessEnter() = 0;
        virtual bool ProcessEscape() = 0;

        virtual const std::vector<InputSegment>& GetSegments() const = 0;
        virtual std::wstring GetCurrentPinyin() const = 0;
        virtual std::wstring GetComposedText() const = 0;

        virtual void Reset() = 0;
        virtual bool IsComposing() const = 0;

        static std::unique_ptr<InputProcessor> Create();
    };
}