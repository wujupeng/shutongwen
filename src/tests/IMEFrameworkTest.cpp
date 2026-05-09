#include <gtest/gtest.h>
#include "ime/IMEFramework.h"
#include "ime/PinyinParser.h"
#include "utils/string_utils.h"

namespace ShuTongWen
{
    TEST(IMEFrameworkTest, CreateInstance)
    {
        auto framework = IMEFramework::Create();
        EXPECT_TRUE(framework != nullptr);
    }

    TEST(IMEFrameworkTest, Initialize)
    {
        auto framework = IMEFramework::Create();
        ASSERT_TRUE(framework != nullptr);

        HRESULT hr = framework->Initialize(nullptr);
        EXPECT_EQ(S_OK, hr);

        hr = framework->Uninitialize();
        EXPECT_EQ(S_OK, hr);
    }

    TEST(IMEFrameworkTest, StatusManagement)
    {
        auto framework = IMEFramework::Create();
        ASSERT_TRUE(framework != nullptr);

        framework->Initialize(nullptr);

        EXPECT_EQ(IMEStatus::Enabled, framework->GetStatus());

        framework->SetStatus(IMEStatus::Composing);
        EXPECT_EQ(IMEStatus::Composing, framework->GetStatus());

        framework->SetStatus(IMEStatus::Disabled);
        EXPECT_EQ(IMEStatus::Disabled, framework->GetStatus());

        framework->Uninitialize();
    }

    TEST(PinyinParserTest, ParseSinglePinyin)
    {
        std::vector<PinyinUnit> result;
        bool success = PinyinParser::Parse(L"nihao", result);

        EXPECT_TRUE(success);
        EXPECT_FALSE(result.empty());
    }

    TEST(PinyinParserTest, IsValidPinyin)
    {
        EXPECT_TRUE(PinyinParser::IsValidPinyin(L"ni"));
        EXPECT_TRUE(PinyinParser::IsValidPinyin(L"hao"));
        EXPECT_FALSE(PinyinParser::IsValidPinyin(L"xyz"));
    }

    TEST(StringUtilsTest, UTF8UTF16Conversion)
    {
        std::string utf8 = "Hello World";
        std::wstring utf16 = StringUtils::UTF8ToUTF16(utf8);
        std::string convertedBack = StringUtils::UTF16ToUTF8(utf16);

        EXPECT_EQ(utf8, convertedBack);
    }

    TEST(StringUtilsTest, CaseConversion)
    {
        std::wstring original = L"Hello World";
        std::wstring lower = StringUtils::ToLower(original);
        std::wstring upper = StringUtils::ToUpper(original);

        EXPECT_EQ(L"hello world", lower);
        EXPECT_EQ(L"HELLO WORLD", upper);
    }

    TEST(StringUtilsTest, Trim)
    {
        std::wstring str = L"  Hello World  ";
        std::wstring trimmed = StringUtils::Trim(str);

        EXPECT_EQ(L"Hello World", trimmed);
    }

    TEST(StringUtilsTest, SplitAndJoin)
    {
        std::wstring str = L"a,b,c,d";
        std::vector<std::wstring> parts = StringUtils::Split(str, L',');
        std::wstring joined = StringUtils::Join(parts, L',');

        EXPECT_EQ(4, parts.size());
        EXPECT_EQ(str, joined);
    }
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}