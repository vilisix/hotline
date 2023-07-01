#pragma once

#include <functional>
#include <string>
#include <map>
#include <memory>

#include "FuzzyScorer.h"
#include "Action.h"

namespace Hotline {

    struct ActionVariant {
        std::string actionName;
        std::vector<std::string> actionArguments;
        FuzzyScore fuzzyResult;
    };

    class ActionSet {
    public:
        ActionSet();

        template<typename F, typename... Args>
        void AddAction(const std::string &name, F &&f, Args &&... args) {
            _actions[name] = std::make_unique<Action<std::decay_t<F>, std::remove_cv_t<std::remove_reference_t<Args>>...>>
                    (name, std::forward<F>(f), std::forward<Args>(args)...);
        }

        void ExecuteAction(const std::string &name);

        void ExecuteAction(const std::string &name, const std::vector<std::string> &args);

        std::vector<ActionVariant> FindVariants(const std::string &query);

    private:
        std::map<std::string, std::unique_ptr<BaseAction>> _actions;
        std::unique_ptr<FuzzyScorer> _scorer;
    };
}