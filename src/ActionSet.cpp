#include "ActionSet.h"
#include "Action.h"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <memory>
#include <sstream>
#include <string>

namespace hotline {
    template<typename T, typename VariantType>
    ActionSetBase<T, VariantType>::ActionSetBase() : _scorer(std::make_unique<FuzzyScorer>()) {}

    void ActionSetFunc::AddAction(const std::string &name, std::function<void()> func) {
        _actions[name] = func;
    }

    void ActionSetFunc::ExecuteAction(const std::string &actionName) {
        if (auto found = _actions.find(actionName); found != _actions.end()) {
            found->second();
        }
    }

    std::vector<FuzzyScore> ActionSetFunc::FindVariants(const std::string &query) {
        std::vector<FuzzyScore> result;

        if (query.empty()) {
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
                result.push_back(score);
            }
        }

        std::sort(result.begin(), result.end(),
                    [](FuzzyScore &a, FuzzyScore &b) { return a.score > b.score; });

        return std::move(result);
    }
    // ActionSetBase::ActionSetBase()
    //         : _scorer(std::make_unique<FuzzyScorer>()) {}

    // void ActionSet::ExecuteAction(const std::string &name, const std::vector<std::string> &args) {
    //     if (auto found = _actions.find(name); found != _actions.end()) {
    //         ActionStartResult result = found->second->Start(args);
    //         if (result == ActionStartResult::Failure) {
    //             _state = InProgress;
    //             _currentActionToFill = found->second.get();
    //         } else {
	// 			_state = Provided;
    //         }
    //     }
    // }

    // void ActionSet::Update() {
    //     if (_state == InProgress) {
	// 		_state = _currentActionToFill->UpdateProviding();
    //     }
    // }

	// void ActionSet::Reset() {
    //     _currentActionToFill = nullptr;
    //     _state = None;
	// }

	// ArgumentProvidingState ActionSet::GetState() {
    //     return _state;
	// }

    // std::vector<ActionVariant> ActionSet::FindVariants(const std::string &query) {
    //     std::vector<ActionVariant> result;

    //     if (query.empty()) {
    //         // for (auto &action: _actions) {
    //         //     result.push_back({action.first, action.second->GetArguments(), {0, {}}});
    //         // }
    //         // return std::move(result);
    //         return result;
    //     }

    //     for (auto &action: _actions) {
    //         auto lowerAction = action.first;
    //         std::transform(lowerAction.begin(), lowerAction.end(), lowerAction.begin(), ::tolower);

    //         auto lowerQuery = query;
    //         std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    //         auto score = _scorer->GetFuzzyScore(query, lowerQuery, query.size(), action.first, lowerAction,
    //                                             action.first.size());
    //         if (score.score > 0) {
    //             result.push_back({score, action.first, action.second->GetArguments()});
    //         }
    //     }

    //     std::sort(result.begin(), result.end(),
    //               [](ActionVariant &a, ActionVariant &b) { return a.score > b.score; });

    //     return std::move(result);
    // }

    // void ActionSet::ExecuteAction(const std::string &actionString) {
    //     //todo several actions with ; symbol?
    //     std::istringstream iss(actionString);
    //     std::string actionName;
    //     std::getline(iss, actionName, ' ');

    //     std::vector<std::string> parsedArgs;
    //     std::string argument;
    //     while (std::getline(iss, argument, ' ')) {
    //         parsedArgs.push_back(argument);
    //     }

    //     ExecuteAction(actionName, parsedArgs);
    // }
}