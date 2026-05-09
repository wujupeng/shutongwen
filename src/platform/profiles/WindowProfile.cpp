#include "platform/profiles/WindowProfile.h"
#include "utils/logger.h"

namespace ShuTongWen
{
    namespace Platform
    {
        bool ChromeProfile::Matches(HWND hwnd) const
        {
            if (!hwnd)
                return false;

            wchar_t className[256];
            GetClassNameW(hwnd, className, sizeof(className) / sizeof(wchar_t));
            return MatchesClassName(className);
        }

        bool ChromeProfile::MatchesClassName(const std::wstring& className) const
        {
            return className == L"Chrome_WidgetWin_1" ||
                   className == L"Chrome_WidgetWin_0" ||
                   className.find(L"Google") != std::wstring::npos;
        }

        bool ChromeProfile::MatchesProcessName(const std::wstring& processName) const
        {
            return processName.find(L"chrome.exe") != std::wstring::npos;
        }

        CaretBehavior ChromeProfile::GetCaretBehavior() const
        {
            CaretBehavior behavior;
            behavior.useTsfCaret = false;
            behavior.useTextServices = true;
            behavior.needsOffsetAdjustment = true;
            behavior.offsetX = 2;
            behavior.offsetY = 2;
            behavior.requiresScreenToClient = true;
            behavior.usePhysicalCoordinates = true;
            return behavior;
        }

        CompositionBehavior ChromeProfile::GetCompositionBehavior() const
        {
            CompositionBehavior behavior;
            behavior.allowInlineComposition = true;
            behavior.useSurrogatePairs = true;
            behavior.requiresCommitOnSpace = false;
            behavior.disableReconversion = true;
            behavior.useUnicodeMode = true;
            return behavior;
        }

        CandidateBehavior ChromeProfile::GetCandidateBehavior() const
        {
            CandidateBehavior behavior;
            behavior.useCustomCandidateWindow = true;
            behavior.needsPositionAdjustment = true;
            behavior.preferredPosition = 1;
            behavior.disableAutoCommit = false;
            behavior.enableShadowWindow = true;
            return behavior;
        }

        void ChromeProfile::AdjustCaretPosition(POINT& position) const
        {
            position.x += 2;
            position.y += 2;
        }

        void ChromeProfile::AdjustCandidatePosition(POINT& position) const
        {
            position.y += 8;
        }

        bool ElectronProfile::Matches(HWND hwnd) const
        {
            if (!hwnd)
                return false;

            wchar_t className[256];
            GetClassNameW(hwnd, className, sizeof(className) / sizeof(wchar_t));
            return MatchesClassName(className);
        }

        bool ElectronProfile::MatchesClassName(const std::wstring& className) const
        {
            return className.find(L"Electron") != std::wstring::npos ||
                   className.find(L"CEF") != std::wstring::npos ||
                   className.find(L"Chromium") != std::wstring::npos;
        }

        bool ElectronProfile::MatchesProcessName(const std::wstring& processName) const
        {
            return processName.find(L"electron.exe") != std::wstring::npos ||
                   processName.find(L"Code.exe") != std::wstring::npos;
        }

        CaretBehavior ElectronProfile::GetCaretBehavior() const
        {
            CaretBehavior behavior;
            behavior.useTsfCaret = false;
            behavior.useTextServices = true;
            behavior.needsOffsetAdjustment = true;
            behavior.offsetX = 1;
            behavior.offsetY = 1;
            behavior.requiresScreenToClient = true;
            behavior.usePhysicalCoordinates = true;
            return behavior;
        }

        CompositionBehavior ElectronProfile::GetCompositionBehavior() const
        {
            CompositionBehavior behavior;
            behavior.allowInlineComposition = true;
            behavior.useSurrogatePairs = true;
            behavior.requiresCommitOnSpace = false;
            behavior.disableReconversion = true;
            behavior.useUnicodeMode = true;
            return behavior;
        }

        CandidateBehavior ElectronProfile::GetCandidateBehavior() const
        {
            CandidateBehavior behavior;
            behavior.useCustomCandidateWindow = true;
            behavior.needsPositionAdjustment = true;
            behavior.preferredPosition = 1;
            behavior.disableAutoCommit = false;
            behavior.enableShadowWindow = true;
            return behavior;
        }

