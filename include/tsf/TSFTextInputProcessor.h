#pragma once

#include <windows.h>
#include <msctf.h>
#include <memory>
#include <vector>
#include <string>

namespace ShuTongWen
{
    class TSFTextInputProcessor : 
        public ITfTextInputProcessor,
        public ITfThreadMgrEventSink,
        public ITfTextEditSink,
        public ITfKeyEventSink,
        public ITfCompositionSink,
        public ITfDisplayAttributeProvider
    {
    public:
        TSFTextInputProcessor();
        virtual ~TSFTextInputProcessor();

        // IUnknown
        STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj) override;
        STDMETHOD_(ULONG, AddRef)() override;
        STDMETHOD_(ULONG, Release)() override;

        // ITfTextInputProcessor
        STDMETHOD(Activate)(ITfThreadMgr* pThreadMgr, TfClientId tfClientId) override;
        STDMETHOD(Deactivate)() override;
        STDMETHOD(IsEnabled)(BOOL* pbEnabled) override;

        // ITfThreadMgrEventSink
        STDMETHOD(OnInitDocumentMgr)(ITfDocumentMgr* pDocMgr) override;
        STDMETHOD(OnUninitDocumentMgr)(ITfDocumentMgr* pDocMgr) override;
        STDMETHOD(OnSetFocus)(ITfDocumentMgr* pDocMgrFocus, ITfDocumentMgr* pDocMgrPrevFocus) override;
        STDMETHOD(OnPushContext)(ITfContext* pContext) override;
        STDMETHOD(OnPopContext)(ITfContext* pContext) override;

        // ITfTextEditSink
        STDMETHOD(OnEndEdit)(ITfContext* pContext, TfEditCookie ecReadOnly, ITfEditRecord* pEditRecord) override;

        // ITfKeyEventSink
        STDMETHOD(OnSetFocus)(BOOL fForeground) override;
        STDMETHOD(OnTestKeyDown)(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) override;
        STDMETHOD(OnKeyDown)(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) override;
        STDMETHOD(OnTestKeyUp)(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) override;
        STDMETHOD(OnKeyUp)(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) override;

        // ITfCompositionSink
        STDMETHOD(OnCompositionTerminated)(ITfComposition* pComposition) override;

        // ITfDisplayAttributeProvider
        STDMETHOD(EnumDisplayAttributeInfo)(IEnumTfDisplayAttributeInfo** ppEnum) override;
        STDMETHOD(GetDisplayAttributeInfo)(REFGUID guid, ITfDisplayAttributeInfo** ppInfo) override;
        STDMETHOD(RegisterDisplayAttributeInfo)(const TF_DISPLAYATTRIBUTE* pAttr, TfGuidAtom* pAtom) override;

        HRESULT Initialize(HINSTANCE hInstance);
        HRESULT Uninitialize();

    private:
        LONG m_cRef;
        ITfThreadMgr* m_pThreadMgr;
        TfClientId m_tfClientId;
        DWORD m_dwThreadMgrEventSinkCookie;
        DWORD m_dwTextEditSinkCookie;
        DWORD m_dwKeyEventSinkCookie;
        DWORD m_dwCompositionSinkCookie;

        ITfContext* m_pCurrentContext;
        ITfComposition* m_pComposition;

        std::wstring m_compositionText;
        std::vector<std::wstring> m_candidates;
        int m_selectedCandidateIndex;

        HRESULT CreateComposition(ITfContext* pContext);
        HRESULT UpdateComposition(ITfContext* pContext, const std::wstring& text);
        HRESULT CommitComposition(ITfContext* pContext);
        HRESULT CancelComposition(ITfContext* pContext);

        HRESULT GetCaretPosition(ITfContext* pContext, POINT& pt);
        HRESULT ShowCandidateWindow(ITfContext* pContext);
        HRESULT HideCandidateWindow();
    };
}