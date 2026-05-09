#include "ime/EmojiAssociator.h"
#include "utils/logger.h"
#include <algorithm>

namespace ShuTongWen
{
    EmojiAssociator::EmojiAssociator()
        : m_initialized(false)
    {}

    EmojiAssociator& EmojiAssociator::Instance()
    {
        static EmojiAssociator instance;
        return instance;
    }

    bool EmojiAssociator::Initialize()
    {
        if (m_initialized)
            return true;

        Logger::Info("Initializing EmojiAssociator...");
        LoadDefaultEmojis();
        m_initialized = true;
        Logger::Info("EmojiAssociator initialized with {} emojis", m_emojiMap.size());
        return true;
    }

    void EmojiAssociator::Uninitialize()
    {
        m_keywordMap.clear();
        m_emojiMap.clear();
        m_initialized = false;
    }

    std::vector<EmojiItem> EmojiAssociator::Query(const std::wstring& input, size_t limit)
    {
        std::vector<EmojiItem> results;
        
        if (!m_initialized || input.empty())
            return results;

        std::wstring lowerInput = StringUtils::ToLower(input);

        auto it = m_keywordMap.find(lowerInput);
        if (it != m_keywordMap.end())
        {
            results = it->second;
        }
        else
        {
            for (const auto& pair : m_keywordMap)
            {
                for (const EmojiItem& item : pair.second)
                {
                    for (const std::wstring& keyword : item.keywords)
                    {
                        if (StringUtils::StartsWith(keyword, lowerInput))
                        {
                            results.push_back(item);
                            break;
                        }
                    }
                }
            }
        }

        std::sort(results.begin(), results.end(),
            [](const EmojiItem& a, const EmojiItem& b) {
                return a.frequency > b.frequency;
            });

        if (results.size() > limit)
        {
            results.resize(limit);
        }

        return results;
    }

    bool EmojiAssociator::HasEmoji(const std::wstring& input)
    {
        return m_keywordMap.find(StringUtils::ToLower(input)) != m_keywordMap.end();
    }

    void EmojiAssociator::AddEmoji(const EmojiItem& item)
    {
        m_emojiMap[item.emoji] = item;
        
        for (const std::wstring& keyword : item.keywords)
        {
            std::wstring lowerKeyword = StringUtils::ToLower(keyword);
            m_keywordMap[lowerKeyword].push_back(item);
        }
    }

    void EmojiAssociator::RemoveEmoji(const std::wstring& emoji)
    {
        auto it = m_emojiMap.find(emoji);
        if (it != m_emojiMap.end())
        {
            for (const std::wstring& keyword : it->second.keywords)
            {
                std::wstring lowerKeyword = StringUtils::ToLower(keyword);
                auto& list = m_keywordMap[lowerKeyword];
                auto removeIt = std::remove_if(list.begin(), list.end(),
                    [&emoji](const EmojiItem& item) {
                        return item.emoji == emoji;
                    });
                list.erase(removeIt, list.end());
            }
            m_emojiMap.erase(it);
        }
    }

    void EmojiAssociator::UpdateFrequency(const std::wstring& emoji, int delta)
    {
        auto it = m_emojiMap.find(emoji);
        if (it != m_emojiMap.end())
        {
            it->second.frequency += delta;
        }
    }

    size_t EmojiAssociator::GetEmojiCount() const
    {
        return m_emojiMap.size();
    }