        bool VSCodeProfile::Matches(HWND hwnd) const
        {
            if (!hwnd)
                return false;

            wchar_t className[256];
            GetClassNameW(hwnd, className, sizeof(className) / sizeof(wchar_t));
            return MatchesClassName(className);
        }

        bool VSCodeProfile::MatchesClassName(const std::wstring& className) const
        {
            return className == L"Chrome_WidgetWin_1" ||
                   className.find(L"Code") != std::wstring::npos;
        }

        bool VSCodeProfile::MatchesProcessName(const std::wstring& processName) const
        {
            return processName.find(L"Code.exe") != std::wstring::npos ||
                   processName.find(L"Code - Insiders.exe") != std::wstring::npos;
        }

        CaretBehavior VSCodeProfile::GetCaretBehavior() const
        {
            CaretBehavior behavior;
            behavior.useTsfCaret = false;
            behavior.useTextServices = true;
            behavior.needsOffsetAdjustment = true;
            behavior.offsetX = 0;
            behavior.offsetY = 4;
            behavior.requiresScreenToClient = true;
            behavior.usePhysicalCoordinates = true;
            return behavior;
        }

        CompositionBehavior VSCodeProfile::GetCompositionBehavior() const
        {
            CompositionBehavior behavior;
            behavior.allowInlineComposition = true;
            behavior.useSurrogatePairs = true;
            behavior.requiresCommitOnSpace = true;
            behavior.disableReconversion = true;
            behavior.useUnicodeMode = true;
            return behavior;
        }

        CandidateBehavior VSCodeProfile::GetCandidateBehavior() const
        {
            CandidateBehavior behavior;
            behavior.useCustomCandidateWindow = true;
            behavior.needsPositionAdjustment = true;
            behavior.preferredPosition = 0;
            behavior.disableAutoCommit = false;
            behavior.enableShadowWindow = false;
            return behavior;
        }

        bool OfficeProfile::Matches(HWND hwnd) const
        {
            if (!hwnd)
                return false;

            wchar_t className[256];
            GetClassNameW(hwnd, className, sizeof(className) / sizeof(wchar_t));
            return MatchesClassName(className);
        }

        bool OfficeProfile::MatchesClassName(const std::wstring& className) const
        {
            return className.find(L"OpusApp") != std::wstring::npos ||
                   className.find(L"EXCEL") != std::wstring::npos ||
                   className.find(L"WINWORD") != std::wstring::npos ||
                   className.find(L"POWERPNT") != std::wstring::npos;
        }

        bool OfficeProfile::MatchesProcessName(const std::wstring& processName) const
        {
            return processName.find(L"EXCEL.EXE") != std::wstring::npos ||
                   processName.find(L"WINWORD.EXE") != std::wstring::npos ||
                   processName.find(L"POWERPNT.EXE") != std::wstring::npos;
        }

        CaretBehavior OfficeProfile::GetCaretBehavior() const
        {
            CaretBehavior behavior;
            behavior.useTsfCaret = true;
            behavior.useTextServices = true;
            behavior.needsOffsetAdjustment = false;
            behavior.offsetX = 0;
            behavior.offsetY = 0;
            behavior.requiresScreenToClient = false;
            behavior.usePhysicalCoordinates = false;
            return behavior;
        }

        CompositionBehavior OfficeProfile::GetCompositionBehavior() const
        {
            CompositionBehavior behavior;
            behavior.allowInlineComposition = true;
            behavior.useSurrogatePairs = true;
            behavior.requiresCommitOnSpace = false;
            behavior.disableReconversion = false;
            behavior.useUnicodeMode = true;
            return behavior;
        }

        CandidateBehavior OfficeProfile::GetCandidateBehavior() const
        {
            CandidateBehavior behavior;
            behavior.useCustomCandidateWindow = false;
            behavior.needsPositionAdjustment = false;
            behavior.preferredPosition = 0;
            behavior.disableAutoCommit = false;
            behavior.enableShadowWindow = false;
            return behavior;
        }

        bool GameProfile::Matches(HWND hwnd) const
        {
            if (!hwnd)
                return false;

            wchar_t className[256];
            GetClassNameW(hwnd, className, sizeof(className) / sizeof(wchar_t));
            return MatchesClassName(className);
        }

