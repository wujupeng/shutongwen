#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace ShuTongWen
{
    struct DAGNode
    {
        int start;
        int end;
        std::wstring pinyin;
        double score;
        
        DAGNode(int s, int e, const std::wstring& p, double sc = 0.0)
            : start(s), end(e), pinyin(p), score(sc) {}
    };

    struct DAGPath
    {
        std::vector<DAGNode> nodes;
        double total_score;
        
        DAGPath() : total_score(0.0) {}
        
        bool operator<(const DAGPath& other) const {
            return total_score < other.total_score;
        }
        
        bool operator>(const DAGPath& other) const {
            return total_score > other.total_score;
        }
    };

    class DAGParser
    {
    public:
        DAGParser();
        ~DAGParser();

        bool Parse(const std::wstring& input, std::vector<DAGPath>& paths, size_t max_paths = 10);
        std::vector<DAGPath> GetTopPaths(size_t count = 5);

        void AddPinyin(const std::wstring& pinyin, double score = 1.0);
        void LoadDictionary(const std::wstring& dictPath);

    private:
        void BuildGraph(const std::wstring& input);
        void FindPaths(int pos, DAGPath current, std::vector<DAGPath>& paths);
        double CalculatePathScore(const DAGPath& path);

        std::unordered_map<int, std::vector<DAGNode>> m_graph;
        std::unordered_map<std::wstring, double> m_pinyinScores;
        std::wstring m_input;
    };
}