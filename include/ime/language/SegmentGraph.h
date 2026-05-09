#pragma once

#include "LanguageModel.h"
#include <unordered_map>
#include <vector>

namespace ShuTongWen
{
    namespace Language
    {
        class SegmentGraph
        {
        public:
            SegmentGraph();
            ~SegmentGraph();

            void BuildGraph(const std::wstring& input);
            std::vector<SegmentPath> FindBestPaths(size_t maxPaths = 5);

            void AddSegmentRule(const std::wstring& pattern, const std::wstring& type, double score);
            void LoadDictionary(const std::wstring& dictPath);

        private:
            void FindPaths(size_t pos, SegmentPath current, std::vector<SegmentPath>& paths);
            double CalculatePathScore(const SegmentPath& path);
            
            std::unordered_map<size_t, std::vector<Segment>> m_graph;
            std::unordered_map<std::wstring, double> m_dictionary;
            std::wstring m_input;
        };
    }
}