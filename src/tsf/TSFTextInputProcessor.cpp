#include "tsf/TSFTextInputProcessor.h"
#include "utils/logger.h"

namespace ShuTongWen
{
    TSFTextInputProcessor::TSFTextInputProcessor()
        : m_cRef(1),
          m_pThreadMgr(nullptr),
          m_tfClientId(0),
          m_dwThreadMgrEventSinkCookie(0),
          m_dwTextEditSinkCookie(0),
          m_dwKeyEventSinkCookie(0),
          m_dwCompositionSinkCookie(0),
          m_pCurrentContext(nullptr),
          m_pComposition(nullptr),
          m_selectedCandidateIndex(0)
    {}

    TSFTextInputProcessor::~TSFTextInputProcessor()
    {
        Uninitialize();
    }

    STDMETHODIMP TSFTextInputProcessor::QueryInterface(REFIID riid, void** ppvObj)
    {
        *ppvObj = nullptr;

        if (riid == IID_IUnknown)
            *ppvObj = static_cast<IUnknown*>(this);
        else if (riid == IID_ITfTextInputProcessor)
            *ppvObj = static_cast<ITfTextInputProcessor*>(this);
        else if (riid == IID_ITfThreadMgrEventSink)
            *ppvObj = static_cast<ITfThreadMgrEventSink*>(this);
        else if (riid == IID_ITfTextEditSink)
            *ppvObj = static_cast<ITfTextEditSink*>(this);
        else if (riid == IID_ITfKeyEventSink)
            *ppvObj = static_cast<ITfKeyEventSink*>(this);
        else if (riid == IID_ITfCompositionSink)
            *ppvObj = static_cast<ITfCompositionSink*>(this);
        else if (riid == IID_ITfDisplayAttributeProvider)
            *ppvObj = static_cast<ITfDisplayAttributeProvider*>(this);

        if (*ppvObj)
        {
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) TSFTextInputProcessor::AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHODIMP_(ULONG) TSFTextInputProcessor::Release()
    {
        LONG cRef = InterlockedDecrement(&m_cRef);
        if (cRef == 0)
            delete this;
        return cRef;
    }

    STDMETHODIMP TSFTextInputProcessor::Activate(ITfThreadMgr* pThreadMgr, TfClientId tfClientId)
    {
        Logger::Info("TSF TextInputProcessor activated");

        m_pThreadMgr = pThreadMgr;
        m_tfClientId = tfClientId;
        m_pThreadMgr->AddRef();

        IID iid = IID_ITfThreadMgrEventSink;
        m_pThreadMgr->AdviseSink(&iid, this, &m_dwThreadMgrEventSinkCookie);

        return S_OK;
    }

    STDMETHODIMP TSFTextInputProcessor::Deactivate()
    {
        Logger::Info("TSF TextInputProcessor deactivated");

        if (m_pThreadMgr)
        {
            m_pThreadMgr->UnadviseSink(m_dwThreadMgrEventSinkCookie);
            m_pThreadMgr->Release();
            m_pThreadMgr = nullptr;
        }

        CancelComposition(m_pCurrentContext);
        return S_OK;
    }

    STDMETHODIMP TSFTextInputProcessor::IsEnabled(BOOL* pbEnabled)
    {
        *pbEnabled = TRUE;
        return S_OK;
    }

    STDMETHODIMP TSFTextInputProcessor::OnInitDocumentMgr(ITfDocumentMgr* pDocMgr)
    {
        Logger::Debug("OnInitDocumentMgr");
        return S_OK;
    }

    STDMETHODIMP TSFTextInputProcessor::OnUninitDocumentMgr(ITfDocumentMgr* pDocMgr)
    {
        Logger::Debug("OnUninitDocumentMgr");
        return S_OK;
    }

    STDMETHODIMP TSFTextInputProcessor::OnSetFocus(ITfDocumentMgr* pDocMgrFocus, ITfDocumentMgr* pDocMgrPrevFocus)
    {
        Logger::Debug("OnSetFocus");

        if (pDocMgrFocus)
        {
            pDocMgrFocus->GetTop(&m_pCurrentContext);
        }

        return S_OK;
    }

    STDMETHODIMP TSFTextInputProcessor::OnPushContext(ITfContext* pContext)
    {
        Logger::Debug("OnPushContext");
        return S_OK;
    }

    STDMETHODIMP TSFTextInputProcessor::OnPopContext(ITfContext* pContext)
    {
        Logger::Debug("OnPopContext");
        return S_OK;
    }

    STDMETHODIMP TSFTextInputProcessor::OnEndEdit(ITfContext* pContext, TfEditCookie ecReadOnly, ITfEditRecord* pEditRecord)
    {
        Logger::Debug("OnEndEdit");
        return S_OK;
    }

    STDMETHODIMP TSFTextInputProcessor::OnSetFocus(BOOL fForeground)
    {
        Logger::Debug("OnSetFocus (keyboard)");
        return S_OK;
    }

    STDMETHODIMP TSFTextInputProcessor::OnTestKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
    {
        *pfEaten = FALSE;

        if (wParam >= 'A' && wParam <= 'Z')
        {
            *pfEaten = TRUE;
        }

        return S_OK;
    }

    STDMETHODIMP TSFTextInputProcessor::OnKeyDown(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
    {
        *pfEaten = FALSE;

        if (wParam >= 'A' && wParam <= 'Z')
        {
            wchar_t ch = static_cast<wchar_t>(wParam);
            m_compositionText += ch;

            UpdateComposition(pContext, m_compositionText);
            *pfEaten = TRUE;
        }
        else if (wParam == VK_SPACE)
        {
            ShowCandidateWindow(pContext);
            *pfEaten = TRUE;
        }
        else if (wParam == VK_RETURN)
        {
            CommitComposition(pContext);
            *pfEaten = TRUE;
        }
        else if (wParam == VK_ESCAPE)
        {
            CancelComposition(pContext);
            *pfEaten = TRUE;
        }
        else if (wParam == VK_BACK)
        {
            if (!m_compositionText.empty())
            {
                m_compositionText.pop_back();
                UpdateComposition(pContext, m_compositionText);
            }
            *pfEaten = TRUE;
        }

        return S_OK;
    }

    STDMETHODIMP TSFTextInputProcessor::OnTestKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
    {
        *pfEaten = FALSE;
        return S_OK;
    }

    STDMETHODIMP TSFTextInputProcessor::OnKeyUp(ITfContext* pContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten)
    {
        *pfEaten = FALSE;
        return S_OK;
    }

    STDMETHODIMP TSFTextInputProcessor::OnCompositionTerminated(ITfComposition* pComposition)
    {
        Logger::Debug("OnCompositionTerminated");
        
        if (m_pComposition == pComposition)
        {
            m_pComposition->Release();
            m_pComposition = nullptr;
        }

        return S_OK;
    }

    STDMETHODIMP TSFTextInputProcessor::EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo** ppEnum)
    {
        *ppEnum = nullptr;
        return E_NOTIMPL;
    }

    STDMETHODIMP TSFTextInputProcessor::GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo** ppInfo)
    {
        *ppInfo = nullptr;
        return E_NOTIMPL;
    }

