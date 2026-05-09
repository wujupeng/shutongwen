#include "ime/IMEModule.h"
#include "utils/logger.h"
#include "utils/win32_utils.h"
#include <objbase.h>

namespace ShuTongWen
{
    IMEModule::IMEModule()
        : m_registered(false)
    {}

    IMEModule::~IMEModule()
    {
        if (m_registered)
        {
            Unregister();
        }
    }

    HRESULT IMEModule::Register(HINSTANCE hInstance)
    {
        if (m_registered)
            return S_FALSE;

        Logger::Info("Registering IME module...");

        if (FAILED(GenerateCLSID(m_clsid)))
        {
            Logger::Error("Failed to generate CLSID");
            return E_FAIL;
        }

        if (FAILED(CreateRegistryEntries(hInstance)))
        {
            Logger::Error("Failed to create registry entries");
            return E_FAIL;
        }

        m_registered = true;
        Logger::Info("IME module registered successfully");
        return S_OK;
    }

    HRESULT IMEModule::Unregister()
    {
        if (!m_registered)
            return S_FALSE;

        Logger::Info("Unregistering IME module...");

        if (FAILED(DeleteRegistryEntries()))
        {
            Logger::Error("Failed to delete registry entries");
            return E_FAIL;
        }

        m_registered = false;
        m_clsid.clear();
        Logger::Info("IME module unregistered successfully");
        return S_OK;
    }

    HRESULT IMEModule::CreateRegistryEntries(HINSTANCE hInstance)
    {
        std::wstring modulePath = Win32Utils::GetModulePath(hInstance);
        std::wstring clsidStr = L"CLSID\\" + m_clsid;
        
        if (!Win32Utils::SetRegistryValue(HKEY_LOCAL_MACHINE, clsidStr, L"", L"ShuTongWen IME"))
            return E_FAIL;
        
        if (!Win32Utils::SetRegistryValue(HKEY_LOCAL_MACHINE, clsidStr + L"\\InprocServer32", L"", modulePath))
            return E_FAIL;
        
        if (!Win32Utils::SetRegistryValue(HKEY_LOCAL_MACHINE, clsidStr + L"\\InprocServer32", L"ThreadingModel", L"Apartment"))
            return E_FAIL;

        std::wstring imeKey = L"SOFTWARE\\Microsoft\\CTF\\TIP\\" + m_clsid;
        if (!Win32Utils::SetRegistryValue(HKEY_LOCAL_MACHINE, imeKey, L"", L"ShuTongWen Chinese IME"))
            return E_FAIL;
        
        if (!Win32Utils::SetRegistryValue(HKEY_LOCAL_MACHINE, imeKey + L"\\LanguageProfile\\0x00000804", L"", L"中文(简体)"))
            return E_FAIL;

        return S_OK;
    }

    HRESULT IMEModule::DeleteRegistryEntries()
    {
        std::wstring clsidStr = L"CLSID\\" + m_clsid;
        std::wstring imeKey = L"SOFTWARE\\Microsoft\\CTF\\TIP\\" + m_clsid;

        Win32Utils::DeleteRegistryValue(HKEY_LOCAL_MACHINE, clsidStr, L"");
        Win32Utils::DeleteRegistryValue(HKEY_LOCAL_MACHINE, imeKey, L"");

        return S_OK;
    }

    HRESULT IMEModule::GenerateCLSID(std::wstring& clsid)
    {
        GUID guid;
        if (FAILED(CoCreateGuid(&guid)))
            return E_FAIL;

        WCHAR buffer[64];
        StringFromGUID2(guid, buffer, _countof(buffer));
        clsid = buffer;
        return S_OK;
    }
}