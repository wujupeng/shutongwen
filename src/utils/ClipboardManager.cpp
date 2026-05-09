#include "utils/ClipboardManager.h"
#include "utils/logger.h"
#include <algorithm>

namespace ShuTongWen
{
    ClipboardManager::ClipboardManager()
        : m_hWnd(nullptr),
          m_monitoring(false),
          m_maxHistorySize(100)
    {
        m_formatMap[ClipboardDataType::Text] = CF_TEXT;
        m_formatMap[ClipboardDataType::UnicodeText] = CF_UNICODETEXT;
        m_formatMap[ClipboardDataType::File] = CF_HDROP;
        m_formatMap[ClipboardDataType::Image] = CF_BITMAP;
    }

    ClipboardManager& ClipboardManager::Instance()
    {
        static ClipboardManager instance;
        return instance;
    }

    bool ClipboardManager::Initialize(HWND hWnd)
    {
        Logger::Info("Initializing ClipboardManager...");

        m_hWnd = hWnd;

        WNDCLASSEXW wc = {0};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"ShuTongWen_ClipboardWindow";

        if (!RegisterClassExW(&wc))
        {
            Logger::Error("Failed to register clipboard window class");
            return false;
        }

        Logger::Info("ClipboardManager initialized successfully");
        return true;
    }

    void ClipboardManager::Uninitialize()
    {
        Logger::Info("Uninitializing ClipboardManager...");
        StopMonitoring();
        m_history.clear();
        UnregisterClassW(L"ShuTongWen_ClipboardWindow", GetModuleHandleW(nullptr));
    }

    bool ClipboardManager::StartMonitoring()
    {
        if (m_monitoring)
            return true;

        if (!m_hWnd)
        {
            m_hWnd = CreateWindowExW(
                0,
                L"ShuTongWen_ClipboardWindow",
                L"",
                0,
                0, 0, 0, 0,
                nullptr,
                nullptr,
                GetModuleHandleW(nullptr),
                this
            );

            if (!m_hWnd)
            {
                Logger::Error("Failed to create clipboard window");
                return false;
            }
        }

        if (!AddClipboardFormatListener(m_hWnd))
        {
            Logger::Error("Failed to add clipboard format listener");
            return false;
        }

        m_monitoring = true;
        Logger::Info("Clipboard monitoring started");
        return true;
    }

    bool ClipboardManager::StopMonitoring()
    {
        if (!m_monitoring)
            return true;

        if (m_hWnd)
        {
            RemoveClipboardFormatListener(m_hWnd);
        }

        m_monitoring = false;
        Logger::Info("Clipboard monitoring stopped");
        return true;
    }

    std::vector<ClipboardItem> ClipboardManager::GetHistory(size_t limit)
    {
        std::vector<ClipboardItem> result;
        size_t count = std::min(limit, m_history.size());
        result.resize(count);
        std::reverse_copy(m_history.begin(), m_history.begin() + count, result.begin());
        return result;
    }

    bool ClipboardManager::GetLatestItem(ClipboardItem& item)
    {
        if (m_history.empty())
            return false;

        item = m_history.back();
        return true;
    }

    bool ClipboardManager::CopyToClipboard(const std::wstring& text)
    {
        if (!OpenClipboard(nullptr))
            return false;

        EmptyClipboard();

        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(wchar_t));
        if (!hGlobal)
        {
            CloseClipboard();
            return false;
        }

        wchar_t* pBuffer = reinterpret_cast<wchar_t*>(GlobalLock(hGlobal));
        if (!pBuffer)
        {
            GlobalFree(hGlobal);
            CloseClipboard();
            return false;
        }

        wcscpy_s(pBuffer, text.length() + 1, text.c_str());
        GlobalUnlock(hGlobal);

        SetClipboardData(CF_UNICODETEXT, hGlobal);
        CloseClipboard();

