#include "ime/IMEFramework.h"
#include "ime/IMEModule.h"
#include "utils/logger.h"
#include "utils/win32_utils.h"
#include <windows.h>

HINSTANCE g_hInstance = nullptr;
std::unique_ptr<ShuTongWen::IMEFramework> g_imeFramework = nullptr;
ShuTongWen::IMEModule g_imeModule;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hInstance = hModule;
        ShuTongWen::Logger::Initialize();
        ShuTongWen::Logger::Info("ShuTongWen IME DLL loaded");
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        if (g_imeFramework)
        {
            g_imeFramework->Uninitialize();
            g_imeFramework.reset();
        }
        ShuTongWen::Logger::Info("ShuTongWen IME DLL unloaded");
        ShuTongWen::Logger::Shutdown();
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) HRESULT InitializeIME()
{
    ShuTongWen::Logger::Info("Initializing IME...");

    if (!g_imeFramework)
    {
        g_imeFramework = ShuTongWen::IMEFramework::Create();
    }

    if (!g_imeFramework)
    {
        ShuTongWen::Logger::Error("Failed to create IME framework");
        return E_FAIL;
    }

    HRESULT hr = g_imeFramework->Initialize(g_hInstance);
    if (FAILED(hr))
    {
        ShuTongWen::Logger::Error("Failed to initialize IME framework");
        return hr;
    }

    ShuTongWen::Logger::Info("IME initialized successfully");
    return S_OK;
}

extern "C" __declspec(dllexport) HRESULT UninitializeIME()
{
    ShuTongWen::Logger::Info("Uninitializing IME...");

    if (g_imeFramework)
    {
        g_imeFramework->Uninitialize();
        g_imeFramework.reset();
    }

    ShuTongWen::Logger::Info("IME uninitialized successfully");
    return S_OK;
}

extern "C" __declspec(dllexport) HRESULT ProcessKey(HWND hWnd, WPARAM wParam, LPARAM lParam, bool isKeyDown)
{
    if (!g_imeFramework)
    {
        return E_NOT_VALID_STATE;
    }

    bool handled = false;
    HRESULT hr;

    if (isKeyDown)
    {
        hr = g_imeFramework->ProcessKeyDown(wParam, lParam, handled);
    }
    else
    {
        hr = g_imeFramework->ProcessKeyUp(wParam, lParam, handled);
    }

    return hr;
}

extern "C" __declspec(dllexport) HRESULT RegisterIME()
{
    ShuTongWen::Logger::Info("Registering IME...");
    return g_imeModule.Register(g_hInstance);
}

extern "C" __declspec(dllexport) HRESULT UnregisterIME()
{
    ShuTongWen::Logger::Info("Unregistering IME...");
    return g_imeModule.Unregister();
}