        bool GameProfile::MatchesClassName(const std::wstring& className) const
        {
            return className.find(L"Unity") != std::wstring::npos ||
                   className.find(L"Unreal") != std::wstring::npos ||
                   className.find(L"Game") != std::wstring::npos;
        }

        bool GameProfile::MatchesProcessName(const std::wstring& processName) const
        {
            return processName.find(L".exe") != std::wstring::npos &&
                   (processName.find(L"Game") != std::wstring::npos ||
                    processName.find(L"Unity") != std::wstring::npos ||
                    processName.find(L"Unreal") != std::wstring::npos);
        }

        CaretBehavior GameProfile::GetCaretBehavior() const
        {
            CaretBehavior behavior;
            behavior.useTsfCaret = false;
            behavior.useTextServices = false;
            behavior.needsOffsetAdjustment = false;
            behavior.offsetX = 0;
            behavior.offsetY = 0;
            behavior.requiresScreenToClient = false;
            behavior.usePhysicalCoordinates = true;
            return behavior;
        }

        CompositionBehavior GameProfile::GetCompositionBehavior() const
        {
            CompositionBehavior behavior;
            behavior.allowInlineComposition = false;
            behavior.useSurrogatePairs = false;
            behavior.requiresCommitOnSpace = true;
            behavior.disableReconversion = true;
            behavior.useUnicodeMode = false;
            return behavior;
        }

        CandidateBehavior GameProfile::GetCandidateBehavior() const
        {
            CandidateBehavior behavior;
            behavior.useCustomCandidateWindow = true;
            behavior.needsPositionAdjustment = false;
            behavior.preferredPosition = 2;
            behavior.disableAutoCommit = true;
            behavior.enableShadowWindow = true;
            return behavior;
        }

        WindowProfileManager::WindowProfileManager()
        {}

        WindowProfileManager& WindowProfileManager::Instance()
        {
            static WindowProfileManager instance;
            return instance;
        }

        bool WindowProfileManager::Initialize()
        {
            Logger::Info("Initializing WindowProfileManager...");

            m_profiles.clear();
            m_profiles.push_back(std::make_shared<ChromeProfile>());
            m_profiles.push_back(std::make_shared<ElectronProfile>());
            m_profiles.push_back(std::make_shared<VSCodeProfile>());
            m_profiles.push_back(std::make_shared<OfficeProfile>());
            m_profiles.push_back(std::make_shared<GameProfile>());

            Logger::Info("Loaded {} window profiles", m_profiles.size());
            return true;
        }

        void WindowProfileManager::Uninitialize()
        {
            m_profiles.clear();
        }

        std::shared_ptr<IWindowProfile> WindowProfileManager::GetProfile(HWND hwnd)
        {
            for (const auto& profile : m_profiles)
            {
                if (profile->Matches(hwnd))
                {
                    Logger::Debug("Found profile {} for window", profile->GetName());
                    return profile;
                }
            }
            return m_defaultProfile;
        }

        std::shared_ptr<IWindowProfile> WindowProfileManager::GetProfileByClassName(const std::wstring& className)
        {
            for (const auto& profile : m_profiles)
            {
                if (profile->MatchesClassName(className))
                {
                    return profile;
                }
            }
            return m_defaultProfile;
        }

        std::shared_ptr<IWindowProfile> WindowProfileManager::GetProfileByProcessName(const std::wstring& processName)
        {
            for (const auto& profile : m_profiles)
            {
                if (profile->MatchesProcessName(processName))
                {
                    return profile;
                }
            }
            return m_defaultProfile;
        }

        void WindowProfileManager::AddProfile(std::shared_ptr<IWindowProfile> profile)
        {
            m_profiles.push_back(profile);
        }

        void WindowProfileManager::RemoveProfile(const std::wstring& name)
        {
            m_profiles.erase(
                std::remove_if(m_profiles.begin(), m_profiles.end(),
                    [&name](const std::shared_ptr<IWindowProfile>& p) {
                        return p->GetName() == name;
                    }),
                m_profiles.end());
        }

        std::vector<std::shared_ptr<IWindowProfile>> WindowProfileManager::GetAllProfiles() const
        {
            return m_profiles;
        }
    }
}