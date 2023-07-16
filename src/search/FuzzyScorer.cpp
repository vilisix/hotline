#include "FuzzyScorer.h"

#include <string>

namespace Hotline {

    FuzzyScore FuzzyScorer::GetFuzzyScore(const std::string &query, const std::string &queryLower, int querySize,
                                          const std::string &target, const std::string &targetLower, int targetSize) {
        if (querySize > targetSize) {
            return {};
        }

        std::vector<int> scores(querySize * targetSize, 0);
        std::vector<int> matches(querySize * targetSize, 0);

        for (int queryIndex = 0; queryIndex < querySize; queryIndex++) {
            const int queryIndexOffset = queryIndex * targetSize;
            const int queryIndexPreviousOffset = queryIndexOffset - targetSize;

            const bool queryIndexGtNull = queryIndex > 0;

            const char queryCharAtIndex = query[queryIndex];
            const char queryLowerCharAtIndex = queryLower[queryIndex];

            for (int targetIndex = 0; targetIndex < targetSize; targetIndex++) {
                const bool targetIndexGtNull = targetIndex > 0;

                const int currentIndex = queryIndexOffset + targetIndex;
                const int leftIndex = currentIndex - 1;
                const int diagIndex = queryIndexPreviousOffset + targetIndex - 1;

                const int leftScore = targetIndexGtNull ? scores[leftIndex] : 0;
                const int diagScore = queryIndexGtNull && targetIndexGtNull ? scores[diagIndex] : 0;

                const int matchesSequenceLength = queryIndexGtNull && targetIndexGtNull ? matches[diagIndex] : 0;

                const char targetChar = target[targetIndex];
                const char targetLowerChar = targetLower[targetIndex];

                int score;
                if (!diagScore && queryIndexGtNull) {
                    score = 0;
                } else {
                    score = ComputeCharScore(queryCharAtIndex, queryLowerCharAtIndex, targetChar, targetLowerChar,
                                             targetIndex, matchesSequenceLength);
                }

                const bool isValidScore = score && diagScore + score >= leftScore;
                if (isValidScore) {
                    matches[currentIndex] = matchesSequenceLength + 1;
                    scores[currentIndex] = diagScore + score;
                } else {
                    matches[currentIndex] = -1;
                    scores[currentIndex] = leftScore;
                }
            }
        }

        // find the path from bottom right
        std::vector<int> positions;
        int queryIndex = querySize - 1;
        int targetIndex = targetSize - 1;
        while (queryIndex >= 0 && targetIndex >= 0) {
            const int currentIndex = queryIndex * targetSize + targetIndex;
            const int match = matches[currentIndex];
            if (match == -1) {
                targetIndex--;
            } else {
                positions.push_back(targetIndex);

                queryIndex--;
                targetIndex--;
            }
        }

        std::reverse(positions.begin(), positions.end());
        return {scores[querySize * targetSize - 1], positions};
    }

    int FuzzyScorer::ComputeCharScore(const char &queryChar, const char &queryCharLower, const char &targetChar,
                                      const char &targetCharLower, int targetIdx, int sequenceMatch) {
        int score = 0;

        if (queryCharLower != targetCharLower) {
            return score;
        }

        // Character match bonus
        score += 1;

        // Sequence match bonus
        if (sequenceMatch > 0) {
            score += (sequenceMatch * 5);
        }

        // Same case bonus
        if (queryChar == targetChar) {
            score += 1;
        }

        // Start of word bonus
        if (targetIdx == 0) {
            score += 8;
        }

        // Bonus for uppercase
        if (targetChar != targetCharLower) {
            score += 2;
        }

        return score;
    }

}