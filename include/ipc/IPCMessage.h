#pragma once

#include <windows.h>
#include <string>
#include <nlohmann/json.hpp>

namespace ShuTongWen
{
    enum class IPCMessageType
    {
        Unknown = 0,
        TextInput,
        CandidateRequest,
        CandidateSelect,
        CompositionUpdate,
        CompositionCommit,
        ConfigRequest,
        ConfigUpdate,
        Ping,
        Shutdown
    };

    struct IPCMessage
    {
        IPCMessageType type;
        std::wstring data;
        std::wstring sender;
        uint64_t timestamp;

        IPCMessage() : type(IPCMessageType::Unknown), timestamp(0) {}

        std::string Serialize() const;
        bool Deserialize(const std::string& jsonStr);
        bool Deserialize(const std::wstring& jsonStr);
    };

    class IPCMessageBuilder
    {
    public:
        static IPCMessage CreateTextInput(const std::wstring& text);
        static IPCMessage CreateCandidateRequest(const std::wstring& pinyin);
        static IPCMessage CreateCandidateSelect(int index);
        static IPCMessage CreateCompositionUpdate(const std::wstring& text);
        static IPCMessage CreateCompositionCommit();
        static IPCMessage CreateConfigRequest(const std::wstring& key);
        static IPCMessage CreateConfigUpdate(const std::wstring& key, const std::wstring& value);
        static IPCMessage CreatePing();
        static IPCMessage CreateShutdown();
    };
}