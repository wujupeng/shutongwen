#include "ipc/IPCProtocol.h"
#include "utils/logger.h"
#include <algorithm>
#include <cstring>

namespace ShuTongWen
{
    namespace IPC
    {
        uint32_t IPCMessage::s_nextSequenceId = 0;

        IPCMessage::IPCMessage()
        {
            m_header.magic = MAGIC;
            m_header.type = MessageType::Invalid;
            m_header.payloadSize = 0;
            m_header.timestamp = 0;
            m_header.sequenceId = ++s_nextSequenceId;
        }

        IPCMessage::IPCMessage(MessageType type)
        {
            m_header.magic = MAGIC;
            m_header.type = type;
            m_header.payloadSize = 0;
            m_header.timestamp = GetTickCount64();
            m_header.sequenceId = ++s_nextSequenceId;
        }

        IPCMessage::~IPCMessage()
        {}

        void IPCMessage::SetPayload(const std::vector<uint8_t>& data)
        {
            m_payload = data;
            m_header.payloadSize = static_cast<uint32_t>(data.size());
        }

        bool IPCMessage::Serialize(std::vector<uint8_t>& buffer) const
        {
            buffer.clear();
            
            size_t headerSize = sizeof(MessageHeader);
            size_t totalSize = headerSize + m_payload.size();
            
            buffer.resize(totalSize);
            
            std::memcpy(buffer.data(), &m_header, headerSize);
            
            if (!m_payload.empty())
            {
                std::memcpy(buffer.data() + headerSize, m_payload.data(), m_payload.size());
            }
            
            return true;
        }

        bool IPCMessage::Deserialize(const std::vector<uint8_t>& buffer)
        {
            if (buffer.size() < sizeof(MessageHeader))
                return false;

            std::memcpy(&m_header, buffer.data(), sizeof(MessageHeader));
            
            if (m_header.magic != MAGIC)
                return false;

            if (buffer.size() > sizeof(MessageHeader))
            {
                m_payload.resize(m_header.payloadSize);
                std::memcpy(m_payload.data(), buffer.data() + sizeof(MessageHeader), m_header.payloadSize);
            }
            else
            {
                m_payload.clear();
            }
            
            return true;
        }

        void IPCMessage::SetCandidateData(const std::vector<CandidateData>& candidates)
        {
            std::vector<uint8_t> payload;
            
            uint32_t count = static_cast<uint32_t>(candidates.size());
            payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&count), 
                          reinterpret_cast<uint8_t*>(&count) + sizeof(uint32_t));
            
