#include "ime/DAGParser.h"
#include "ime/PinyinParser.h"
#include "utils/logger.h"
#include <algorithm>
#include <fstream>

namespace ShuTongWen
{
    DAGParser::DAGParser()
    {}

    DAGParser::~DAGParser()
    {}

    bool DAGParser::Parse(const std::wstring& input, std::vector<DAGPath>& paths, size_t max_paths)
    {
        m_input = input;
        m_graph.clear();
        
        BuildGraph(input);
        
        DAGPath current;
        paths.clear();
        FindPaths(0, current, paths);
        
        std::sort(paths.begin(), paths.end(), std::greater<DAGPath>());
        
        if (paths.size() > max_paths)
        {
            paths.resize(max_paths);
        }
        
        return !paths.empty();
    }

    std::vector<DAGPath> DAGParser::GetTopPaths(size_t count)
    {
        return {};
    }

    void DAGParser::AddPinyin(const std::wstring& pinyin, double score)
    {
        m_pinyinScores[pinyin] = score;
    }

    void DAGParser::LoadDictionary(const std::wstring& dictPath)
    {
        std::wifstream file(dictPath);
        if (file.is_open())
        {
            std::wstring line;
            while (std::getline(file, line))
            {
                size_t tabPos = line.find(L'\t');
                if (tabPos != std::wstring::npos)
                {
                    std::wstring pinyin = line.substr(0, tabPos);
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
                    
                    m_pinyinScores[pinyin] = score;
                }
            }
            file.close();
        }
    }

    void DAGParser::BuildGraph(const std::wstring& input)
    {
        int n = static_cast<int>(input.length());
        
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j <= n; ++j)
            {
                std::wstring substr = input.substr(i, j - i);
                std::wstring lowerSubstr = StringUtils::ToLower(substr);
                
                if (PinyinParser::IsValidPinyin(lowerSubstr))
                {
                    double score = m_pinyinScores.count(lowerSubstr) 
                        ? m_pinyinScores[lowerSubstr] 
                        : 1.0;
                    
                    m_graph[i].emplace_back(i, j, lowerSubstr, score);
                }
            }
        }
    }

    void DAGParser::FindPaths(int pos, DAGPath current, std::vector<DAGPath>& paths)
    {
        if (pos >= static_cast<int>(m_input.length()))
        {
            current.total_score = CalculatePathScore(current);
            paths.push_back(current);
            return;
        }
        
        auto it = m_graph.find(pos);
        if (it == m_graph.end())
            return;
        
        for (const DAGNode& node : it->second)
        {
            DAGPath newPath = current;
            newPath.nodes.push_back(node);
            FindPaths(node.end, newPath, paths);
        }
    }

    double DAGParser::CalculatePathScore(const DAGPath& path)
    {
        double score = 0.0;
        int totalLength = 0;
        
        for (const DAGNode& node : path.nodes)
        {
            score += node.score;
            totalLength += node.end - node.start;
        }
        
        double lengthBonus = static_cast<double>(totalLength) / static_cast<double>(m_input.length());
        return score * lengthBonus;
    }
}