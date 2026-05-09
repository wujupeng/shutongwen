#pragma once

#include <windows.h>
#include <msctf.h>

namespace ShuTongWen
{
    class CTFTextInputProcessorClassFactory : public IClassFactory
    {
    public:
        CTFTextInputProcessorClassFactory();
        ~CTFTextInputProcessorClassFactory();

        STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj) override;
        STDMETHOD_(ULONG, AddRef)() override;
        STDMETHOD_(ULONG, Release)() override;
        STDMETHOD(CreateInstance)(IUnknown* pUnkOuter, REFIID riid, void** ppvObj) override;
        STDMETHOD(LockServer)(BOOL fLock) override;

    private:
        LONG m_cRef;
    };

    HRESULT RegisterTextInputProcessor(HINSTANCE hInstance, const GUID& clsid);
    HRESULT UnregisterTextInputProcessor(const GUID& clsid);

    extern const GUID CLSID_ShuTongWenTextInputProcessor;
    extern const GUID IID_IShuTongWenTextInputProcessor;
}