#include "ime/language/ContextAnalyzer.h"
#include "utils/logger.h"
#include <algorithm>
#include <regex>

namespace ShuTongWen
{
    namespace Language
    {
        ContextAnalyzer::ContextAnalyzer()
            : m_initialized(false)
        {}

        ContextAnalyzer::~ContextAnalyzer()
        {
            Uninitialize();
        }

        bool ContextAnalyzer::Initialize()
        {
            Logger::Info("Initializing ContextAnalyzer...");
            m_initialized = true;
            return true;
        }

        void ContextAnalyzer::Uninitialize()
        {
            m_initialized = false;
        }

        ContextInfo ContextAnalyzer::Analyze(const std::wstring& text, int cursorPos, const std::wstring& appName)
        {
            ContextInfo info;
            info.sentence = text;
            info.cursorPosition = cursorPos;
            info.appName = appName;
            info.inputMode = DetectInputMode(text, appName);
            info.precedingWords = GetContextWords(text, cursorPos, 3);
            
            return info;
        }

        std::vector<std::wstring> ContextAnalyzer::GetContextWords(const std::wstring& text, int cursorPos, int windowSize)
        {
            std::vector<std::wstring> words;
            
            std::wstring leftPart = text.substr(0, cursorPos);
            std::reverse(leftPart.begin(), leftPart.end());

            std::wstring word;
            int count = 0;
            
            for (wchar_t ch : leftPart)
            {
                if (ch == L' ' || ch == L'\t' || ch == L'\n' ||
                    ch == L'，' || ch == L'。' || ch == L'！' || ch == L'？')
                {
                    if (!word.empty())
                    {
                        std::reverse(word.begin(), word.end());
                        words.push_back(word);
                        word.clear();
                        count++;
                        
                        if (count >= windowSize)
                            break;
                    }
                }
                else
                {
                    word.push_back(ch);
                }
            }

            if (!word.empty() && count < windowSize)
            {
                std::reverse(word.begin(), word.end());
                words.push_back(word);
            }

            std::reverse(words.begin(), words.end());
            return words;
        }

        std::wstring ContextAnalyzer::DetectInputMode(const std::wstring& text, const std::wstring& appName)
        {
            if (IsCodeContext(appName))
                return L"code";
            
            if (IsEmailContext(text))
                return L"email";
            
            if (IsUrlContext(text))
                return L"url";

            int englishCount = 0;
            int chineseCount = 0;
            
            for (wchar_t ch : text)
            {
                if ((ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z'))
                    englishCount++;
                else if (ch >= 0x4E00 && ch <= 0x9FFF)
                    chineseCount++;
            }

            if (englishCount > chineseCount * 2)
                return L"english";
            
            return L"chinese";
        }

        bool ContextAnalyzer::IsEmailContext(const std::wstring& text)
        {
            static const std::wregex emailRegex(L"[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}");
            return std::regex_search(text, emailRegex);
        }

        bool ContextAnalyzer::IsUrlContext(const std::wstring& text)
        {
            static const std::wregex urlRegex(L"https?://[\\w.-]+(?:/[\\w./-]*)?");
            return std::regex_search(text, urlRegex);
        }

        bool ContextAnalyzer::IsCodeContext(const std::wstring& appName)
        {
            static const std::vector<std::wstring> codeApps = {
                L"Code", L"Visual Studio", L"JetBrains", L"Vim", L"Emacs",
                L"Android Studio", L"Xcode", L"PyCharm", L"GoLand", L"CLion"
            };

            for (const std::wstring& app : codeApps)
            {
                if (appName.find(app) != std::wstring::npos)
                    return true;
            }

            return false;
        }
    }
}