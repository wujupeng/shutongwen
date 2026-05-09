#include "ime/language/SegmentGraph.h"
#include "ime/PinyinParser.h"
#include "utils/logger.h"
#include <algorithm>
#include <fstream>

namespace ShuTongWen
{
    namespace Language
    {
        SegmentGraph::SegmentGraph()
        {}

        SegmentGraph::~SegmentGraph()
        {}

        void SegmentGraph::BuildGraph(const std::wstring& input)
        {
            m_input = input;
            m_graph.clear();

            size_t n = input.length();

            for (size_t i = 0; i < n; ++i)
            {
                for (size_t j = i + 1; j <= n; ++j)
                {
                    std::wstring substr = input.substr(i, j - i);
                    std::wstring lowerSubstr = StringUtils::ToLower(substr);

                    double score = 0.0;
                    std::wstring type = L"unknown";

                    if (IsAllEnglish(lowerSubstr))
                    {
                        type = L"english";
                        score = CalculateEnglishScore(lowerSubstr);
                    }
                    else if (IsAllDigit(substr))
                    {
                        type = L"number";
                        score = 0.9;
                    }
                    else if (PinyinParser::IsValidPinyin(lowerSubstr))
                    {
                        type = L"pinyin";
                        score = CalculatePinyinScore(lowerSubstr);
                    }

                    if (score > 0)
                    {
                        m_graph[i].emplace_back(substr, type, score, i, j);
                    }
                }
            }
        }

        bool SegmentGraph::IsAllEnglish(const std::wstring& str)
        {
            for (wchar_t ch : str)
            {
                if (!((ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z')))
                    return false;
            }
            return !str.empty();
        }

        bool SegmentGraph::IsAllDigit(const std::wstring& str)
        {
            for (wchar_t ch : str)
            {
                if (!(ch >= L'0' && ch <= L'9'))
                    return false;
            }
            return !str.empty();
        }

        double SegmentGraph::CalculateEnglishScore(const std::wstring& word)
        {
            if (word.length() == 1)
                return 0.3;

            if (m_dictionary.count(word))
                return 0.95 + m_dictionary[word] * 0.05;

            double lengthBonus = std::min(0.3, static_cast<double>(word.length()) * 0.05);
            return 0.7 + lengthBonus;
        }

        double SegmentGraph::CalculatePinyinScore(const std::wstring& pinyin)
        {
            double baseScore = 0.8;
            
            if (m_dictionary.count(pinyin))
                baseScore += m_dictionary[pinyin] * 0.1;

            return baseScore;
        }

        std::vector<SegmentPath> SegmentGraph::FindBestPaths(size_t maxPaths)
        {
            std::vector<SegmentPath> paths;
            
            SegmentPath current;
            FindPaths(0, current, paths);

            std::sort(paths.begin(), paths.end(), std::greater<SegmentPath>());

            if (paths.size() > maxPaths)
                paths.resize(maxPaths);

            return paths;
        }

        void SegmentGraph::FindPaths(size_t pos, SegmentPath current, std::vector<SegmentPath>& paths)
        {
            if (pos >= m_input.length())
            {
                current.total_score = CalculatePathScore(current);
                paths.push_back(current);
                return;
            }

            auto it = m_graph.find(pos);
            if (it == m_graph.end())
                return;

            for (const Segment& segment : it->second)
            {
                SegmentPath newPath = current;
                newPath.segments.push_back(segment);
                FindPaths(segment.end, newPath, paths);
            }
        }

        double SegmentGraph::CalculatePathScore(const SegmentPath& path)
        {
            double score = 0.0;
            size_t totalLength = 0;

            for (size_t i = 0; i < path.segments.size(); ++i)
            {
                const Segment& seg = path.segments[i];
                score += seg.score;
                totalLength += seg.end - seg.start;

                if (i > 0)
                {
                    const Segment& prev = path.segments[i - 1];
                    score += CalculateTransitionScore(prev.type, seg.type);
                }
            }

            double lengthBonus = static_cast<double>(totalLength) / static_cast<double>(m_input.length());
            return score * lengthBonus;
        }

        double SegmentGraph::CalculateTransitionScore(const std::wstring& from, const std::wstring& to)
        {
            if (from == to)
                return 0.1;
            
            if ((from == L"english" && to == L"number") ||
                (from == L"number" && to == L"english"))
                return 0.05;

            if ((from == L"pinyin" && to == L"english") ||
                (from == L"english" && to == L"pinyin"))
                return -0.05;

            return 0.0;
        }

        void SegmentGraph::AddSegmentRule(const std::wstring& pattern, const std::wstring& type, double score)
        {
            m_dictionary[pattern] = score;
        }

        void SegmentGraph::LoadDictionary(const std::wstring& dictPath)
        {
            std::wifstream file(dictPath);
            if (!file.is_open())
                return;

            std::wstring line;
            while (std::getline(file, line))
            {
                size_t tabPos = line.find(L'\t');
                if (tabPos != std::wstring::npos)
                {
                    std::wstring word = line.substr(0, tabPos);
                    double score = 1.0;
                    
                    size_t scorePos = line.find(L'\t', tabPos + 1);
                    if (scorePos != std::wstring::npos)
                    {
                        try
                        {
                            score = std::stod(StringUtils::UTF16ToUTF8(line.substr(scorePos + 1)));
                        }
                        catch (...)
                        {
                            score = 1.0;
                        }
                    }
                    
                    m_dictionary[word] = score;
                }
            }

            file.close();
        }
    }
}