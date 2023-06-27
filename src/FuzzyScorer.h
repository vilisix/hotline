#pragma once
#include <vector>
#include <xstring>

namespace Hotline{

struct FuzzyScore
{
    std::string target;
    int score = 0;
    std::vector<int> positions;
};

class FuzzyScorer{
public:
    FuzzyScore GetFuzzyScore(const std::string& query, const std::string& queryLower, int querySize,
                            const std::string& target, const std::string& targetLower, int targetSize);
private:
    int ComputeCharScore(const char& queryChar, const char& queryCharLower,
						const char& targetChar, const char& targetCharLower,
						int targetIdx, int sequenceMatch);

};

}