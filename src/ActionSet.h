#pragma once
#include <functional>
#include <string>
#include <map>
#include <memory>

#include "FuzzyScorer.h"

namespace Hotline{


class ActionSet{
public:
    ActionSet();
	void AddAction(const std::string& name, std::function<void()>&& func);
    void ExecuteAction(const std::string& name);

    std::vector<FuzzyScore> GetActionVariants(const std::string& query);
private:
    std::map<std::string, std::function<void()>> _actions;
    std::shared_ptr<FuzzyScorer> _scorer;
};

}