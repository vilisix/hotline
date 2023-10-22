#pragma once

#include <functional>
#include <string>
#include <map>
#include <memory>

#include "Action.h"
#include "search/FuzzyScorer.h"

namespace hotline {
	struct ActionVariant : public FuzzyScore{
		std::string actionName;
		std::vector<std::string> actionArguments;
	};

	template<typename T, typename VariantType>
	class ActionSetBase {
	public:
		ActionSetBase();

		virtual std::vector<VariantType> FindVariants(const std::string& query) = 0;

	protected:
		std::map<std::string, T> _actions;
		std::unique_ptr<FuzzyScorer> _scorer;
	};

	class ActionSetFunc : public ActionSetBase<std::function<void()>, FuzzyScore> {
	public:
		void AddAction(const std::string& name, std::function<void()> func);
		void ExecuteAction(const std::string& actionName);

		std::vector<FuzzyScore> FindVariants(const std::string &query) override;
	};

	class ActionSetFuncPar : public ActionSetBase<std::unique_ptr<BaseAction>, ActionVariant> {
	public:
		template <typename F, typename... Args>
		void AddAction(const std::string& name, F&& f, Args&&... args) {
			_actions[name] = std::make_unique<Action<
					std::decay_t<F>, std::remove_cv_t<std::remove_reference_t<Args>>...>>
				(name, std::forward<F>(f), std::forward<Args>(args)...);
		}

		void ExecuteAction(const std::string& name, const std::vector<std::string>& args);
		void ExecuteAction(const std::string& actionString);
		std::vector<ActionVariant> FindVariants(const std::string &query) override;
	};

	class ActionSetFuncParProvider : public ActionSetBase<std::unique_ptr<BaseAction>, ActionVariant> {
	public:
		template <typename F, typename... Args>
		void AddAction(const std::string& name, F&& f, Args&&... args) {
			_actions[name] = std::make_unique<Action<
					std::decay_t<F>, std::remove_cv_t<std::remove_reference_t<Args>>...>>
				(name, std::forward<F>(f), std::forward<Args>(args)...);
		}

		void ExecuteAction(const std::string& name, const std::vector<std::string>& args);
		void ExecuteAction(const std::string& actionString);
		std::vector<ActionVariant> FindVariants(const std::string &query) override;

		void Update(); // to IActionBackend
		void Reset(); // to IActionBackend
		ArgumentProvidingState GetState(); // to IActionBackend

	private:
		BaseAction* _currentActionToFill = nullptr; // to IActionBackend
		ArgumentProvidingState _state; // to IActionBackend
	};
}
