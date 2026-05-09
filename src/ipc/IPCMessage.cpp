#include "ipc/IPCMessage.h"
#include "utils/string_utils.h"

namespace ShuTongWen
{
    std::string IPCMessage::Serialize() const
    {
        nlohmann::json j;
        j["type"] = static_cast<int>(type);
        j["data"] = StringUtils::UTF16ToUTF8(data);
        j["sender"] = StringUtils::UTF16ToUTF8(sender);
        j["timestamp"] = timestamp;
        return j.dump();
    }

    bool IPCMessage::Deserialize(const std::string& jsonStr)
    {
        try
        {
            nlohmann::json j = nlohmann::json::parse(jsonStr);
            type = static_cast<IPCMessageType>(j["type"].get<int>());
            data = StringUtils::UTF8ToUTF16(j["data"].get<std::string>());
            sender = StringUtils::UTF8ToUTF16(j["sender"].get<std::string>());
            timestamp = j["timestamp"].get<uint64_t>();
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool IPCMessage::Deserialize(const std::wstring& jsonStr)
    {
        return Deserialize(StringUtils::UTF16ToUTF8(jsonStr));
    }

    IPCMessage IPCMessageBuilder::CreateTextInput(const std::wstring& text)
    {
        IPCMessage msg;
        msg.type = IPCMessageType::TextInput;
        msg.data = text;
        msg.timestamp = GetTickCount64();
        return msg;
    }

    IPCMessage IPCMessageBuilder::CreateCandidateRequest(const std::wstring& pinyin)
    {
        IPCMessage msg;
        msg.type = IPCMessageType::CandidateRequest;
        msg.data = pinyin;
        msg.timestamp = GetTickCount64();
        return msg;
    }

    IPCMessage IPCMessageBuilder::CreateCandidateSelect(int index)
    {
        IPCMessage msg;
        msg.type = IPCMessageType::CandidateSelect;
        msg.data = std::to_wstring(index);
        msg.timestamp = GetTickCount64();
        return msg;
    }

    IPCMessage IPCMessageBuilder::CreateCompositionUpdate(const std::wstring& text)
    {
        IPCMessage msg;
        msg.type = IPCMessageType::CompositionUpdate;
        msg.data = text;
        msg.timestamp = GetTickCount64();
        return msg;
    }

    IPCMessage IPCMessageBuilder::CreateCompositionCommit()
    {
        IPCMessage msg;
        msg.type = IPCMessageType::CompositionCommit;
        msg.timestamp = GetTickCount64();
        return msg;
    }

    IPCMessage IPCMessageBuilder::CreateConfigRequest(const std::wstring& key)
    {
        IPCMessage msg;
        msg.type = IPCMessageType::ConfigRequest;
        msg.data = key;
        msg.timestamp = GetTickCount64();
        return msg;
    }

    IPCMessage IPCMessageBuilder::CreateConfigUpdate(const std::wstring& key, const std::wstring& value)
    {
        IPCMessage msg;
        msg.type = IPCMessageType::ConfigUpdate;
        msg.data = key + L"|" + value;
        msg.timestamp = GetTickCount64();
        return msg;
    }

    IPCMessage IPCMessageBuilder::CreatePing()
    {
        IPCMessage msg;
        msg.type = IPCMessageType::Ping;
        msg.timestamp = GetTickCount64();
        return msg;
    }

    IPCMessage IPCMessageBuilder::CreateShutdown()
    {
        IPCMessage msg;
        msg.type = IPCMessageType::Shutdown;
        msg.timestamp = GetTickCount64();
        return msg;
    }
}