    STDMETHODIMP TSFTextInputProcessor::RegisterDisplayAttributeInfo(const TF_DISPLAYATTRIBUTE* pAttr, TfGuidAtom* pAtom)
    {
        return E_NOTIMPL;
    }

    HRESULT TSFTextInputProcessor::Initialize(HINSTANCE hInstance)
    {
        Logger::Info("Initializing TSF TextInputProcessor...");
        return S_OK;
    }

    HRESULT TSFTextInputProcessor::Uninitialize()
    {
        Logger::Info("Uninitializing TSF TextInputProcessor...");
        
        if (m_pComposition)
        {
            m_pComposition->Release();
            m_pComposition = nullptr;
        }

        if (m_pCurrentContext)
        {
            m_pCurrentContext->Release();
            m_pCurrentContext = nullptr;
        }

        return S_OK;
    }

    HRESULT TSFTextInputProcessor::CreateComposition(ITfContext* pContext)
    {
        if (!pContext)
            return E_INVALIDARG;

        TfEditCookie ec;
        HRESULT hr = pContext->RequestEditSession(m_tfClientId, &ec, TF_ES_READ | TF_ES_WRITE, nullptr);
        if (FAILED(hr))
            return hr;

        ITfInsertAtSelection* pInsert = nullptr;
        hr = pContext->QueryInterface(IID_ITfInsertAtSelection, reinterpret_cast<void**>(&pInsert));
        if (FAILED(hr))
            return hr;

        hr = pInsert->InsertTextAtSelection(ec, TF_IAS_NOQUERY, L"", 0, nullptr);
        pInsert->Release();

        return S_OK;
    }

