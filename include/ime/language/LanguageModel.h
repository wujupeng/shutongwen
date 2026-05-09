#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace ShuTongWen
{
    namespace Language
    {
        struct Segment
        {
            std::wstring text;
            std::wstring type;
            double score;
            size_t start;
            size_t end;

            Segment() : score(0.0), start(0), end(0) {}
            Segment(const std::wstring& t, const std::wstring& ty, double s, size_t st, size_t e)
                : text(t), type(ty), score(s), start(st), end(e) {}
        };

        struct SegmentPath
        {
            std::vector<Segment> segments;
            double total_score;
            
            SegmentPath() : total_score(0.0) {}
            
            bool operator<(const SegmentPath& other) const { return total_score < other.total_score; }
            bool operator>(const SegmentPath& other) const { return total_score > other.total_score; }
        };

        struct Candidate
        {
            std::wstring text;
            std::wstring pinyin;
            double score;
            int frequency;
            std::wstring category;

            Candidate() : score(0.0), frequency(0) {}
            Candidate(const std::wstring& t, const std::wstring& p, double s, int f, const std::wstring& c = L"")
                : text(t), pinyin(p), score(s), frequency(f), category(c) {}
        };

        class LanguageModel
        {
        public:
            virtual ~LanguageModel() = default;

            virtual bool Initialize(const std::wstring& modelPath) = 0;
            virtual void Uninitialize() = 0;

            virtual std::vector<SegmentPath> Segment(const std::wstring& input, size_t maxPaths = 5) = 0;
            virtual std::vector<Candidate> GenerateCandidates(const std::vector<Segment>& segments, size_t limit = 10) = 0;
            virtual double CalculateScore(const std::vector<std::wstring>& context, const std::wstring& word) = 0;

            virtual bool IsInitialized() const = 0;
        };

        std::unique_ptr<LanguageModel> CreateLanguageModel();
    }
}