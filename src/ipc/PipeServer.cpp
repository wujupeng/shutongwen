#include "ipc/PipeServer.h"
#include "utils/logger.h"

namespace ShuTongWen
{
    PipeServer::PipeServer()
        : m_hThread(nullptr),
          m_running(false)
    {}

    PipeServer::~PipeServer()
    {
        Stop();
    }

    HRESULT PipeServer::Start(const std::wstring& pipeName, MessageHandler handler)
    {
        if (m_running)
            return S_FALSE;

        Logger::Info("Starting pipe server: {}", StringUtils::UTF16ToUTF8(pipeName));

        m_pipeName = L"\\\\.\\pipe\\" + pipeName;
        m_handler = handler;
        m_running = true;

        m_hThread = CreateThread(
            nullptr,
            0,
            PipeThread,
            this,
            0,
            nullptr
        );

        if (!m_hThread)
        {
            Logger::Error("Failed to create pipe server thread");
            m_running = false;
            return E_FAIL;
        }

        Logger::Info("Pipe server started successfully");
        return S_OK;
    }

    HRESULT PipeServer::Stop()
    {
        if (!m_running)
            return S_FALSE;

        Logger::Info("Stopping pipe server...");

        m_running = false;

        HANDLE hPipe = CreateFileW(
            m_pipeName.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr
        );

        if (hPipe != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hPipe);
        }

        if (m_hThread)
        {
            WaitForSingleObject(m_hThread, 5000);
            CloseHandle(m_hThread);
            m_hThread = nullptr;
        }

        for (HANDLE hThread : m_clientThreads)
        {
            WaitForSingleObject(hThread, 1000);
            CloseHandle(hThread);
        }
        m_clientThreads.clear();

        Logger::Info("Pipe server stopped successfully");
        return S_OK;
    }

    DWORD WINAPI PipeServer::PipeThread(LPVOID lpParameter)
    {
        PipeServer* pServer = reinterpret_cast<PipeServer*>(lpParameter);
        if (!pServer)
            return 0;

        while (pServer->m_running)
        {
            HANDLE hPipe = CreateNamedPipeW(
                pServer->m_pipeName.c_str(),
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                4096,
                4096,
                5000,
                nullptr
            );

            if (hPipe == INVALID_HANDLE_VALUE)
            {
                Logger::Error("Failed to create named pipe");
                Sleep(1000);
                continue;
            }

            if (ConnectNamedPipe(hPipe, nullptr))
            {
                HANDLE hThread = CreateThread(
                    nullptr,
                    0,
                    ClientThread,
                    new std::pair<PipeServer*, HANDLE>(pServer, hPipe),
                    0,
                    nullptr
                );

                if (hThread)
                {
                    std::lock_guard<std::mutex> lock(pServer->m_mutex);
                    pServer->m_clientThreads.push_back(hThread);
                }
                else
                {
                    CloseHandle(hPipe);
                }
            }
            else
            {
                CloseHandle(hPipe);
            }
        }

        return 0;
    }

    DWORD WINAPI PipeServer::ClientThread(LPVOID lpParameter)
    {
        auto pData = reinterpret_cast<std::pair<PipeServer*, HANDLE>*>(lpParameter);
        PipeServer* pServer = pData->first;
        HANDLE hPipe = pData->second;
        delete pData;

        pServer->ProcessClient(hPipe);

        std::lock_guard<std::mutex> lock(pServer->m_mutex);
        auto it = std::find(pServer->m_clientThreads.begin(), pServer->m_clientThreads.end(), GetCurrentThread());
        if (it != pServer->m_clientThreads.end())
        {
            pServer->m_clientThreads.erase(it);
        }

        return 0;
    }

    bool PipeServer::ProcessClient(HANDLE hPipe)
    {
        std::wstring message;
        std::wstring response;

        while (ReadMessage(hPipe, message))
        {
            Logger::Debug("Received message: {}", StringUtils::UTF16ToUTF8(message));

            bool handled = false;
            if (m_handler)
            {
                handled = m_handler(message, response);
            }

            if (handled && !response.empty())
            {
                WriteMessage(hPipe, response);
            }
        }

        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
        return true;
    }

    bool PipeServer::ReadMessage(HANDLE hPipe, std::wstring& message)
    {
        DWORD bytesRead = 0;
        BYTE buffer[4096] = {0};

        if (!ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, nullptr))
        {
            return false;
        }

        if (bytesRead > 0)
        {
            message = reinterpret_cast<wchar_t*>(buffer);
            message.resize(bytesRead / sizeof(wchar_t));
        }

        return true;
    }

    bool PipeServer::WriteMessage(HANDLE hPipe, const std::wstring& message)
    {
        DWORD bytesWritten = 0;
        DWORD bytesToWrite = static_cast<DWORD>(message.length() * sizeof(wchar_t));

        return WriteFile(hPipe, message.c_str(), bytesToWrite, &bytesWritten, nullptr);
    }
}