        return true;
    }

    bool ClipboardManager::PasteFromClipboard(std::wstring& text)
    {
        if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
            return false;

        if (!OpenClipboard(nullptr))
            return false;

        HGLOBAL hGlobal = GetClipboardData(CF_UNICODETEXT);
        if (!hGlobal)
        {
            CloseClipboard();
            return false;
        }

        wchar_t* pBuffer = reinterpret_cast<wchar_t*>(GlobalLock(hGlobal));
        if (!pBuffer)
        {
            CloseClipboard();
            return false;
        }

        text = pBuffer;
        GlobalUnlock(hGlobal);
        CloseClipboard();

        return true;
    }

    bool ClipboardManager::CopyImageToClipboard(const std::vector<uint8_t>& imageData)
    {
        if (imageData.empty())
            return false;

        if (!OpenClipboard(nullptr))
            return false;

        EmptyClipboard();

        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, imageData.size());
        if (!hGlobal)
        {
            CloseClipboard();
            return false;
        }

        void* pBuffer = GlobalLock(hGlobal);
        if (!pBuffer)
        {
            GlobalFree(hGlobal);
            CloseClipboard();
            return false;
        }

        memcpy(pBuffer, imageData.data(), imageData.size());
        GlobalUnlock(hGlobal);

        SetClipboardData(CF_DIB, hGlobal);
        CloseClipboard();

        return true;
    }

    bool ClipboardManager::PasteImageFromClipboard(std::vector<uint8_t>& imageData)
    {
        if (!IsClipboardFormatAvailable(CF_DIB) && !IsClipboardFormatAvailable(CF_BITMAP))
            return false;

        if (!OpenClipboard(nullptr))
            return false;

        UINT format = IsClipboardFormatAvailable(CF_DIB) ? CF_DIB : CF_BITMAP;
        HGLOBAL hGlobal = GetClipboardData(format);
        if (!hGlobal)
        {
            CloseClipboard();
            return false;
        }

        void* pBuffer = GlobalLock(hGlobal);
        if (!pBuffer)
        {
            CloseClipboard();
            return false;
        }

        DWORD size = GlobalSize(hGlobal);
        imageData.resize(size);
        memcpy(imageData.data(), pBuffer, size);

        GlobalUnlock(hGlobal);
        CloseClipboard();

        return true;
    }

    bool ClipboardManager::CopyHtmlToClipboard(const std::wstring& html)
    {
        if (!OpenClipboard(nullptr))
            return false;

        EmptyClipboard();

        UINT htmlFormat = RegisterClipboardFormatW(L"HTML Format");
        if (htmlFormat == 0)
        {
            CloseClipboard();
            return false;
        }

        std::wstring header = L"Version:0.9\r\nStartHTML:00000000\r\nEndHTML:00000000\r\nStartFragment:00000000\r\nEndFragment:00000000\r\nStartSelection:00000000\r\nEndSelection:00000000\r\n";
        std::wstring fullHtml = header + L"<html><body>" + html + L"</body></html>";

        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, (fullHtml.length() + 1) * sizeof(wchar_t));
        if (!hGlobal)
        {
            CloseClipboard();
            return false;
        }

        wchar_t* pBuffer = reinterpret_cast<wchar_t*>(GlobalLock(hGlobal));
        if (!pBuffer)
        {
            GlobalFree(hGlobal);
            CloseClipboard();
            return false;
        }

        wcscpy_s(pBuffer, fullHtml.length() + 1, fullHtml.c_str());
        GlobalUnlock(hGlobal);

        SetClipboardData(htmlFormat, hGlobal);
        CloseClipboard();

        return true;
    }

    bool ClipboardManager::PasteHtmlFromClipboard(std::wstring& html)
    {
        UINT htmlFormat = RegisterClipboardFormatW(L"HTML Format");
        if (htmlFormat == 0 || !IsClipboardFormatAvailable(htmlFormat))
            return false;

        if (!OpenClipboard(nullptr))
            return false;

        HGLOBAL hGlobal = GetClipboardData(htmlFormat);
        if (!hGlobal)
        {
            CloseClipboard();
            return false;
        }

        wchar_t* pBuffer = reinterpret_cast<wchar_t*>(GlobalLock(hGlobal));
        if (!pBuffer)
        {
            CloseClipboard();
            return false;
        }

        std::wstring fullHtml = pBuffer;
        size_t bodyStart = fullHtml.find(L"<body>");
        size_t bodyEnd = fullHtml.find(L"</body>");

        if (bodyStart != std::wstring::npos && bodyEnd != std::wstring::npos)
        {
            html = fullHtml.substr(bodyStart + 6, bodyEnd - bodyStart - 6);
        }
        else
        {
            html = fullHtml;
        }

        GlobalUnlock(hGlobal);
        CloseClipboard();

        return true;
    }

    bool ClipboardManager::CopyRtfToClipboard(const std::wstring& rtf)
    {
        if (!OpenClipboard(nullptr))
            return false;

        EmptyClipboard();

        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, (rtf.length() + 1) * sizeof(wchar_t));
        if (!hGlobal)
        {
            CloseClipboard();
            return false;
        }

        wchar_t* pBuffer = reinterpret_cast<wchar_t*>(GlobalLock(hGlobal));
        if (!pBuffer)
        {
            GlobalFree(hGlobal);
            CloseClipboard();
            return false;
        }

        wcscpy_s(pBuffer, rtf.length() + 1, rtf.c_str());
        GlobalUnlock(hGlobal);

        SetClipboardData(CF_RTF, hGlobal);
        CloseClipboard();

        return true;
    }

    bool ClipboardManager::PasteRtfFromClipboard(std::wstring& rtf)
    {
        if (!IsClipboardFormatAvailable(CF_RTF))
            return false;

        if (!OpenClipboard(nullptr))
            return false;

        HGLOBAL hGlobal = GetClipboardData(CF_RTF);
        if (!hGlobal)
        {
            CloseClipboard();
            return false;
        }

        wchar_t* pBuffer = reinterpret_cast<wchar_t*>(GlobalLock(hGlobal));
        if (!pBuffer)
        {
            CloseClipboard();
            return false;
        }

        rtf = pBuffer;
        GlobalUnlock(hGlobal);
        CloseClipboard();

        return true;
    }

    void ClipboardManager::ClearHistory()
    {
        m_history.clear();
        Logger::Info("Clipboard history cleared");
    }

    size_t ClipboardManager::GetHistoryCount() const
    {
        return m_history.size();
    }

    bool ClipboardManager::IsFormatAvailable(ClipboardDataType type) const
    {
        auto it = m_formatMap.find(type);
        if (it != m_formatMap.end())
        {
            return IsClipboardFormatAvailable(it->second) != FALSE;
        }
        return false;
    }

    void ClipboardManager::SetClipboardChangedCallback(ClipboardChangedCallback callback)
    {
        m_callback = callback;
    }

    LRESULT CALLBACK ClipboardManager::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_CLIPBOARDUPDATE:
        {
            ClipboardManager& instance = ClipboardManager::Instance();
            instance.OnClipboardChanged();
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(hWnd, uMsg, wParam, lParam);
        }
    }

    void ClipboardManager::OnClipboardChanged()
    {
        ClipboardItem item;
        if (ReadClipboardData(item))
        {
            if (!m_history.empty())
            {
                const ClipboardItem& last = m_history.back();
                if (last.type == item.type && last.text == item.text)
                {
                    return;
                }
            }

            m_history.push_back(item);

            if (m_history.size() > m_maxHistorySize)
            {
                m_history.erase(m_history.begin());
            }

            if (m_callback)
            {
                m_callback(item);
            }

            Logger::Debug("Clipboard changed: type={}, text_length={}", 
                static_cast<int>(item.type), item.text.length());
        }
    }

    ClipboardDataType ClipboardManager::DetectClipboardFormat() const
    {
        if (IsClipboardFormatAvailable(CF_UNICODETEXT))
            return ClipboardDataType::UnicodeText;
        if (IsClipboardFormatAvailable(CF_TEXT))
            return ClipboardDataType::Text;
        if (IsClipboardFormatAvailable(CF_HDROP))
            return ClipboardDataType::File;
        if (IsClipboardFormatAvailable(CF_DIB) || IsClipboardFormatAvailable(CF_BITMAP))
            return ClipboardDataType::Image;
        
        UINT htmlFormat = RegisterClipboardFormatW(L"HTML Format");
        if (htmlFormat != 0 && IsClipboardFormatAvailable(htmlFormat))
            return ClipboardDataType::Html;
        
        if (IsClipboardFormatAvailable(CF_RTF))
            return ClipboardDataType::Rtf;

        return ClipboardDataType::Unknown;
    }

    bool ClipboardManager::ReadClipboardData(ClipboardItem& item)
    {
        if (!OpenClipboard(nullptr))
            return false;

        item.timestamp = GetTickCount64();
        item.type = DetectClipboardFormat();

        switch (item.type)
        {
        case ClipboardDataType::UnicodeText:
        case ClipboardDataType::Text:
            ReadTextFromClipboard(item.text);
            item.size = item.text.length() * sizeof(wchar_t);
            break;

        case ClipboardDataType::File:
            ReadFilesFromClipboard(item.files);
            for (const auto& file : item.files)
                item.size += file.length() * sizeof(wchar_t);
            break;

        case ClipboardDataType::Image:
            ReadImageFromClipboard(item.imageData);
            item.size = item.imageData.size();
            break;

        case ClipboardDataType::Html:
            ReadHtmlFromClipboard(item.html);
            item.text = item.html;
            item.size = item.html.length() * sizeof(wchar_t);
            break;

        case ClipboardDataType::Rtf:
            ReadRtfFromClipboard(item.rtf);
            item.text = item.rtf;
            item.size = item.rtf.length() * sizeof(wchar_t);
            break;
        }

        CloseClipboard();

        return item.type != ClipboardDataType::Unknown;
    }

    bool ClipboardManager::ReadTextFromClipboard(std::wstring& text)
    {
        HGLOBAL hGlobal = GetClipboardData(CF_UNICODETEXT);
        if (!hGlobal)
        {
            hGlobal = GetClipboardData(CF_TEXT);
            if (!hGlobal)
                return false;
        }

        wchar_t* pBuffer = reinterpret_cast<wchar_t*>(GlobalLock(hGlobal));
        if (!pBuffer)
            return false;

        text = pBuffer;
        GlobalUnlock(hGlobal);
        return true;
    }

    bool ClipboardManager::ReadFilesFromClipboard(std::vector<std::wstring>& files)
    {
        HDROP hDrop = reinterpret_cast<HDROP>(GetClipboardData(CF_HDROP));
        if (!hDrop)
            return false;

        UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
        files.resize(fileCount);

        for (UINT i = 0; i < fileCount; ++i)
        {
            UINT size = DragQueryFileW(hDrop, i, nullptr, 0);
            std::vector<wchar_t> buffer(size + 1);
            DragQueryFileW(hDrop, i, buffer.data(), size + 1);
            files[i] = buffer.data();
        }

        DragFinish(hDrop);
        return true;
    }

    bool ClipboardManager::ReadImageFromClipboard(std::vector<uint8_t>& imageData)
    {
        UINT format = IsClipboardFormatAvailable(CF_DIB) ? CF_DIB : CF_BITMAP;
        HGLOBAL hGlobal = GetClipboardData(format);
        if (!hGlobal)
            return false;

        void* pBuffer = GlobalLock(hGlobal);
        if (!pBuffer)
            return false;

        DWORD size = GlobalSize(hGlobal);
        imageData.resize(size);
        memcpy(imageData.data(), pBuffer, size);

        GlobalUnlock(hGlobal);
        return true;
    }

    bool ClipboardManager::ReadHtmlFromClipboard(std::wstring& html)
    {
        UINT htmlFormat = RegisterClipboardFormatW(L"HTML Format");
        if (htmlFormat == 0)
            return false;

        HGLOBAL hGlobal = GetClipboardData(htmlFormat);
        if (!hGlobal)
            return false;

        wchar_t* pBuffer = reinterpret_cast<wchar_t*>(GlobalLock(hGlobal));
        if (!pBuffer)
            return false;

        std::wstring fullHtml = pBuffer;
        size_t bodyStart = fullHtml.find(L"<body>");
        size_t bodyEnd = fullHtml.find(L"</body>");

        if (bodyStart != std::wstring::npos && bodyEnd != std::wstring::npos)
        {
            html = fullHtml.substr(bodyStart + 6, bodyEnd - bodyStart - 6);
        }
        else
        {
            html = fullHtml;
        }

        GlobalUnlock(hGlobal);
        return true;
    }

    bool ClipboardManager::ReadRtfFromClipboard(std::wstring& rtf)
    {
        HGLOBAL hGlobal = GetClipboardData(CF_RTF);
        if (!hGlobal)
            return false;

        wchar_t* pBuffer = reinterpret_cast<wchar_t*>(GlobalLock(hGlobal));
        if (!pBuffer)
            return false;

        rtf = pBuffer;
        GlobalUnlock(hGlobal);
        return true;
    }
}