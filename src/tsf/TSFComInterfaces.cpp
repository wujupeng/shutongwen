#include "tsf/TSFComInterfaces.h"
#include "tsf/TSFTextInputProcessor.h"
#include "utils/logger.h"

namespace ShuTongWen
{
    const GUID CLSID_ShuTongWenTextInputProcessor = 
        { 0x12345678, 0x1234, 0x5678, { 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF } };

    const GUID IID_IShuTongWenTextInputProcessor = 
        { 0x87654321, 0x4321, 0x8765, { 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10, 0xFE, 0xDC } };

    CTFTextInputProcessorClassFactory::CTFTextInputProcessorClassFactory()
        : m_cRef(1)
    {}

    CTFTextInputProcessorClassFactory::~CTFTextInputProcessorClassFactory()
    {}

    STDMETHODIMP CTFTextInputProcessorClassFactory::QueryInterface(REFIID riid, void** ppvObj)
    {
        *ppvObj = nullptr;

        if (riid == IID_IUnknown || riid == IID_IClassFactory)
        {
            *ppvObj = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) CTFTextInputProcessorClassFactory::AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHODIMP_(ULONG) CTFTextInputProcessorClassFactory::Release()
    {
        LONG cRef = InterlockedDecrement(&m_cRef);
        if (cRef == 0)
            delete this;
        return cRef;
    }

    STDMETHODIMP CTFTextInputProcessorClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObj)
    {
        *ppvObj = nullptr;

        if (pUnkOuter)
            return CLASS_E_NOAGGREGATION;

        TSFTextInputProcessor* pProcessor = new TSFTextInputProcessor();
        if (!pProcessor)
            return E_OUTOFMEMORY;

        HRESULT hr = pProcessor->QueryInterface(riid, ppvObj);
        pProcessor->Release();

        return hr;
    }

    STDMETHODIMP CTFTextInputProcessorClassFactory::LockServer(BOOL fLock)
    {
        return S_OK;
    }

    HRESULT RegisterTextInputProcessor(HINSTANCE hInstance, const GUID& clsid)
    {
        Logger::Info("Registering TSF Text Input Processor");

        WCHAR szCLSID[64];
        StringFromGUID2(clsid, szCLSID, _countof(szCLSID));

        std::wstring clsidStr = szCLSID;
        std::wstring modulePath;
        
        WCHAR path[MAX_PATH];
        GetModuleFileNameW(hInstance, path, MAX_PATH);
        modulePath = path;

        std::wstring clsidKey = L"CLSID\\" + clsidStr;
        
        HKEY hKey;
        LONG result = RegCreateKeyExW(HKEY_LOCAL_MACHINE, clsidKey.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
        if (result != ERROR_SUCCESS)
            return HRESULT_FROM_WIN32(result);

        RegSetValueExW(hKey, L"", 0, REG_SZ, reinterpret_cast<const BYTE*>(L"ShuTongWen Text Input Processor"), sizeof(L"ShuTongWen Text Input Processor"));
        RegCloseKey(hKey);

        std::wstring inprocKey = clsidKey + L"\\InprocServer32";
        result = RegCreateKeyExW(HKEY_LOCAL_MACHINE, inprocKey.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
        if (result != ERROR_SUCCESS)
            return HRESULT_FROM_WIN32(result);

        RegSetValueExW(hKey, L"", 0, REG_SZ, reinterpret_cast<const BYTE*>(modulePath.c_str()), static_cast<DWORD>((modulePath.length() + 1) * sizeof(WCHAR)));
        RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ, reinterpret_cast<const BYTE*>(L"Apartment"), sizeof(L"Apartment"));
        RegCloseKey(hKey);

        std::wstring tipKey = L"SOFTWARE\\Microsoft\\CTF\\TIP\\" + clsidStr;
        result = RegCreateKeyExW(HKEY_LOCAL_MACHINE, tipKey.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
        if (result != ERROR_SUCCESS)
            return HRESULT_FROM_WIN32(result);

        RegSetValueExW(hKey, L"", 0, REG_SZ, reinterpret_cast<const BYTE*>(L"ShuTongWen IME"), sizeof(L"ShuTongWen IME"));
        RegSetValueExW(hKey, L"Language", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&0x0804), sizeof(DWORD));
        RegCloseKey(hKey);

        std::wstring langProfileKey = tipKey + L"\\LanguageProfile\\0x00000804";
        result = RegCreateKeyExW(HKEY_LOCAL_MACHINE, langProfileKey.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
        if (result != ERROR_SUCCESS)
            return HRESULT_FROM_WIN32(result);

        RegSetValueExW(hKey, L"", 0, REG_SZ, reinterpret_cast<const BYTE*>(L"中文(简体) - ShuTongWen"), sizeof(L"中文(简体) - ShuTongWen"));
        RegCloseKey(hKey);

        Logger::Info("TSF Text Input Processor registered successfully");
        return S_OK;
    }

    HRESULT UnregisterTextInputProcessor(const GUID& clsid)
    {
        Logger::Info("Unregistering TSF Text Input Processor");

        WCHAR szCLSID[64];
        StringFromGUID2(clsid, szCLSID, _countof(szCLSID));

        std::wstring clsidStr = szCLSID;

        std::wstring tipKey = L"SOFTWARE\\Microsoft\\CTF\\TIP\\" + clsidStr;
        SHDeleteKeyW(HKEY_LOCAL_MACHINE, tipKey.c_str());

        std::wstring clsidKey = L"CLSID\\" + clsidStr;
        SHDeleteKeyW(HKEY_LOCAL_MACHINE, clsidKey.c_str());

        Logger::Info("TSF Text Input Processor unregistered successfully");
        return S_OK;
    }
}