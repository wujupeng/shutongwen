#include "ime/MixedInputProcessor.h"
#include "ime/PinyinParser.h"
#include "ime/language/SegmentGraph.h"
#include "ime/language/CandidateRanker.h"
#include "utils/logger.h"

namespace ShuTongWen
{
    class MixedInputProcessorImpl : public MixedInputProcessor
    {
    public:
        MixedInputProcessorImpl() 
            : m_isComposing(false),
              m_segmentGraph(std::make_unique<Language::SegmentGraph>()),
              m_candidateRanker(std::make_unique<Language::CandidateRanker>())
        {
            m_segmentGraph->LoadDictionary(L"data/dictionaries/mixed_input.txt");
            m_candidateRanker->Initialize();
        }

        bool ProcessChar(wchar_t ch) override
        {
            if (IsEnglishLetter(ch))
            {
                return ProcessEnglishChar(ch);
            }
            else if (IsDigit(ch))
            {
                return ProcessDigit(ch);
            }
            else if (IsPunctuation(ch))
            {
                return ProcessPunctuation(ch);
            }
            else if (ch == L' ')
            {
                return ProcessSpace();
            }
            else
            {
                return ProcessPinyinChar(ch);
            }
        }

        bool ProcessBackspace() override
        {
            if (m_currentToken.text.empty())
            {
                if (!m_tokens.empty())
                {
                    m_currentToken = m_tokens.back();
                    m_tokens.pop_back();
                    m_currentToken.completed = false;
                }
                return !m_tokens.empty() || !m_currentToken.text.empty();
            }

            m_currentToken.text.pop_back();
            m_isComposing = !m_currentToken.text.empty() || !m_tokens.empty();
            
            if (m_isComposing)
            {
                UpdateSegmentation();
            }
            
            return true;
        }

        bool ProcessSpace() override
        {
            if (!m_currentToken.text.empty())
            {
                m_currentToken.completed = true;
                m_tokens.push_back(m_currentToken);
                m_currentToken = InputToken();
            }
            m_isComposing = !m_tokens.empty();
            return true;
        }

        bool ProcessEnter() override
        {
            if (!m_currentToken.text.empty())
            {
                m_currentToken.completed = true;
                m_tokens.push_back(m_currentToken);
            }
            bool wasComposing = m_isComposing;
            Reset();
            return wasComposing;
        }

        bool ProcessEscape() override
        {
            bool wasComposing = m_isComposing;
            Reset();
            return wasComposing;
        }

        const std::vector<InputToken>& GetTokens() const override
        {
            return m_tokens;
        }

        std::wstring GetCurrentPinyin() const override
        {
            if (m_currentToken.type == InputTokenType::Pinyin)
            {
                return m_currentToken.text;
            }
            return L"";
        }

        std::wstring GetComposedText() const override
        {
            std::wstring text;
            for (const auto& token : m_tokens)
            {
                text += token.text;
            }
            text += m_currentToken.text;
            return text;
        }

        void Reset() override
        {
            m_tokens.clear();
            m_currentToken = InputToken();
            m_isComposing = false;
            m_segmentedPaths.clear();
        }

        bool IsComposing() const override
        {
            return m_isComposing;
        }

        const std::vector<Language::SegmentPath>& GetSegmentedPaths() const override
        {
            return m_segmentedPaths;
        }

        std::vector<CandidateItem> GetSegmentedCandidates(size_t limit) const override
        {
            std::vector<CandidateItem> candidates;
            
            if (!m_segmentedPaths.empty())
            {
                const auto& bestPath = m_segmentedPaths[0];
                for (const auto& segment : bestPath.segments)
                {
                    if (segment.type == L"pinyin")
                    {
                        std::vector<std::wstring> pinyinList = PinyinParser::SplitPinyin(segment.text);
                        for (const auto& pinyin : pinyinList)
                        {
                            std::vector<CandidateItem> pinyinCandidates = PinyinParser::GetPinyinCandidates(pinyin);
                            candidates.insert(candidates.end(), pinyinCandidates.begin(), pinyinCandidates.end());
                        }
                    }
                    else if (segment.type == L"english")
                    {
                        CandidateItem item;
                        item.text = segment.text;
                        item.pinyin = L"";
                        item.score = segment.score;
                        candidates.push_back(item);
                    }
                }
            }

            if (candidates.size() > limit)
            {
                candidates.resize(limit);
            }

            return candidates;
        }

