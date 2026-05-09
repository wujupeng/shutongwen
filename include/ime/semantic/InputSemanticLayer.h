#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <memory>

namespace ShuTongWen
{
    namespace Semantic
    {
        enum class CaretSource
        {
            TSF,
            PHYSICAL,
            TEXT_SERVICE,
            WINDOW_FALLBACK
        };

        enum class InputMode
        {
            UNKNOWN,
            CHINESE,
            ENGLISH,
            MIXED,
            CODE,
            EMAIL,
            URL
        };

        enum class WindowType
        {
            UNKNOWN,
            CHROME,
            ELECTRON,
            OFFICE,
            GAME,
            UWP,
            STANDARD
        };

        struct InputSemanticContext
        {
            CaretSource caretSource;
            POINT caretPos;
            POINT clientPos;
            POINT screenPos;
            RECT clientRect;
            RECT windowRect;
            float dpiScale;
            bool isSecureDesktop;
            bool isCompositionLocked;
            bool isHighDpi;
            bool isMultiMonitor;
            WindowType windowType;
            InputMode inputMode;
            HWND targetWindow;

            InputSemanticContext() 
                : caretSource(CaretSource::TSF), dpiScale(1.0f),
                  isSecureDesktop(false), isCompositionLocked(false),
                  isHighDpi(false), isMultiMonitor(false),
                  windowType(WindowType::UNKNOWN), inputMode(InputMode::UNKNOWN),
                  targetWindow(nullptr) {}
        };

        struct CompositionSemantic
        {
            std::wstring text;
            std::wstring pinyin;
            int cursorPosition;
            bool isComposing;
            bool needsCommit;
            bool isReconversion;

            CompositionSemantic() : cursorPosition(0), isComposing(false), 
                                   needsCommit(false), isReconversion(false) {}
        };

        struct CandidateSemantic
        {
            std::vector<CandidateItem> candidates;
            int selectedIndex;
            bool needsPositionAdjustment;
            POINT anchorPosition;
            bool useCustomWindow;

            CandidateSemantic() : selectedIndex(0), needsPositionAdjustment(false), 
                                 useCustomWindow(false) {}
        };

        class InputSemanticLayer
        {
        public:
            ~InputSemanticLayer() = default;

            static InputSemanticLayer& Instance();

            bool Initialize();
            void Uninitialize();

            InputSemanticContext ResolveContext(HWND hwnd);
            void UpdateContext(HWND hwnd, InputSemanticContext& context);

            void ResolveCaretPosition(const InputSemanticContext& context, POINT& outPosition);
            void ResolveCandidatePosition(const InputSemanticContext& context, const CandidateSemantic& candidate, POINT& outPosition);

            void UpdateComposition(HWND hwnd, CompositionSemantic& composition);
            void ApplyContextRules(InputSemanticContext& context);

            WindowType DetectWindowType(HWND hwnd) const;
            InputMode DetectInputMode(HWND hwnd) const;

        private:
            InputSemanticLayer();

            bool ResolveTSFCaret(HWND hwnd, POINT& position);
            bool ResolvePhysicalCaret(HWND hwnd, POINT& position);
            bool ResolveTextServiceCaret(HWND hwnd, POINT& position);
            bool ResolveWindowFallbackCaret(HWND hwnd, POINT& position);

            void ApplyChromeRules(InputSemanticContext& context);
            void ApplyElectronRules(InputSemanticContext& context);
            void ApplyOfficeRules(InputSemanticContext& context);
            void ApplyGameRules(InputSemanticContext& context);

            std::unordered_map<HWND, InputSemanticContext> m_contextCache;
        };
    }
}