    void EmojiAssociator::LoadDefaultEmojis()
    {
        AddEmoji(EmojiItem(L"😊", L"微笑", { L"weixiao", L"smile", L"kaixin", L"haoyun" }, 100));
        AddEmoji(EmojiItem(L"😂", L"笑哭", { L"xiaoku", L"laugh", L"haoxiao" }, 95));
        AddEmoji(EmojiItem(L"🤣", L"笑翻", { L"xiaofan", L"haha", L"kuxiao" }, 90));
        AddEmoji(EmojiItem(L"😍", L"花痴", { L"hua chi", L"ai", L"re", L"xingfen" }, 85));
        AddEmoji(EmojiItem(L"🥰", L"喜欢", { L"xihuan", L"love", L"aini" }, 80));
        AddEmoji(EmojiItem(L"😭", L"大哭", { L"daku", L"sad", L"beishang" }, 75));
        AddEmoji(EmojiItem(L"😢", L"流泪", { L"liulei", L"cry", L"shangxin" }, 70));
        AddEmoji(EmojiItem(L"😤", L"生气", { L"shengqi", L"angry", L"fennu" }, 65));
        AddEmoji(EmojiItem(L"😱", L"惊讶", { L"jingya", L"surprise", L"chijing" }, 60));
        AddEmoji(EmojiItem(L"😴", L"睡觉", { L"shuijiao", L"sleep", L"lan" }, 55));
        AddEmoji(EmojiItem(L"🤔", L"思考", { L"sikao", L"think", L"naojin" }, 50));
        AddEmoji(EmojiItem(L"🤗", L"拥抱", { L"yongbao", L"hug", L"wen" }, 45));
        AddEmoji(EmojiItem(L"👍", L"点赞", { L"dianzan", L"good", L"zan" }, 120));
        AddEmoji(EmojiItem(L"👎", L"踩", { L"cai", L"bad", L"buhao" }, 35));
        AddEmoji(EmojiItem(L"👏", L"鼓掌", { L"guzhang", L"clap", L"zan" }, 88));
        AddEmoji(EmojiItem(L"🎉", L"庆祝", { L"qingzhu", L"celebrate", L"huodong" }, 78));
        AddEmoji(EmojiItem(L"🎁", L"礼物", { L"liwu", L"gift", L"shengri" }, 72));
        AddEmoji(EmojiItem(L"❤️", L"爱心", { L"aixin", L"heart", L"hongxin" }, 110));
        AddEmoji(EmojiItem(L"💔", L"心碎", { L"xinsui", L"broken", L"shangxin" }, 68));
        AddEmoji(EmojiItem(L"🔥", L"火焰", { L"huoyan", L"fire", L"re" }, 92));
        AddEmoji(EmojiItem(L"⭐", L"星星", { L"xingxing", L"star", L"liang" }, 82));
        AddEmoji(EmojiItem(L"🌟", L"闪亮", { L"shanliang", L"shine", L"ming" }, 77));
        AddEmoji(EmojiItem(L"✨", L"闪烁", { L"shanshuo", L"sparkle", L"guang" }, 73));
        AddEmoji(EmojiItem(L"💪", L"加油", { L"jiayou", L"fighting", L"ganba" }, 115));
        AddEmoji(EmojiItem(L"🎯", L"目标", { L"mubiao", L"target", L"zhong" }, 58));
        AddEmoji(EmojiItem(L"💯", L"满分", { L"manfen", L"perfect", L"100" }, 86));
        AddEmoji(EmojiItem(L"💢", L"怒火", { L"nu huo", L"angry", L"fen" }, 48));
        AddEmoji(EmojiItem(L"💤", L"睡觉", { L"shuijiao", L"sleep", L"mian" }, 53));
        AddEmoji(EmojiItem(L"💡", L"灯泡", { L"dengpao", L"idea", L"ming" }, 63));
        AddEmoji(EmojiItem(L"💬", L"说话", { L"shuohua", L"chat", L"yu" }, 56));
        AddEmoji(EmojiItem(L"🐶", L"狗", { L"gou", L"dog", L"wangwang" }, 42));
        AddEmoji(EmojiItem(L"🐱", L"猫", { L"mao", L"cat", L"mimi" }, 46));
        AddEmoji(EmojiItem(L"🐼", L"熊猫", { L"xiongmao", L"panda", L"kawaii" }, 98));
        AddEmoji(EmojiItem(L"🐸", L"青蛙", { L"qingwa", L"frog", L"wa" }, 38));
        AddEmoji(EmojiItem(L"🦊", L"狐狸", { L"huli", L"fox", L"lingli" }, 44));
        AddEmoji(EmojiItem(L"🦁", L"狮子", { L"shizi", L"lion", L"wang" }, 51));
        AddEmoji(EmojiItem(L"🐯", L"老虎", { L"laohu", L"tiger", L"huang" }, 54));
        AddEmoji(EmojiItem(L"🐰", L"兔子", { L"tuzi", L"rabbit", L"cute" }, 84));
        AddEmoji(EmojiItem(L"🐻", L"熊", { L"xiong", L"bear", L"meng" }, 61));
        AddEmoji(EmojiItem(L"🐨", L"考拉", { L"kaola", L"koala", L"aodaliya" }, 71));
        AddEmoji(EmojiItem(L"🍎", L"苹果", { L"pingguo", L"apple", L"guo" }, 89));
        AddEmoji(EmojiItem(L"🍊", L"橙子", { L"chengzi", L"orange", L"ju" }, 76));
        AddEmoji(EmojiItem(L"🍋", L"柠檬", { L"ningmeng", L"lemon", L"suan" }, 67));
        AddEmoji(EmojiItem(L"🍇", L"葡萄", { L"putao", L"grape", L"zi" }, 62));
        AddEmoji(EmojiItem(L"🍓", L"草莓", { L"caomei", L"strawberry", L"mei" }, 91));
        AddEmoji(EmojiItem(L"🍑", L"桃子", { L"taozi", L"peach", L"shui" }, 74));
        AddEmoji(EmojiItem(L"🍒", L"樱桃", { L"yingtao", L"cherry", L"hong" }, 66));
        AddEmoji(EmojiItem(L"🌸", L"樱花", { L"yinghua", L"sakura", L"hua" }, 81));
        AddEmoji(EmojiItem(L"🌺", L"花", { L"hua", L"flower", L"mei" }, 59));
        AddEmoji(EmojiItem(L"🌹", L"玫瑰", { L"meigui", L"rose", L"hong" }, 87));
        AddEmoji(EmojiItem(L"🌻", L"向日葵", { L"xiangrikui", L"sunflower", L"ri" }, 57));
        AddEmoji(EmojiItem(L"🌙", L"月亮", { L"yueliang", L"moon", L"night" }, 79));
        AddEmoji(EmojiItem(L"☀️", L"太阳", { L"taiyang", L"sun", L"ri" }, 83));
        AddEmoji(EmojiItem(L"🌈", L"彩虹", { L"caihong", L"rainbow", L"hong" }, 93));
        AddEmoji(EmojiItem(L"❄️", L"雪花", { L"xuehua", L"snow", L"bai" }, 69));
        AddEmoji(EmojiItem(L"🔥", L"火", { L"huo", L"fire", L"re" }, 94));
        AddEmoji(EmojiItem(L"💧", L"水滴", { L"shuidi", L"water", L"qing" }, 47));
        AddEmoji(EmojiItem(L"🎵", L"音符", { L"yinfu", L"music", L"ge" }, 52));
        AddEmoji(EmojiItem(L"🎶", L"旋律", { L"xuanlv", L"melody", L"qu" }, 43));
        AddEmoji(EmojiItem(L"🎸", L"吉他", { L"jita", L"guitar", L"yueqi" }, 49));
        AddEmoji(EmojiItem(L"🎹", L"钢琴", { L"gangqin", L"piano", L"yueqi" }, 41));
        AddEmoji(EmojiItem(L"🎤", L"麦克风", { L"maikefeng", L"mic", L"ge" }, 55));
        AddEmoji(EmojiItem(L"🏀", L"篮球", { L"lanqiu", L"basketball", L"qiu" }, 64));
        AddEmoji(EmojiItem(L"⚽", L"足球", { L"zuqiu", L"football", L"qiu" }, 70));
        AddEmoji(EmojiItem(L"🏈", L"橄榄球", { L"ganlanqiu", L"football", L"qiu" }, 36));
        AddEmoji(EmojiItem(L"⚾", L"棒球", { L"bangqiu", L"baseball", L"qiu" }, 39));
        AddEmoji(EmojiItem(L"🎾", L"网球", { L"wangqiu", L"tennis", L"qiu" }, 40));
        AddEmoji(EmojiItem(L"🏆", L"奖杯", { L"jiangbei", L"trophy", L"ying" }, 80));
        AddEmoji(EmojiItem(L"🎁", L"礼物", { L"liwu", L"gift", L"shengri" }, 78));
        AddEmoji(EmojiItem(L"🎂", L"蛋糕", { L"dangao", L"cake", L"shengri" }, 85));
        AddEmoji(EmojiItem(L"🍰", L"蛋糕", { L"dangao", L"cake", L"chi" }, 82));
        AddEmoji(EmojiItem(L"🍫", L"巧克力", { L"qiaokeli", L"chocolate", L"tian" }, 77));
        AddEmoji(EmojiItem(L"🍿", L"爆米花", { L"baomihua", L"popcorn", L"chi" }, 68));
        AddEmoji(EmojiItem(L"🍦", L"冰淇淋", { L"bingqilin", L"icecream", L"liang" }, 90));
        AddEmoji(EmojiItem(L"🍩", L"甜甜圈", { L"tiantianquan", L"donut", L"tian" }, 63));
        AddEmoji(EmojiItem(L"🍪", L"饼干", { L"binggan", L"cookie", L"chi" }, 58));
        AddEmoji(EmojiItem(L"☕", L"咖啡", { L"kafei", L"coffee", L"he" }, 96));
        AddEmoji(EmojiItem(L"🍵", L"茶", { L"cha", L"tea", L"he" }, 88));
        AddEmoji(EmojiItem(L"🍶", L"酒", { L"jiu", L"wine", L"he" }, 54));
        AddEmoji(EmojiItem(L"🚀", L"火箭", { L"huojian", L"rocket", L"fei" }, 97));
        AddEmoji(EmojiItem(L"✈️", L"飞机", { L"feiji", L"plane", L"chu" }, 86));
        AddEmoji(EmojiItem(L"🚗", L"汽车", { L"qiche", L"car", L"che" }, 84));
        AddEmoji(EmojiItem(L"🚲", L"自行车", { L"zixingche", L"bike", L"che" }, 75));
        AddEmoji(EmojiItem(L"🏠", L"房子", { L"fangzi", L"house", L"zhu" }, 69));
        AddEmoji(EmojiItem(L"🏢", L"建筑", { L"jianzhu", L"building", L"gong" }, 56));
        AddEmoji(EmojiItem(L"🏰", L"城堡", { L"chengbao", L"castle", L"huang" }, 61));
        AddEmoji(EmojiItem(L"⛵", L"船", { L"chuan", L"ship", L"shui" }, 53));
        AddEmoji(EmojiItem(L"🚢", L"轮船", { L"lunchuan", L"ship", L"da" }, 48));
        AddEmoji(EmojiItem(L"🚂", L"火车", { L"huoche", L"train", L"che" }, 66));
        AddEmoji(EmojiItem(L"🚁", L"直升机", { L"zhishengji", L"helicopter", L"fei" }, 51));
        AddEmoji(EmojiItem(L"🛸", L"飞碟", { L"feidie", L"ufo", L"waixing" }, 45));
        AddEmoji(EmojiItem(L"📱", L"手机", { L"shouji", L"phone", L"dian" }, 99));
        AddEmoji(EmojiItem(L"💻", L"电脑", { L"diannao", L"computer", L"ji" }, 95));
        AddEmoji(EmojiItem(L"📷", L"相机", { L"xiangji", L"camera", L"zhao" }, 72));
        AddEmoji(EmojiItem(L"📚", L"书", { L"shu", L"book", L"du" }, 81));
        AddEmoji(EmojiItem(L"✏️", L"铅笔", { L"qianbi", L"pencil", L"xie" }, 67));
        AddEmoji(EmojiItem(L"✒️", L"钢笔", { L"gangbi", L"pen", L"xie" }, 62));
        AddEmoji(EmojiItem(L"💼", L"公文包", { L"gongwenbao", L"bag", L"ban" }, 57));
        AddEmoji(EmojiItem(L"👔", L"领带", { L"lingdai", L"tie", L"zhuang" }, 44));
        AddEmoji(EmojiItem(L"👗", L"裙子", { L"qunzi", L"skirt", L"nv" }, 52));
        AddEmoji(EmojiItem(L"👠", L"高跟鞋", { L"gaogenxie", L"shoe", L"nv" }, 47));
        AddEmoji(EmojiItem(L"👑", L"皇冠", { L"huangguan", L"crown", L"wang" }, 76));
        AddEmoji(EmojiItem(L"💍", L"戒指", { L"jiezhi", L"ring", L"ai" }, 74));
        AddEmoji(EmojiItem(L"🎩", L"礼帽", { L"limao", L"hat", L"zhuang" }, 43));
        AddEmoji(EmojiItem(L"🌍", L"地球", { L"diqiu", L"earth", L"qiu" }, 68));
        AddEmoji(EmojiItem(L"🌎", L"世界", { L"shijie", L"world", L"quan" }, 65));
        AddEmoji(EmojiItem(L"🌏", L"亚洲", { L"yazhou", L"asia", L"zhou" }, 59));
        AddEmoji(EmojiItem(L"🗺️", L"地图", { L"ditu", L"map", L"lu" }, 55));
        AddEmoji(EmojiItem(L"🏔️", L"山", { L"shan", L"mountain", L"gao" }, 60));
        AddEmoji(EmojiItem(L"🌊", L"海浪", { L"hailang", L"wave", L"shui" }, 64));
        AddEmoji(EmojiItem(L"🏖️", L"海滩", { L"haitan", L"beach", L"sha" }, 58));
        AddEmoji(EmojiItem(L"🌴", L"椰子树", { L"yezishu", L"coconut", L"shu" }, 53));
        AddEmoji(EmojiItem(L"🎄", L"圣诞树", { L"shengdanshu", L"christmas", L"shu" }, 50));
        AddEmoji(EmojiItem(L"🎆", L"烟花", { L"yanhua", L"firework", L"hua" }, 73));
        AddEmoji(EmojiItem(L"🎇", L"烟火", { L"yanhuo", L"firework", L"kong" }, 71));
        AddEmoji(EmojiItem(L"✨", L"星光", { L"xingguang", L"sparkle", L"liang" }, 70));
        AddEmoji(EmojiItem(L"💫", L"旋转", { L"xuanzhuan", L"spin", L"huan" }, 54));
        AddEmoji(EmojiItem(L"🌟", L"星星", { L"xingxing", L"star", L"ming" }, 82));
        AddEmoji(EmojiItem(L"⚡", L"闪电", { L"shandian", L"lightning", L"dian" }, 63));
        AddEmoji(EmojiItem(L"🌈", L"彩虹", { L"caihong", L"rainbow", L"hong" }, 93));
        AddEmoji(EmojiItem(L"🌤️", L"晴天", { L"qingtian", L"sunny", L"tian" }, 78));
        AddEmoji(EmojiItem(L"⛅", L"多云", { L"duoyun", L"cloudy", L"tian" }, 67));
        AddEmoji(EmojiItem(L"🌧️", L"下雨", { L"xiayu", L"rain", L"tian" }, 62));
        AddEmoji(EmojiItem(L"⛈️", L"雷雨", { L"leiyu", L"storm", L"tian" }, 57));
        AddEmoji(EmojiItem(L"❄️", L"下雪", { L"xiaxue", L"snow", L"tian" }, 69));
        AddEmoji(EmojiItem(L"🌫️", L"雾", { L"wu", L"fog", L"tian" }, 52));
        AddEmoji(EmojiItem(L"🔥", L"燃烧", { L"ranshao", L"fire", L"re" }, 94));
        AddEmoji(EmojiItem(L"💨", L"风", { L"feng", L"wind", L"qi" }, 56));
        AddEmoji(EmojiItem(L"💦", L"汗水", { L"hanshui", L"sweat", L"re" }, 51));
        AddEmoji(EmojiItem(L"💧", L"泪水", { L"leishui", L"tear", L"ku" }, 48));
    }
}