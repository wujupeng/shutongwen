#include "ime/InputProcessor.h"
#include "ime/PinyinParser.h"
#include "utils/logger.h"

namespace ShuTongWen
{
    class InputProcessorImpl : public InputProcessor
    {
    public:
        InputProcessorImpl() 
            : m_isComposing(false)
        {}

        bool ProcessChar(wchar_t ch) override
        {
            if (ch >= 'A' && ch <= 'Z')
            {
                ch = ch + ('a' - 'A');
            }

            if (ch >= 'a' && ch <= 'z')
            {
                m_currentPinyin += ch;
                UpdateComposition();
                return true;
            }
            
            return false;
        }

        bool ProcessBackspace() override
        {
            if (m_currentPinyin.empty())
                return false;

            m_currentPinyin.pop_back();
            UpdateComposition();
            return true;
        }

        bool ProcessSpace() override
        {
            if (m_currentPinyin.empty())
                return false;

            std::vector<PinyinUnit> units;
            if (PinyinParser::Parse(m_currentPinyin, units))
            {
                for (const auto& unit : units)
                {
                    InputSegment segment;
                    segment.pinyin = unit.pinyin;
                    segment.hanzi = L"";
                    segment.completed = false;
                    m_segments.push_back(segment);
                }
            }
            m_currentPinyin.clear();
            m_isComposing = !m_segments.empty();
            return true;
        }

        bool ProcessEnter() override
        {
            bool result = m_isComposing;
            Reset();
            return result;
        }

        bool ProcessEscape() override
        {
            bool result = m_isComposing;
            Reset();
            return result;
        }

        const std::vector<InputSegment>& GetSegments() const override
        {
            return m_segments;
        }

        std::wstring GetCurrentPinyin() const override
        {
            return m_currentPinyin;
        }

        std::wstring GetComposedText() const override
        {
            std::wstring text;
            for (const auto& segment : m_segments)
            {
                text += segment.hanzi.empty() ? segment.pinyin : segment.hanzi;
            }
            text += m_currentPinyin;
            return text;
        }

        void Reset() override
        {
            m_segments.clear();
            m_currentPinyin.clear();
            m_isComposing = false;
        }

        bool IsComposing() const override
        {
            return m_isComposing || !m_currentPinyin.empty();
        }

    private:
        void UpdateComposition()
        {
            m_isComposing = !m_currentPinyin.empty();
        }

        std::vector<InputSegment> m_segments;
        std::wstring m_currentPinyin;
        bool m_isComposing;
    };

    std::unique_ptr<InputProcessor> InputProcessor::Create()
    {
        return std::make_unique<InputProcessorImpl>();
    }
}