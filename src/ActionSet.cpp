#include "ActionSet.h"

#include <algorithm>
#include <functional>
#include <sstream>

namespace hotline {

    ActionSet::ActionSet()
            : _scorer(std::make_unique<FuzzyScorer>()) {}

    ActionStartResult ActionSet::ExecuteAction(const std::string &name, const std::vector<std::string> &args) {
        ActionStartResult result = Success;
        if (auto found = _actions.find(name); found != _actions.end()) {
            result = found->second->Start(args);
            if (result == ActionStartResult::Failure) {
                _state = InProgress;
                _currentActionToFill = found->second.get();
            } else {
				_state = Provided;
            }
        }
        return result;
    }

    void ActionSet::Update() {
        if (_state == InProgress) {
			_state = _currentActionToFill->UpdateProviding();
        }
    }

	void ActionSet::Reset() {
        _currentActionToFill = nullptr;
        _state = None;
	}

	ArgumentProvidingState ActionSet::GetState() {
        return _state;
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

    ActionStartResult ActionSet::ExecuteAction(const std::string &actionString) {
        //todo several actions with ; symbol?
        std::istringstream iss(actionString);
        std::string actionName;
        std::getline(iss, actionName, ' ');

        std::vector<std::string> parsedArgs;
        std::string argument;
        while (std::getline(iss, argument, ' ')) {
            parsedArgs.push_back(argument);
        }

        return ExecuteAction(actionName, parsedArgs);
    }
}