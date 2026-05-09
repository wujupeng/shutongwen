#pragma once

#include <windows.h>
#include <string>
#include <memory>

namespace ShuTongWen
{
    namespace Platform
    {
        struct CaretBehavior
        {
            bool useTsfCaret;
            bool useTextServices;
            bool needsOffsetAdjustment;
            int offsetX;
            int offsetY;
            bool requiresScreenToClient;
            bool usePhysicalCoordinates;

            CaretBehavior() 
                : useTsfCaret(true), useTextServices(true), needsOffsetAdjustment(false),
                  offsetX(0), offsetY(0), requiresScreenToClient(false), usePhysicalCoordinates(false) {}
        };

        struct CompositionBehavior
        {
            bool allowInlineComposition;
            bool useSurrogatePairs;
            bool requiresCommitOnSpace;
            bool disableReconversion;
            bool useUnicodeMode;

            CompositionBehavior() 
                : allowInlineComposition(true), useSurrogatePairs(false), 
                  requiresCommitOnSpace(false), disableReconversion(false), useUnicodeMode(true) {}
        };

        struct CandidateBehavior
        {
            bool useCustomCandidateWindow;
            bool needsPositionAdjustment;
            int preferredPosition;
            bool disableAutoCommit;
            bool enableShadowWindow;

            CandidateBehavior() 
                : useCustomCandidateWindow(false), needsPositionAdjustment(false), 
                  preferredPosition(0), disableAutoCommit(false), enableShadowWindow(false) {}
        };

        class IWindowProfile
        {
        public:
            virtual ~IWindowProfile() = default;

            virtual std::wstring GetName() const = 0;
            virtual bool Matches(HWND hwnd) const = 0;
            virtual bool MatchesClassName(const std::wstring& className) const = 0;
            virtual bool MatchesProcessName(const std::wstring& processName) const = 0;

            virtual CaretBehavior GetCaretBehavior() const = 0;
            virtual CompositionBehavior GetCompositionBehavior() const = 0;
            virtual CandidateBehavior GetCandidateBehavior() const = 0;

            virtual void AdjustCaretPosition(POINT& position) const {}
            virtual void AdjustCandidatePosition(POINT& position) const {}
        };

        class ChromeProfile : public IWindowProfile
        {
        public:
            std::wstring GetName() const override { return L"Chrome"; }
            bool Matches(HWND hwnd) const override;
            bool MatchesClassName(const std::wstring& className) const override;
            bool MatchesProcessName(const std::wstring& processName) const override;

            CaretBehavior GetCaretBehavior() const override;
            CompositionBehavior GetCompositionBehavior() const override;
            CandidateBehavior GetCandidateBehavior() const override;

            void AdjustCaretPosition(POINT& position) const override;
            void AdjustCandidatePosition(POINT& position) const override;
        };

        class ElectronProfile : public IWindowProfile
        {
        public:
            std::wstring GetName() const override { return L"Electron"; }
            bool Matches(HWND hwnd) const override;
            bool MatchesClassName(const std::wstring& className) const override;
            bool MatchesProcessName(const std::wstring& processName) const override;

            CaretBehavior GetCaretBehavior() const override;
            CompositionBehavior GetCompositionBehavior() const override;
            CandidateBehavior GetCandidateBehavior() const override;
        };

        class VSCodeProfile : public IWindowProfile
        {
        public:
            std::wstring GetName() const override { return L"VSCode"; }
            bool Matches(HWND hwnd) const override;
            bool MatchesClassName(const std::wstring& className) const override;
            bool MatchesProcessName(const std::wstring& processName) const override;

            CaretBehavior GetCaretBehavior() const override;
            CompositionBehavior GetCompositionBehavior() const override;
            CandidateBehavior GetCandidateBehavior() const override;
        };

        class OfficeProfile : public IWindowProfile
        {
        public:
            std::wstring GetName() const override { return L"Office"; }
            bool Matches(HWND hwnd) const override;
            bool MatchesClassName(const std::wstring& className) const override;
            bool MatchesProcessName(const std::wstring& processName) const override;

            CaretBehavior GetCaretBehavior() const override;
            CompositionBehavior GetCompositionBehavior() const override;
            CandidateBehavior GetCandidateBehavior() const override;
        };

        class GameProfile : public IWindowProfile
        {
        public:
            std::wstring GetName() const override { return L"Game"; }
            bool Matches(HWND hwnd) const override;
            bool MatchesClassName(const std::wstring& className) const override;
            bool MatchesProcessName(const std::wstring& processName) const override;

            CaretBehavior GetCaretBehavior() const override;
            CompositionBehavior GetCompositionBehavior() const override;
            CandidateBehavior GetCandidateBehavior() const override;
        };

        class WindowProfileManager
        {
        public:
            ~WindowProfileManager() = default;

            static WindowProfileManager& Instance();

            bool Initialize();
            void Uninitialize();

            std::shared_ptr<IWindowProfile> GetProfile(HWND hwnd);
            std::shared_ptr<IWindowProfile> GetProfileByClassName(const std::wstring& className);
            std::shared_ptr<IWindowProfile> GetProfileByProcessName(const std::wstring& processName);

            void AddProfile(std::shared_ptr<IWindowProfile> profile);
            void RemoveProfile(const std::wstring& name);

            std::vector<std::shared_ptr<IWindowProfile>> GetAllProfiles() const;

        private:
            WindowProfileManager();

            std::vector<std::shared_ptr<IWindowProfile>> m_profiles;
            std::shared_ptr<IWindowProfile> m_defaultProfile;
        };
    }
}