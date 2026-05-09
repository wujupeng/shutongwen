#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <map>

namespace ShuTongWen
{
    enum class ClipboardDataType
    {
        Unknown,
        Text,
        UnicodeText,
        File,
        Image,
        Html,
        Rtf,
        Custom
    };

    struct ClipboardItem
    {
        ClipboardDataType type;
        std::wstring text;
        std::vector<std::wstring> files;
        std::vector<uint8_t> imageData;
        std::wstring html;
        std::wstring rtf;
        std::wstring customFormat;
        uint64_t timestamp;
        size_t size;

        ClipboardItem() : type(ClipboardDataType::Unknown), timestamp(0), size(0) {}
    };

    class ClipboardManager
    {
    public:
        ~ClipboardManager() = default;

        static ClipboardManager& Instance();

        bool Initialize(HWND hWnd);
        void Uninitialize();

        bool StartMonitoring();
        bool StopMonitoring();
        bool IsMonitoring() const { return m_monitoring; }

        std::vector<ClipboardItem> GetHistory(size_t limit = 20);
        bool GetLatestItem(ClipboardItem& item);
        
        bool CopyToClipboard(const std::wstring& text);
        bool PasteFromClipboard(std::wstring& text);

        bool CopyImageToClipboard(const std::vector<uint8_t>& imageData);
        bool PasteImageFromClipboard(std::vector<uint8_t>& imageData);

        bool CopyHtmlToClipboard(const std::wstring& html);
        bool PasteHtmlFromClipboard(std::wstring& html);

        bool CopyRtfToClipboard(const std::wstring& rtf);
        bool PasteRtfFromClipboard(std::wstring& rtf);

        void ClearHistory();
        size_t GetHistoryCount() const;

        bool IsFormatAvailable(ClipboardDataType type) const;

        using ClipboardChangedCallback = std::function<void(const ClipboardItem&)>;
        void SetClipboardChangedCallback(ClipboardChangedCallback callback);

    private:
        ClipboardManager();
        ClipboardManager(const ClipboardManager&) = delete;
        ClipboardManager& operator=(const ClipboardManager&) = delete;

        static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        void OnClipboardChanged();
        bool ReadClipboardData(ClipboardItem& item);
        bool ReadTextFromClipboard(std::wstring& text);
        bool ReadFilesFromClipboard(std::vector<std::wstring>& files);
        bool ReadImageFromClipboard(std::vector<uint8_t>& imageData);
        bool ReadHtmlFromClipboard(std::wstring& html);
        bool ReadRtfFromClipboard(std::wstring& rtf);

        ClipboardDataType DetectClipboardFormat() const;

        HWND m_hWnd;
        bool m_monitoring;
        std::vector<ClipboardItem> m_history;
        ClipboardChangedCallback m_callback;
        const size_t m_maxHistorySize;
        
        std::map<ClipboardDataType, UINT> m_formatMap;
    };
}