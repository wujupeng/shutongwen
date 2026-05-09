#pragma once

#include <windows.h>
#include <functional>
#include <string>
#include <memory>
#include <mutex>
#include <vector>

namespace ShuTongWen
{
    class PipeServer
    {
    public:
        using MessageHandler = std::function<bool(const std::wstring& message, std::wstring& response)>;

        PipeServer();
        ~PipeServer();

        HRESULT Start(const std::wstring& pipeName, MessageHandler handler);
        HRESULT Stop();

        bool IsRunning() const { return m_running; }

    private:
        static DWORD WINAPI PipeThread(LPVOID lpParameter);
        static DWORD WINAPI ClientThread(LPVOID lpParameter);

        bool ProcessClient(HANDLE hPipe);
        bool ReadMessage(HANDLE hPipe, std::wstring& message);
        bool WriteMessage(HANDLE hPipe, const std::wstring& message);

        std::wstring m_pipeName;
        MessageHandler m_handler;
        HANDLE m_hThread;
        bool m_running;
        std::mutex m_mutex;
        std::vector<HANDLE> m_clientThreads;
    };
}