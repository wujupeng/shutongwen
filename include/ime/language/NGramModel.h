#pragma once

#include "LanguageModel.h"
#include <unordered_map>
#include <memory>

namespace ShuTongWen
{
    namespace Language
    {
        class NGramModel : public LanguageModel
        {
        public:
            NGramModel();
            ~NGramModel() override;

            bool Initialize(const std::wstring& modelPath) override;
            void Uninitialize() override;

            std::vector<SegmentPath> Segment(const std::wstring& input, size_t maxPaths = 5) override;
            std::vector<Candidate> GenerateCandidates(const std::vector<Segment>& segments, size_t limit = 10) override;
            double CalculateScore(const std::vector<std::wstring>& context, const std::wstring& word) override;

            bool IsInitialized() const override { return m_initialized; }

            void LoadNGramData(const std::wstring& filePath);
            double GetNGramProbability(const std::vector<std::wstring>& tokens);

        private:
            bool m_initialized;
            std::unordered_map<std::wstring, double> m_unigramCounts;
            std::unordered_map<std::wstring, double> m_bigramCounts;
            std::unordered_map<std::wstring, double> m_trigramCounts;
            size_t m_totalTokens;
        };
    }
}