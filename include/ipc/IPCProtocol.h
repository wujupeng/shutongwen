#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <memory>

namespace ShuTongWen
{
    namespace IPC
    {
        enum class MessageType : uint32_t
        {
            Invalid = 0,
            
            CandidateUpdate = 1,
            CompositionUpdate = 2,
            FocusChanged = 3,
            ThemeChanged = 4,
            EmojiSuggestion = 5,
            ClipboardUpdate = 6,
            InputModeChanged = 7,
            PreferencesChanged = 8,
            SessionStarted = 9,
            SessionEnded = 10,
            
            UIReady = 100,
            UIRequestCandidates = 101,
            UIRequestComposition = 102,
            UICandidateSelected = 103,
            UIFocusLost = 104,
            
            SystemNotification = 200,
            SystemShutdown = 201,
            SystemLanguageChanged = 202,
            
            Error = 900,
            Ping = 999
        };

        struct CandidateData
        {
            std::wstring text;
            std::wstring pinyin;
            double score;
            int frequency;
            std::wstring category;
            int index;
        };

        struct CompositionData
        {
            std::wstring text;
            int cursorPosition;
            std::wstring pinyin;
        };

        struct ThemeData
        {
            bool isDarkMode;
            std::wstring accentColor;
            std::wstring backgroundColor;
            double opacity;
        };

        struct ClipboardData
        {
            std::wstring text;
            std::vector<std::wstring> files;
            std::vector<uint8_t> imageData;
            std::wstring html;
            std::wstring rtf;
            uint32_t dataType;
            uint64_t timestamp;
        };

        struct EmojiSuggestionData
        {
            std::wstring emoji;
            std::wstring text;
            std::vector<std::wstring> keywords;
            int frequency;
        };

        struct MessageHeader
        {
            uint32_t magic;
            MessageType type;
            uint32_t payloadSize;
            uint64_t timestamp;
            uint32_t sequenceId;
        };

        class IPCMessage
        {
        public:
            IPCMessage();
            IPCMessage(MessageType type);
            ~IPCMessage();

            MessageType GetType() const { return m_header.type; }
            uint32_t GetSequenceId() const { return m_header.sequenceId; }
            uint64_t GetTimestamp() const { return m_header.timestamp; }

            void SetPayload(const std::vector<uint8_t>& data);
            const std::vector<uint8_t>& GetPayload() const { return m_payload; }

            bool Serialize(std::vector<uint8_t>& buffer) const;
            bool Deserialize(const std::vector<uint8_t>& buffer);

            void SetCandidateData(const std::vector<CandidateData>& candidates);
            bool GetCandidateData(std::vector<CandidateData>& candidates) const;

            void SetCompositionData(const CompositionData& data);
            bool GetCompositionData(CompositionData& data) const;

            void SetThemeData(const ThemeData& data);
            bool GetThemeData(ThemeData& data) const;

            void SetClipboardData(const ClipboardData& data);
            bool GetClipboardData(ClipboardData& data) const;

            void SetEmojiSuggestions(const std::vector<EmojiSuggestionData>& suggestions);
            bool GetEmojiSuggestions(std::vector<EmojiSuggestionData>& suggestions) const;

        private:
            MessageHeader m_header;
            std::vector<uint8_t> m_payload;
            
            static uint32_t s_nextSequenceId;
            static const uint32_t MAGIC = 0x53545749;
        };

        class MessageFactory
        {
        public:
            static std::unique_ptr<IPCMessage> CreateCandidateUpdate(const std::vector<CandidateData>& candidates);
            static std::unique_ptr<IPCMessage> CreateCompositionUpdate(const CompositionData& data);
            static std::unique_ptr<IPCMessage> CreateFocusChanged(bool hasFocus);
            static std::unique_ptr<IPCMessage> CreateThemeChanged(const ThemeData& theme);
            static std::unique_ptr<IPCMessage> CreateEmojiSuggestion(const std::vector<EmojiSuggestionData>& suggestions);
            static std::unique_ptr<IPCMessage> CreateClipboardUpdate(const ClipboardData& data);
            static std::unique_ptr<IPCMessage> CreateInputModeChanged(const std::wstring& mode);
            static std::unique_ptr<IPCMessage> CreatePing();
        };
    }
}