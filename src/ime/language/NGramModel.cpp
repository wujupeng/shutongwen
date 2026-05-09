#include "ime/language/NGramModel.h"
#include "utils/logger.h"
#include <fstream>
#include <algorithm>

namespace ShuTongWen
{
    namespace Language
    {
        NGramModel::NGramModel()
            : m_initialized(false),
              m_totalTokens(0)
        {}

        NGramModel::~NGramModel()
        {
            Uninitialize();
        }

        bool NGramModel::Initialize(const std::wstring& modelPath)
        {
            Logger::Info("Initializing NGramModel...");

            m_unigramCounts.clear();
            m_bigramCounts.clear();
            m_trigramCounts.clear();

            std::wstring unigramPath = modelPath + L"\\unigram.txt";
            std::wstring bigramPath = modelPath + L"\\bigram.txt";
            std::wstring trigramPath = modelPath + L"\\trigram.txt";

            LoadNGramData(unigramPath);
            LoadNGramData(bigramPath);
            LoadNGramData(trigramPath);

            m_initialized = true;
            Logger::Info("NGramModel initialized successfully");
            return true;
        }

        void NGramModel::Uninitialize()
        {
            m_unigramCounts.clear();
            m_bigramCounts.clear();
            m_trigramCounts.clear();
            m_initialized = false;
        }

        void NGramModel::LoadNGramData(const std::wstring& filePath)
        {
            std::wifstream file(filePath);
            if (!file.is_open())
                return;

            std::wstring line;
            while (std::getline(file, line))
            {
                size_t tabPos = line.find(L'\t');
                if (tabPos == std::wstring::npos)
                    continue;

                std::wstring ngram = line.substr(0, tabPos);
                size_t count = 1;

                size_t countPos = line.find(L'\t', tabPos + 1);
                if (countPos != std::wstring::npos)
                {
                    try
                    {
                        count = std::stoull(StringUtils::UTF16ToUTF8(line.substr(countPos + 1)));
                    }
                    catch (...)
                    {
                        count = 1;
                    }
                }

                size_t tokenCount = std::count(ngram.begin(), ngram.end(), L' ') + 1;
                
                if (tokenCount == 1)
                    m_unigramCounts[ngram] = count;
                else if (tokenCount == 2)
                    m_bigramCounts[ngram] = count;
                else if (tokenCount == 3)
                    m_trigramCounts[ngram] = count;

                m_totalTokens += count;
            }

            file.close();
        }

        double NGramModel::GetNGramProbability(const std::vector<std::wstring>& tokens)
        {
            if (tokens.empty())
                return 0.0;

            std::wstring key = StringUtils::Join(tokens, L' ');
            
            double count = 0.0;
            double total = static_cast<double>(m_totalTokens);

            if (tokens.size() == 1)
            {
                auto it = m_unigramCounts.find(key);
                if (it != m_unigramCounts.end())
                    count = it->second;
            }
            else if (tokens.size() == 2)
            {
                auto it = m_bigramCounts.find(key);
                if (it != m_bigramCounts.end())
                    count = it->second;
                
                std::wstring prefix = tokens[0];
                auto prefixIt = m_unigramCounts.find(prefix);
                if (prefixIt != m_unigramCounts.end())
                    total = prefixIt->second;
            }
            else if (tokens.size() == 3)
            {
                auto it = m_trigramCounts.find(key);
                if (it != m_trigramCounts.end())
                    count = it->second;
                
                std::wstring prefix = tokens[0] + L" " + tokens[1];
                auto prefixIt = m_bigramCounts.find(prefix);
                if (prefixIt != m_bigramCounts.end())
                    total = prefixIt->second;
            }

            if (total == 0)
                return 0.0;

            double probability = count / total;
            
            if (probability == 0)
                probability = 1e-10;

            return probability;
        }

        std::vector<SegmentPath> NGramModel::Segment(const std::wstring& input, size_t maxPaths)
        {
            std::vector<SegmentPath> paths;
            return paths;
        }

        std::vector<Candidate> NGramModel::GenerateCandidates(const std::vector<Segment>& segments, size_t limit)
        {
            std::vector<Candidate> candidates;
            return candidates;
        }

        double NGramModel::CalculateScore(const std::vector<std::wstring>& context, const std::wstring& word)
        {
            if (context.empty())
            {
                std::vector<std::wstring> unigram = {word};
                return GetNGramProbability(unigram);
            }

            if (context.size() >= 2)
            {
                std::vector<std::wstring> trigram = {context[context.size() - 2], context.back(), word};
                double trigramProb = GetNGramProbability(trigram);
                if (trigramProb > 0)
                    return trigramProb;
            }

            if (!context.empty())
            {
                std::vector<std::wstring> bigram = {context.back(), word};
                double bigramProb = GetNGramProbability(bigram);
                if (bigramProb > 0)
                    return bigramProb;
            }

            std::vector<std::wstring> unigram = {word};
            return GetNGramProbability(unigram);
        }
    }
}