            for (const auto& candidate : candidates)
            {
                uint32_t textLen = static_cast<uint32_t>(candidate.text.size());
                payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&textLen),
                              reinterpret_cast<uint8_t*>(&textLen) + sizeof(uint32_t));
                payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(candidate.text.data()),
                              reinterpret_cast<const uint8_t*>(candidate.text.data()) + textLen * sizeof(wchar_t));
                
                uint32_t pinyinLen = static_cast<uint32_t>(candidate.pinyin.size());
                payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&pinyinLen),
                              reinterpret_cast<uint8_t*>(&pinyinLen) + sizeof(uint32_t));
                payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(candidate.pinyin.data()),
                              reinterpret_cast<const uint8_t*>(candidate.pinyin.data()) + pinyinLen * sizeof(wchar_t));
                
                payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(&candidate.score),
                              reinterpret_cast<const uint8_t*>(&candidate.score) + sizeof(double));
                
                payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(&candidate.frequency),
                              reinterpret_cast<const uint8_t*>(&candidate.frequency) + sizeof(int));
                
                uint32_t categoryLen = static_cast<uint32_t>(candidate.category.size());
                payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&categoryLen),
                              reinterpret_cast<uint8_t*>(&categoryLen) + sizeof(uint32_t));
                payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(candidate.category.data()),
                              reinterpret_cast<const uint8_t*>(candidate.category.data()) + categoryLen * sizeof(wchar_t));
                
                payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(&candidate.index),
                              reinterpret_cast<const uint8_t*>(&candidate.index) + sizeof(int));
            }
            
            SetPayload(payload);
        }

        bool IPCMessage::GetCandidateData(std::vector<CandidateData>& candidates) const
        {
            candidates.clear();
            
            if (m_payload.empty())
                return true;
            
            const uint8_t* ptr = m_payload.data();
            size_t offset = 0;
            
            uint32_t count = *reinterpret_cast<const uint32_t*>(ptr + offset);
            offset += sizeof(uint32_t);
            
            for (uint32_t i = 0; i < count; ++i)
            {
                CandidateData candidate;
                
                uint32_t textLen = *reinterpret_cast<const uint32_t*>(ptr + offset);
                offset += sizeof(uint32_t);
                candidate.text.resize(textLen);
                std::memcpy(&candidate.text[0], ptr + offset, textLen * sizeof(wchar_t));
                offset += textLen * sizeof(wchar_t);
                
                uint32_t pinyinLen = *reinterpret_cast<const uint32_t*>(ptr + offset);
                offset += sizeof(uint32_t);
                candidate.pinyin.resize(pinyinLen);
                std::memcpy(&candidate.pinyin[0], ptr + offset, pinyinLen * sizeof(wchar_t));
                offset += pinyinLen * sizeof(wchar_t);
                
                candidate.score = *reinterpret_cast<const double*>(ptr + offset);
                offset += sizeof(double);
                
                candidate.frequency = *reinterpret_cast<const int*>(ptr + offset);
                offset += sizeof(int);
                
                uint32_t categoryLen = *reinterpret_cast<const uint32_t*>(ptr + offset);
                offset += sizeof(uint32_t);
                candidate.category.resize(categoryLen);
                std::memcpy(&candidate.category[0], ptr + offset, categoryLen * sizeof(wchar_t));
                offset += categoryLen * sizeof(wchar_t);
                
                candidate.index = *reinterpret_cast<const int*>(ptr + offset);
                offset += sizeof(int);
                
                candidates.push_back(candidate);
            }
            
            return true;
        }

        void IPCMessage::SetCompositionData(const CompositionData& data)
        {
            std::vector<uint8_t> payload;
            
            uint32_t textLen = static_cast<uint32_t>(data.text.size());
            payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&textLen),
                          reinterpret_cast<uint8_t*>(&textLen) + sizeof(uint32_t));
            payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(data.text.data()),
                          reinterpret_cast<const uint8_t*>(data.text.data()) + textLen * sizeof(wchar_t));
            
            payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(&data.cursorPosition),
                          reinterpret_cast<const uint8_t*>(&data.cursorPosition) + sizeof(int));
            
            uint32_t pinyinLen = static_cast<uint32_t>(data.pinyin.size());
            payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&pinyinLen),
                          reinterpret_cast<uint8_t*>(&pinyinLen) + sizeof(uint32_t));
            payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(data.pinyin.data()),
                          reinterpret_cast<const uint8_t*>(data.pinyin.data()) + pinyinLen * sizeof(wchar_t));
            
            SetPayload(payload);
        }

        bool IPCMessage::GetCompositionData(CompositionData& data) const
        {
            if (m_payload.empty())
                return false;
            
            const uint8_t* ptr = m_payload.data();
            size_t offset = 0;
            
            uint32_t textLen = *reinterpret_cast<const uint32_t*>(ptr + offset);
            offset += sizeof(uint32_t);
            data.text.resize(textLen);
            std::memcpy(&data.text[0], ptr + offset, textLen * sizeof(wchar_t));
            offset += textLen * sizeof(wchar_t);
            
            data.cursorPosition = *reinterpret_cast<const int*>(ptr + offset);
            offset += sizeof(int);
            
            uint32_t pinyinLen = *reinterpret_cast<const uint32_t*>(ptr + offset);
            offset += sizeof(uint32_t);
            data.pinyin.resize(pinyinLen);
            std::memcpy(&data.pinyin[0], ptr + offset, pinyinLen * sizeof(wchar_t));
            
            return true;
        }

        void IPCMessage::SetThemeData(const ThemeData& data)
        {
            std::vector<uint8_t> payload;
            
            payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(&data.isDarkMode),
                          reinterpret_cast<const uint8_t*>(&data.isDarkMode) + sizeof(bool));
            
            uint32_t accentLen = static_cast<uint32_t>(data.accentColor.size());
            payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&accentLen),
                          reinterpret_cast<uint8_t*>(&accentLen) + sizeof(uint32_t));
            payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(data.accentColor.data()),
                          reinterpret_cast<const uint8_t*>(data.accentColor.data()) + accentLen * sizeof(wchar_t));
            
            uint32_t bgLen = static_cast<uint32_t>(data.backgroundColor.size());
            payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&bgLen),
                          reinterpret_cast<uint8_t*>(&bgLen) + sizeof(uint32_t));
            payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(data.backgroundColor.data()),
                          reinterpret_cast<const uint8_t*>(data.backgroundColor.data()) + bgLen * sizeof(wchar_t));
            
            payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(&data.opacity),
                          reinterpret_cast<const uint8_t*>(&data.opacity) + sizeof(double));
            
            SetPayload(payload);
        }

        bool IPCMessage::GetThemeData(ThemeData& data) const
        {
            if (m_payload.empty())
                return false;
            
            const uint8_t* ptr = m_payload.data();
            size_t offset = 0;
            
            data.isDarkMode = *reinterpret_cast<const bool*>(ptr + offset);
            offset += sizeof(bool);
            
            uint32_t accentLen = *reinterpret_cast<const uint32_t*>(ptr + offset);
            offset += sizeof(uint32_t);
            data.accentColor.resize(accentLen);
            std::memcpy(&data.accentColor[0], ptr + offset, accentLen * sizeof(wchar_t));
            offset += accentLen * sizeof(wchar_t);
            
            uint32_t bgLen = *reinterpret_cast<const uint32_t*>(ptr + offset);
            offset += sizeof(uint32_t);
            data.backgroundColor.resize(bgLen);
            std::memcpy(&data.backgroundColor[0], ptr + offset, bgLen * sizeof(wchar_t));
            offset += bgLen * sizeof(wchar_t);
            
            data.opacity = *reinterpret_cast<const double*>(ptr + offset);
            
            return true;
        }

        void IPCMessage::SetClipboardData(const ClipboardData& data)
        {
            std::vector<uint8_t> payload;
            
            uint32_t textLen = static_cast<uint32_t>(data.text.size());
            payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&textLen),
                          reinterpret_cast<uint8_t*>(&textLen) + sizeof(uint32_t));
            payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(data.text.data()),
                          reinterpret_cast<const uint8_t*>(data.text.data()) + textLen * sizeof(wchar_t));
            
            uint32_t fileCount = static_cast<uint32_t>(data.files.size());
            payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&fileCount),
                          reinterpret_cast<uint8_t*>(&fileCount) + sizeof(uint32_t));
            for (const auto& file : data.files)
            {
                uint32_t fileLen = static_cast<uint32_t>(file.size());
                payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&fileLen),
                              reinterpret_cast<uint8_t*>(&fileLen) + sizeof(uint32_t));
                payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(file.data()),
                              reinterpret_cast<const uint8_t*>(file.data()) + fileLen * sizeof(wchar_t));
            }
            
            uint32_t imageSize = static_cast<uint32_t>(data.imageData.size());
            payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&imageSize),
                          reinterpret_cast<uint8_t*>(&imageSize) + sizeof(uint32_t));
            payload.insert(payload.end(), data.imageData.begin(), data.imageData.end());
            
            uint32_t htmlLen = static_cast<uint32_t>(data.html.size());
            payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&htmlLen),
                          reinterpret_cast<uint8_t*>(&htmlLen) + sizeof(uint32_t));
            payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(data.html.data()),
                          reinterpret_cast<const uint8_t*>(data.html.data()) + htmlLen * sizeof(wchar_t));
            
            uint32_t rtfLen = static_cast<uint32_t>(data.rtf.size());
            payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&rtfLen),
                          reinterpret_cast<uint8_t*>(&rtfLen) + sizeof(uint32_t));
            payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(data.rtf.data()),
                          reinterpret_cast<const uint8_t*>(data.rtf.data()) + rtfLen * sizeof(wchar_t));
            
            payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(&data.dataType),
                          reinterpret_cast<const uint8_t*>(&data.dataType) + sizeof(uint32_t));
            
            payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(&data.timestamp),
                          reinterpret_cast<const uint8_t*>(&data.timestamp) + sizeof(uint64_t));
            
            SetPayload(payload);
        }

        bool IPCMessage::GetClipboardData(ClipboardData& data) const
        {
            if (m_payload.empty())
                return false;
            
            const uint8_t* ptr = m_payload.data();
            size_t offset = 0;
            
            uint32_t textLen = *reinterpret_cast<const uint32_t*>(ptr + offset);
            offset += sizeof(uint32_t);
            data.text.resize(textLen);
            std::memcpy(&data.text[0], ptr + offset, textLen * sizeof(wchar_t));
            offset += textLen * sizeof(wchar_t);
            
            uint32_t fileCount = *reinterpret_cast<const uint32_t*>(ptr + offset);
            offset += sizeof(uint32_t);
            data.files.resize(fileCount);
            for (uint32_t i = 0; i < fileCount; ++i)
            {
                uint32_t fileLen = *reinterpret_cast<const uint32_t*>(ptr + offset);
                offset += sizeof(uint32_t);
                data.files[i].resize(fileLen);
                std::memcpy(&data.files[i][0], ptr + offset, fileLen * sizeof(wchar_t));
                offset += fileLen * sizeof(wchar_t);
            }
            
            uint32_t imageSize = *reinterpret_cast<const uint32_t*>(ptr + offset);
            offset += sizeof(uint32_t);
            data.imageData.resize(imageSize);
            std::memcpy(&data.imageData[0], ptr + offset, imageSize);
            offset += imageSize;
            
            uint32_t htmlLen = *reinterpret_cast<const uint32_t*>(ptr + offset);
            offset += sizeof(uint32_t);
            data.html.resize(htmlLen);
            std::memcpy(&data.html[0], ptr + offset, htmlLen * sizeof(wchar_t));
            offset += htmlLen * sizeof(wchar_t);
            
            uint32_t rtfLen = *reinterpret_cast<const uint32_t*>(ptr + offset);
            offset += sizeof(uint32_t);
            data.rtf.resize(rtfLen);
            std::memcpy(&data.rtf[0], ptr + offset, rtfLen * sizeof(wchar_t));
            offset += rtfLen * sizeof(wchar_t);
            
            data.dataType = *reinterpret_cast<const uint32_t*>(ptr + offset);
            offset += sizeof(uint32_t);
            
            data.timestamp = *reinterpret_cast<const uint64_t*>(ptr + offset);
            
            return true;
        }

        void IPCMessage::SetEmojiSuggestions(const std::vector<EmojiSuggestionData>& suggestions)
        {
            std::vector<uint8_t> payload;
            
            uint32_t count = static_cast<uint32_t>(suggestions.size());
            payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&count),
                          reinterpret_cast<uint8_t*>(&count) + sizeof(uint32_t));
            
            for (const auto& emoji : suggestions)
            {
                uint32_t emojiLen = static_cast<uint32_t>(emoji.emoji.size());
                payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&emojiLen),
                              reinterpret_cast<uint8_t*>(&emojiLen) + sizeof(uint32_t));
                payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(emoji.emoji.data()),
                              reinterpret_cast<const uint8_t*>(emoji.emoji.data()) + emojiLen * sizeof(wchar_t));
                
                uint32_t textLen = static_cast<uint32_t>(emoji.text.size());
                payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&textLen),
                              reinterpret_cast<uint8_t*>(&textLen) + sizeof(uint32_t));
                payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(emoji.text.data()),
                              reinterpret_cast<const uint8_t*>(emoji.text.data()) + textLen * sizeof(wchar_t));
                
                uint32_t keywordCount = static_cast<uint32_t>(emoji.keywords.size());
                payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&keywordCount),
                              reinterpret_cast<uint8_t*>(&keywordCount) + sizeof(uint32_t));
                for (const auto& keyword : emoji.keywords)
                {
                    uint32_t kwLen = static_cast<uint32_t>(keyword.size());
                    payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&kwLen),
                                  reinterpret_cast<uint8_t*>(&kwLen) + sizeof(uint32_t));
                    payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(keyword.data()),
                                  reinterpret_cast<const uint8_t*>(keyword.data()) + kwLen * sizeof(wchar_t));
                }
                
                payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(&emoji.frequency),
                              reinterpret_cast<const uint8_t*>(&emoji.frequency) + sizeof(int));
            }
            
            SetPayload(payload);
        }

        bool IPCMessage::GetEmojiSuggestions(std::vector<EmojiSuggestionData>& suggestions) const
        {
            suggestions.clear();
            
            if (m_payload.empty())
                return true;
            
            const uint8_t* ptr = m_payload.data();
            size_t offset = 0;
            
            uint32_t count = *reinterpret_cast<const uint32_t*>(ptr + offset);
            offset += sizeof(uint32_t);
            
            for (uint32_t i = 0; i < count; ++i)
            {
                EmojiSuggestionData emoji;
                
                uint32_t emojiLen = *reinterpret_cast<const uint32_t*>(ptr + offset);
                offset += sizeof(uint32_t);
                emoji.emoji.resize(emojiLen);
                std::memcpy(&emoji.emoji[0], ptr + offset, emojiLen * sizeof(wchar_t));
                offset += emojiLen * sizeof(wchar_t);
                
                uint32_t textLen = *reinterpret_cast<const uint32_t*>(ptr + offset);
                offset += sizeof(uint32_t);
                emoji.text.resize(textLen);
                std::memcpy(&emoji.text[0], ptr + offset, textLen * sizeof(wchar_t));
                offset += textLen * sizeof(wchar_t);
                
                uint32_t keywordCount = *reinterpret_cast<const uint32_t*>(ptr + offset);
                offset += sizeof(uint32_t);
                emoji.keywords.resize(keywordCount);
                for (uint32_t j = 0; j < keywordCount; ++j)
                {
                    uint32_t kwLen = *reinterpret_cast<const uint32_t*>(ptr + offset);
                    offset += sizeof(uint32_t);
                    emoji.keywords[j].resize(kwLen);
                    std::memcpy(&emoji.keywords[j][0], ptr + offset, kwLen * sizeof(wchar_t));
                    offset += kwLen * sizeof(wchar_t);
                }
                
                emoji.frequency = *reinterpret_cast<const int*>(ptr + offset);
                offset += sizeof(int);
                
                suggestions.push_back(emoji);
            }
            
            return true;
        }

        std::unique_ptr<IPCMessage> MessageFactory::CreateCandidateUpdate(const std::vector<CandidateData>& candidates)
        {
            auto msg = std::make_unique<IPCMessage>(MessageType::CandidateUpdate);
            msg->SetCandidateData(candidates);
            return msg;
        }

        std::unique_ptr<IPCMessage> MessageFactory::CreateCompositionUpdate(const CompositionData& data)
        {
            auto msg = std::make_unique<IPCMessage>(MessageType::CompositionUpdate);
            msg->SetCompositionData(data);
            return msg;
        }

        std::unique_ptr<IPCMessage> MessageFactory::CreateFocusChanged(bool hasFocus)
        {
            auto msg = std::make_unique<IPCMessage>(MessageType::FocusChanged);
            
            std::vector<uint8_t> payload;
            payload.push_back(hasFocus ? 1 : 0);
            msg->SetPayload(payload);
            
            return msg;
        }

        std::unique_ptr<IPCMessage> MessageFactory::CreateThemeChanged(const ThemeData& theme)
        {
            auto msg = std::make_unique<IPCMessage>(MessageType::ThemeChanged);
            msg->SetThemeData(theme);
            return msg;
        }

        std::unique_ptr<IPCMessage> MessageFactory::CreateEmojiSuggestion(const std::vector<EmojiSuggestionData>& suggestions)
        {
            auto msg = std::make_unique<IPCMessage>(MessageType::EmojiSuggestion);
            msg->SetEmojiSuggestions(suggestions);
            return msg;
        }

        std::unique_ptr<IPCMessage> MessageFactory::CreateClipboardUpdate(const ClipboardData& data)
        {
            auto msg = std::make_unique<IPCMessage>(MessageType::ClipboardUpdate);
            msg->SetClipboardData(data);
            return msg;
        }

        std::unique_ptr<IPCMessage> MessageFactory::CreateInputModeChanged(const std::wstring& mode)
        {
            auto msg = std::make_unique<IPCMessage>(MessageType::InputModeChanged);
            
            std::vector<uint8_t> payload;
            uint32_t len = static_cast<uint32_t>(mode.size());
            payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&len),
                          reinterpret_cast<uint8_t*>(&len) + sizeof(uint32_t));
            payload.insert(payload.end(), reinterpret_cast<const uint8_t*>(mode.data()),
                          reinterpret_cast<const uint8_t*>(mode.data()) + len * sizeof(wchar_t));
            msg->SetPayload(payload);
            
            return msg;
        }

        std::unique_ptr<IPCMessage> MessageFactory::CreatePing()
        {
            return std::make_unique<IPCMessage>(MessageType::Ping);
        }
    }
}