#include "ipc/PipeClient.h"
#include "utils/logger.h"

namespace ShuTongWen
{
    PipeClient::PipeClient()
        : m_hPipe(INVALID_HANDLE_VALUE),
          m_connected(false)
    {}

    PipeClient::~PipeClient()
    {
        Disconnect();
    }

    HRESULT PipeClient::Connect(const std::wstring& pipeName)
    {
        if (m_connected)
            return S_FALSE;

        m_pipeName = L"\\\\.\\pipe\\" + pipeName;

        if (ConnectPipe())
        {
            Logger::Info("Connected to pipe server: {}", StringUtils::UTF16ToUTF8(m_pipeName));
            return S_OK;
        }

        return E_FAIL;
    }

    HRESULT PipeClient::Disconnect()
    {
        if (!m_connected)
            return S_FALSE;

        if (m_hPipe != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_hPipe);
            m_hPipe = INVALID_HANDLE_VALUE;
        }

        m_connected = false;
        Logger::Info("Disconnected from pipe server");
        return S_OK;
    }

    HRESULT PipeClient::SendMessage(const std::wstring& message, std::wstring& response)
    {
        if (!m_connected)
        {
            if (!ConnectPipe())
                return E_NOT_CONNECTED;
        }

        DWORD bytesWritten = 0;
        DWORD bytesToWrite = static_cast<DWORD>(message.length() * sizeof(wchar_t));

        if (!WriteFile(m_hPipe, message.c_str(), bytesToWrite, &bytesWritten, nullptr))
        {
            Logger::Error("Failed to write to pipe");
            m_connected = false;
            return E_FAIL;
        }

        BYTE buffer[4096] = {0};
        DWORD bytesRead = 0;

        if (!ReadFile(m_hPipe, buffer, sizeof(buffer), &bytesRead, nullptr))
        {
            Logger::Error("Failed to read from pipe");
            m_connected = false;
            return E_FAIL;
        }

        if (bytesRead > 0)
        {
            response = reinterpret_cast<wchar_t*>(buffer);
            response.resize(bytesRead / sizeof(wchar_t));
        }

        return S_OK;
    }

    HRESULT PipeClient::SendMessageAsync(const std::wstring& message)
    {
        if (!m_connected)
        {
            if (!ConnectPipe())
                return E_NOT_CONNECTED;
        }

        DWORD bytesWritten = 0;
        DWORD bytesToWrite = static_cast<DWORD>(message.length() * sizeof(wchar_t));

        if (!WriteFile(m_hPipe, message.c_str(), bytesToWrite, &bytesWritten, nullptr))
        {
            Logger::Error("Failed to write to pipe");
            m_connected = false;
            return E_FAIL;
        }

        return S_OK;
    }

    bool PipeClient::ConnectPipe()
    {
        m_hPipe = CreateFileW(
            m_pipeName.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr
        );

        if (m_hPipe == INVALID_HANDLE_VALUE)
        {
            Logger::Error("Failed to connect to pipe: {}", StringUtils::UTF16ToUTF8(m_pipeName));
            return false;
        }

        DWORD mode = PIPE_READMODE_MESSAGE;
        if (!SetNamedPipeHandleState(m_hPipe, &mode, nullptr, nullptr))
        {
            CloseHandle(m_hPipe);
            m_hPipe = INVALID_HANDLE_VALUE;
            return false;
        }

        m_connected = true;
        return true;
    }
}