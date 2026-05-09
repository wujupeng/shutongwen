#pragma once

#include <vector>
#include <string>
#include <memory>

namespace ShuTongWen
{
    namespace Language
    {
        struct SegmentPath;
    }

    enum class InputTokenType
    {
        Unknown,
        Pinyin,
        EnglishWord,
        EnglishLetter,
        Number,
        Punctuation,
        Space,
        Symbol
    };

    struct InputToken
    {
        InputTokenType type;
        std::wstring text;
        bool completed;

        InputToken() : type(InputTokenType::Unknown), completed(false) {}
        InputToken(InputTokenType t, const std::wstring& txt, bool c = false)
            : type(t), text(txt), completed(c) {}
    };

    struct CandidateItem
    {
        std::wstring text;
        std::wstring pinyin;
        double score;
        int frequency;
        std::wstring category;

        CandidateItem() : score(0.0), frequency(0) {}
        CandidateItem(const std::wstring& t, const std::wstring& p, double s, int f, const std::wstring& c = L"")
            : text(t), pinyin(p), score(s), frequency(f), category(c) {}
    };

    class MixedInputProcessor
    {
    public:
        virtual ~MixedInputProcessor() = default;

        virtual bool ProcessChar(wchar_t ch) = 0;
        virtual bool ProcessBackspace() = 0;
        virtual bool ProcessSpace() = 0;
        virtual bool ProcessEnter() = 0;
        virtual bool ProcessEscape() = 0;

        virtual const std::vector<InputToken>& GetTokens() const = 0;
        virtual std::wstring GetCurrentPinyin() const = 0;
        virtual std::wstring GetComposedText() const = 0;

        virtual void Reset() = 0;
        virtual bool IsComposing() const = 0;

        virtual const std::vector<Language::SegmentPath>& GetSegmentedPaths() const = 0;
        virtual std::vector<CandidateItem> GetSegmentedCandidates(size_t limit) const = 0;

        static std::unique_ptr<MixedInputProcessor> Create();
    };
}