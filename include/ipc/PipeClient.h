#pragma once

#include <windows.h>
#include <string>
#include <memory>

namespace ShuTongWen
{
    class PipeClient
    {
    public:
        PipeClient();
        ~PipeClient();

        HRESULT Connect(const std::wstring& pipeName);
        HRESULT Disconnect();

        HRESULT SendMessage(const std::wstring& message, std::wstring& response);
        HRESULT SendMessageAsync(const std::wstring& message);

        bool IsConnected() const { return m_connected; }

    private:
        HANDLE m_hPipe;
        bool m_connected;
        std::wstring m_pipeName;

        bool ConnectPipe();
    };
}