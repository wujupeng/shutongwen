#include "ime/semantic/InputSemanticLayer.h"
#include "platform/profiles/WindowProfile.h"
#include "platform/caret/CaretLocator.h"
#include "platform/caret/DpiTransformer.h"
#include "utils/logger.h"

namespace ShuTongWen
{
    namespace Semantic
    {
        InputSemanticLayer::InputSemanticLayer()
        {}

        InputSemanticLayer& InputSemanticLayer::Instance()
        {
            static InputSemanticLayer instance;
            return instance;
        }

        bool InputSemanticLayer::Initialize()
        {
            Logger::Info("Initializing InputSemanticLayer...");
            return true;
        }

        void InputSemanticLayer::Uninitialize()
        {
            m_contextCache.clear();
        }

        InputSemanticContext InputSemanticLayer::ResolveContext(HWND hwnd)
        {
            InputSemanticContext context;
            UpdateContext(hwnd, context);
            ApplyContextRules(context);
            return context;
        }

        void InputSemanticLayer::UpdateContext(HWND hwnd, InputSemanticContext& context)
        {
            context.targetWindow = hwnd;
            context.windowType = DetectWindowType(hwnd);
            context.inputMode = DetectInputMode(hwnd);

            GetWindowRect(hwnd, &context.windowRect);
            GetClientRect(hwnd, &context.clientRect);

            UINT dpi = Platform::DpiTransformer::Instance().GetWindowDpi(hwnd);
            context.dpiScale = static_cast<float>(dpi) / 96.0f;
            context.isHighDpi = dpi > 96;

            context.isSecureDesktop = GetDesktopWindow() != nullptr;

            HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFOEX monitorInfo = {sizeof(MONITORINFOEX)};
            GetMonitorInfoW(hMonitor, &monitorInfo);
            context.isMultiMonitor = GetSystemMetrics(SM_CMONITORS) > 1;

            switch (context.windowType)
            {
            case WindowType::CHROME:
            case WindowType::ELECTRON:
                context.caretSource = CaretSource::PHYSICAL;
                break;
            case WindowType::OFFICE:
                context.caretSource = CaretSource::TSF;
                break;
            case WindowType::GAME:
                context.caretSource = CaretSource::WINDOW_FALLBACK;
                break;
            default:
                context.caretSource = CaretSource::TEXT_SERVICE;
                break;
            }

            ResolveCaretPosition(context, context.caretPos);

            Logger::Debug("Resolved context for window type: {}, caret source: {}", 
                static_cast<int>(context.windowType), static_cast<int>(context.caretSource));
        }

        void InputSemanticLayer::ResolveCaretPosition(const InputSemanticContext& context, POINT& outPosition)
        {
            switch (context.caretSource)
            {
            case CaretSource::TSF:
                if (!ResolveTSFCaret(context.targetWindow, outPosition))
                    ResolvePhysicalCaret(context.targetWindow, outPosition);
                break;
            case CaretSource::PHYSICAL:
                if (!ResolvePhysicalCaret(context.targetWindow, outPosition))
                    ResolveWindowFallbackCaret(context.targetWindow, outPosition);
                break;
            case CaretSource::TEXT_SERVICE:
                if (!ResolveTextServiceCaret(context.targetWindow, outPosition))
                    ResolvePhysicalCaret(context.targetWindow, outPosition);
                break;
            case CaretSource::WINDOW_FALLBACK:
                ResolveWindowFallbackCaret(context.targetWindow, outPosition);
                break;
            }

            outPosition.x = static_cast<int>(outPosition.x * context.dpiScale);
            outPosition.y = static_cast<int>(outPosition.y * context.dpiScale);
        }

        void InputSemanticLayer::ResolveCandidatePosition(const InputSemanticContext& context, const CandidateSemantic& candidate, POINT& outPosition)
        {
            outPosition = context.caretPos;

            if (candidate.needsPositionAdjustment)
            {
                switch (context.windowType)
                {
                case WindowType::CHROME:
                case WindowType::ELECTRON:
                    outPosition.y += static_cast<int>(8 * context.dpiScale);
                    outPosition.x += static_cast<int>(2 * context.dpiScale);
                    break;
                case WindowType::VSCODE:
                    outPosition.y += static_cast<int>(4 * context.dpiScale);
                    break;
                }
            }

            if (!context.isHighDpi)
            {
                outPosition.x = static_cast<int>(outPosition.x / context.dpiScale);
                outPosition.y = static_cast<int>(outPosition.y / context.dpiScale);
            }
        }