    private:
        void UpdateSegmentation()
        {
            std::wstring fullInput = GetComposedText();
            if (!fullInput.empty())
            {
                m_segmentGraph->BuildGraph(fullInput);
                m_segmentedPaths = m_segmentGraph->FindBestPaths(5);
            }
        }

        bool IsEnglishLetter(wchar_t ch) const
        {
            return (ch >= L'A' && ch <= L'Z') || (ch >= L'a' && ch <= L'z');
        }

        bool IsDigit(wchar_t ch) const
        {
            return ch >= L'0' && ch <= L'9';
        }

        bool IsPunctuation(wchar_t ch) const
        {
            static const wchar_t punctuations[] = L".,?!;:\"'`~@#$%^&*()_+-=[]{}|\\<>/";
            for (size_t i = 0; i < wcslen(punctuations); ++i)
            {
                if (ch == punctuations[i])
                    return true;
            }
            return false;
        }

        bool ProcessEnglishChar(wchar_t ch)
        {
            if (ch >= L'A' && ch <= L'Z')
            {
                ch = ch + (L'a' - L'A');
            }

            if (m_currentToken.type == InputTokenType::EnglishWord || 
                m_currentToken.type == InputTokenType::Unknown)
            {
                m_currentToken.type = InputTokenType::EnglishWord;
                m_currentToken.text += ch;
            }
            else if (m_currentToken.type == InputTokenType::Pinyin)
            {
                m_currentToken.completed = true;
                m_tokens.push_back(m_currentToken);
                m_currentToken = InputToken(InputTokenType::EnglishWord, std::wstring(1, ch));
            }

            m_isComposing = true;
            UpdateSegmentation();
            return true;
        }

        bool ProcessDigit(wchar_t ch)
        {
            if (m_currentToken.type == InputTokenType::Number ||
                m_currentToken.type == InputTokenType::Unknown)
            {
                m_currentToken.type = InputTokenType::Number;
                m_currentToken.text += ch;
            }
            else
            {
                m_currentToken.completed = true;
                m_tokens.push_back(m_currentToken);
                m_currentToken = InputToken(InputTokenType::Number, std::wstring(1, ch));
            }

            m_isComposing = true;
            UpdateSegmentation();
            return true;
        }

        bool ProcessPunctuation(wchar_t ch)
        {
            if (!m_currentToken.text.empty())
            {
                m_currentToken.completed = true;
                m_tokens.push_back(m_currentToken);
            }
            m_currentToken = InputToken(InputTokenType::Punctuation, std::wstring(1, ch), true);
            m_tokens.push_back(m_currentToken);
            m_currentToken = InputToken();

            m_isComposing = !m_tokens.empty();
            UpdateSegmentation();
            return true;
        }

        bool ProcessPinyinChar(wchar_t ch)
        {
            if (ch >= L'A' && ch <= L'Z')
            {
                ch = ch + (L'a' - L'A');
            }

            if (m_currentToken.type == InputTokenType::Pinyin ||
                m_currentToken.type == InputTokenType::Unknown)
            {
                m_currentToken.type = InputTokenType::Pinyin;
                m_currentToken.text += ch;
            }
            else
            {
                m_currentToken.completed = true;
                m_tokens.push_back(m_currentToken);
                m_currentToken = InputToken(InputTokenType::Pinyin, std::wstring(1, ch));
            }

            m_isComposing = true;
            UpdateSegmentation();
            return true;
        }

        std::vector<InputToken> m_tokens;
        InputToken m_currentToken;
        bool m_isComposing;
        
        std::unique_ptr<Language::SegmentGraph> m_segmentGraph;
        std::unique_ptr<Language::CandidateRanker> m_candidateRanker;
        std::vector<Language::SegmentPath> m_segmentedPaths;
    };

    std::unique_ptr<MixedInputProcessor> MixedInputProcessor::Create()
    {
        return std::make_unique<MixedInputProcessorImpl>();
    }
}