#pragma once

#include <windows.h>
#include <string>

namespace ShuTongWen
{
    class IMEModule
    {
    public:
        IMEModule();
        ~IMEModule();

        HRESULT Register(HINSTANCE hInstance);
        HRESULT Unregister();

        bool IsRegistered() const { return m_registered; }
        const std::wstring& GetCLSID() const { return m_clsid; }

    private:
        bool m_registered;
        std::wstring m_clsid;

        HRESULT CreateRegistryEntries(HINSTANCE hInstance);
        HRESULT DeleteRegistryEntries();
        HRESULT GenerateCLSID(std::wstring& clsid);
    };
}