        void InputSemanticLayer::UpdateComposition(HWND hwnd, CompositionSemantic& composition)
        {
            InputSemanticContext context = ResolveContext(hwnd);
            
            switch (context.windowType)
            {
            case WindowType::CHROME:
            case WindowType::ELECTRON:
                composition.needsCommit = true;
                break;
            case WindowType::OFFICE:
                composition.needsCommit = false;
                break;
            case WindowType::GAME:
                composition.isComposing = false;
                composition.needsCommit = true;
                break;
            }
        }

        void InputSemanticLayer::ApplyContextRules(InputSemanticContext& context)
        {
            switch (context.windowType)
            {
            case WindowType::CHROME:
                ApplyChromeRules(context);
                break;
            case WindowType::ELECTRON:
                ApplyElectronRules(context);
                break;
            case WindowType::OFFICE:
                ApplyOfficeRules(context);
                break;
            case WindowType::GAME:
                ApplyGameRules(context);
                break;
            }
        }

        WindowType InputSemanticLayer::DetectWindowType(HWND hwnd) const
        {
            auto profile = Platform::WindowProfileManager::Instance().GetProfile(hwnd);
            
            if (!profile)
                return WindowType::STANDARD;

            std::wstring name = profile->GetName();
            if (name == L"Chrome")
                return WindowType::CHROME;
            if (name == L"Electron")
                return WindowType::ELECTRON;
            if (name == L"VSCode")
                return WindowType::ELECTRON;
            if (name == L"Office")
                return WindowType::OFFICE;
            if (name == L"Game")
                return WindowType::GAME;

            return WindowType::STANDARD;
        }

        InputMode InputSemanticLayer::DetectInputMode(HWND hwnd) const
        {
            auto profile = Platform::WindowProfileManager::Instance().GetProfile(hwnd);
            if (profile)
            {
                if (profile->GetName() == L"Game")
                    return InputMode::CODE;
                if (profile->GetName() == L"VSCode")
                    return InputMode::MIXED;
            }

            return InputMode::CHINESE;
        }

        bool InputSemanticLayer::ResolveTSFCaret(HWND hwnd, POINT& position)
        {
            POINT pt;
            if (GetCaretPos(&pt))
            {
                ClientToScreen(hwnd, &pt);
                position = pt;
                return true;
            }
            return false;
        }

        bool InputSemanticLayer::ResolvePhysicalCaret(HWND hwnd, POINT& position)
        {
            auto locator = Platform::CaretLocator::Instance();
            auto pos = locator.GetCaretPositionForWindow(hwnd);
            
            if (pos.isValid)
            {
                position = pos.screenPos;
                return true;
            }
            return false;
        }

        bool InputSemanticLayer::ResolveTextServiceCaret(HWND hwnd, POINT& position)
        {
            TTFINDRESULT findResult = {0};
            findResult.cbStruct = sizeof(TTFINDRESULT);

            if (TF_FindCurrent(hwnd, &findResult) == S_OK)
            {
                position = findResult.rcCaret.leftTop;
                return true;
            }
            return false;
        }

        bool InputSemanticLayer::ResolveWindowFallbackCaret(HWND hwnd, POINT& position)
        {
            RECT rect;
            if (GetClientRect(hwnd, &rect))
            {
                POINT pt = {rect.left + 10, rect.top + 10};
                ClientToScreen(hwnd, &pt);
                position = pt;
                return true;
            }
            return false;
        }

        void InputSemanticLayer::ApplyChromeRules(InputSemanticContext& context)
        {
            context.caretSource = CaretSource::PHYSICAL;
            context.isCompositionLocked = false;
            context.inputMode = InputMode::MIXED;
        }

        void InputSemanticLayer::ApplyElectronRules(InputSemanticContext& context)
        {
            context.caretSource = CaretSource::PHYSICAL;
            context.isCompositionLocked = false;
            context.inputMode = InputMode::MIXED;
        }

        void InputSemanticLayer::ApplyOfficeRules(InputSemanticContext& context)
        {
            context.caretSource = CaretSource::TSF;
            context.isCompositionLocked = false;
            context.inputMode = InputMode::CHINESE;
        }

        void InputSemanticLayer::ApplyGameRules(InputSemanticContext& context)
        {
            context.caretSource = CaretSource::WINDOW_FALLBACK;
            context.isCompositionLocked = true;
            context.inputMode = InputMode::ENGLISH;
        }
    }
}