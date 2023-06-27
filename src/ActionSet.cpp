#include "ActionSet.h"

#include <algorithm>
#include <functional>

#include "FuzzyScorer.h"
#include <iostream>

namespace Hotline{

ActionSet::ActionSet() : _scorer(std::make_shared<FuzzyScorer>())
{
}

void ActionSet::AddAction(const std::string &name, std::function<void()> &&func)
{
    _actions[name] = std::move(func);
}

void ActionSet::ExecuteAction(const std::string &name)
{
    _actions[name]();
}

std::vector<FuzzyScore> ActionSet::GetActionVariants(const std::string &query)
{
    std::vector<FuzzyScore> result;

    if(query.empty())
    {
	    for (auto& action : _actions)
	    {
		    result.push_back({action.first, 0, {}});
	    }
        return std::move(result);
    }

    for(auto& action : _actions){
        auto lowerAction = action.first;
        std::transform(lowerAction.begin(), lowerAction.end(), lowerAction.begin(), ::tolower);

        auto lowerQuery = query;
        std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

        auto score = _scorer->GetFuzzyScore(query, lowerQuery, query.size(), action.first, lowerAction, action.first.size());
		if(score.score > 0)
		{
			result.push_back(score);
		}
    }

    std::sort(result.begin(), result.end(), [](FuzzyScore& a, FuzzyScore& b){ return a.score > b.score;});

    return std::move(result);
}

}