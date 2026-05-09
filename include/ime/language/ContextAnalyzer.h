#pragma once

#include <vector>
#include <string>
#include <memory>

namespace ShuTongWen
{
    namespace Language
    {
        struct ContextInfo
        {
            std::vector<std::wstring> precedingWords;
            std::vector<std::wstring> followingWords;
            std::wstring sentence;
            int cursorPosition;
            std::wstring appName;
            std::wstring inputMode;
        };

        class ContextAnalyzer
        {
        public:
            ContextAnalyzer();
            ~ContextAnalyzer();

            bool Initialize();
            void Uninitialize();

            ContextInfo Analyze(const std::wstring& text, int cursorPos, const std::wstring& appName);
            
            std::vector<std::wstring> GetContextWords(const std::wstring& text, int cursorPos, int windowSize = 3);
            std::wstring DetectInputMode(const std::wstring& text, const std::wstring& appName);
            
            bool IsEmailContext(const std::wstring& text);
            bool IsUrlContext(const std::wstring& text);
            bool IsCodeContext(const std::wstring& appName);

        private:
            bool m_initialized;
        };
    }
}