    HRESULT TSFTextInputProcessor::UpdateComposition(ITfContext* pContext, const std::wstring& text)
    {
        if (!pContext || text.empty())
            return E_INVALIDARG;

        TfEditCookie ec;
        HRESULT hr = pContext->RequestEditSession(m_tfClientId, &ec, TF_ES_READ | TF_ES_WRITE, nullptr);
        if (FAILED(hr))
            return hr;

        if (!m_pComposition)
        {
            hr = CreateComposition(pContext);
            if (FAILED(hr))
                return hr;
        }

        ITfRange* pRange = nullptr;
        hr = m_pComposition->GetRange(&pRange);
        if (FAILED(hr))
            return hr;

        hr = pRange->SetText(ec, 0, text.c_str(), static_cast<LONG>(text.length()));
        pRange->Release();

        return S_OK;
    }

    HRESULT TSFTextInputProcessor::CommitComposition(ITfContext* pContext)
    {
        if (!pContext || !m_pComposition)
            return E_INVALIDARG;

        TfEditCookie ec;
        HRESULT hr = pContext->RequestEditSession(m_tfClientId, &ec, TF_ES_READ | TF_ES_WRITE, nullptr);
        if (FAILED(hr))
            return hr;

        hr = m_pComposition->EndComposition(ec);
        m_pComposition->Release();
        m_pComposition = nullptr;
        m_compositionText.clear();

        HideCandidateWindow();
        return hr;
    }

    HRESULT TSFTextInputProcessor::CancelComposition(ITfContext* pContext)
    {
        if (!m_pComposition)
            return S_OK;

        TfEditCookie ec;
        HRESULT hr = pContext->RequestEditSession(m_tfClientId, &ec, TF_ES_READ | TF_ES_WRITE, nullptr);
        if (FAILED(hr))
            return hr;

        hr = m_pComposition->EndComposition(ec);
        m_pComposition->Release();
        m_pComposition = nullptr;
        m_compositionText.clear();

        HideCandidateWindow();
        return hr;
    }

    HRESULT TSFTextInputProcessor::GetCaretPosition(ITfContext* pContext, POINT& pt)
    {
        pt.x = 0;
        pt.y = 0;

        if (!pContext)
            return E_INVALIDARG;

        ITfRange* pRange = nullptr;
        HRESULT hr = pContext->GetSelection(0, TF_DEFAULT_SELECTION, &pRange);
        if (FAILED(hr))
            return hr;

        LONG start = 0;
        hr = pRange->Start(&start);
        
        ITfContextView* pView = nullptr;
        hr = pContext->GetActiveView(&pView);
        if (SUCCEEDED(hr))
        {
            RECT rect;
            hr = pView->GetTextExt(ec, pRange, &rect);
            pt.x = rect.left;
            pt.y = rect.top;
            pView->Release();
        }

        pRange->Release();
        return hr;
    }

    HRESULT TSFTextInputProcessor::ShowCandidateWindow(ITfContext* pContext)
    {
        Logger::Debug("Showing candidate window");
        m_candidates = { L"你好", L"您好", L"泥好", L"拟好" };
        return S_OK;
    }

    HRESULT TSFTextInputProcessor::HideCandidateWindow()
    {
        Logger::Debug("Hiding candidate window");
        return S_OK;
    }
}