#include "ActionSet.h"

#include <algorithm>
#include <functional>

#include "FuzzyScorer.h"

namespace Hotline {

    ActionSet::ActionSet()
            : _scorer(std::make_unique<FuzzyScorer>()) {}

    ActionStartResult ActionSet::ExecuteAction(const std::string &name, const std::vector<std::string> &args) {
        ActionStartResult result = Success;
        if (auto found = _actions.find(name); found != _actions.end()) {
            result = found->second->Start(args);
            if (result == ActionStartResult::Failure) {
                _currentActionToFill = found->second.get();
            }
        }
        return result;
    }

    ArgumentProvidingState ActionSet::UpdateActionToFill() {
        auto result = _currentActionToFill->UpdateProviding();
        if (result == Provided || result == Cancelled) {
            _currentActionToFill = nullptr;
        }
        return result;
    }

    std::vector<ActionVariant> ActionSet::FindVariants(const std::string &query) {
        std::vector<ActionVariant> result;

        if (query.empty()) {
            // for (auto &action: _actions) {
            //     result.push_back({action.first, action.second->GetArguments(), {0, {}}});
            // }
            // return std::move(result);
            return result;
        }

        for (auto &action: _actions) {
            auto lowerAction = action.first;
            std::transform(lowerAction.begin(), lowerAction.end(), lowerAction.begin(), ::tolower);

            auto lowerQuery = query;
            std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

            auto score = _scorer->GetFuzzyScore(query, lowerQuery, query.size(), action.first, lowerAction,
                                                action.first.size());
            if (score.score > 0) {
                result.push_back({action.first, action.second->GetArguments(), score});
            }
        }

        std::sort(result.begin(), result.end(),
                  [](ActionVariant &a, ActionVariant &b) { return a.fuzzyResult.score > b.fuzzyResult.score; });

        return std::move(result);
    }
}