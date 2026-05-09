#pragma once

#include <windows.h>
#include <functional>
#include <string>
#include <vector>
#include <memory>

namespace ShuTongWen
{
    enum class IMEStatus
    {
        Disabled,
        Enabled,
        Composing,
        Committed
    };

    struct CandidateItem
    {
        std::wstring text;
        std::wstring pinyin;
        int frequency;
        std::wstring comment;
    };

    struct CompositionString
    {
        std::wstring preedit;
        std::wstring committed;
        int cursor_pos;
    };

    class IMEFramework
    {
    public:
        virtual ~IMEFramework() = default;

        virtual HRESULT Initialize(HINSTANCE hInstance) = 0;
        virtual HRESULT Uninitialize() = 0;

        virtual HRESULT SetStatus(IMEStatus status) = 0;
        virtual IMEStatus GetStatus() const = 0;

        virtual HRESULT ProcessKeyDown(WPARAM wParam, LPARAM lParam, bool& handled) = 0;
        virtual HRESULT ProcessKeyUp(WPARAM wParam, LPARAM lParam, bool& handled) = 0;

        virtual HRESULT Compose(const std::wstring& input, CompositionString& result) = 0;
        virtual HRESULT GetCandidates(std::vector<CandidateItem>& candidates) = 0;
        virtual HRESULT SelectCandidate(int index) = 0;

        virtual HRESULT CommitString(const std::wstring& str) = 0;
        virtual HRESULT ClearComposition() = 0;

        using CandidateChangedCallback = std::function<void(const std::vector<CandidateItem>&)>;
        using CompositionChangedCallback = std::function<void(const CompositionString&)>;
        using StatusChangedCallback = std::function<void(IMEStatus)>;

        virtual void SetCandidateChangedCallback(CandidateChangedCallback callback) = 0;
        virtual void SetCompositionChangedCallback(CompositionChangedCallback callback) = 0;
        virtual void SetStatusChangedCallback(StatusChangedCallback callback) = 0;

        static std::unique_ptr<IMEFramework> Create();
    };
}