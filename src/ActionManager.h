#pragma once
#include <map>
#include <memory>
#include <string>

#include "IActionFrontend.h"

namespace hotline {
	class ActionManager {
	public:
		explicit ActionManager(std::shared_ptr<ActionSet> set);

		void Update();
		void EnableFrontend(const std::string& name);
		void Close();

		void AddActionFrontend(const std::string& name, std::unique_ptr<IActionFrontend> frontend);
		void SetProviderFrontend(std::unique_ptr<IProviderFrontend> frontend);
	private:
		std::map<std::string, std::unique_ptr<IActionFrontend>> _actionFrontends;
		std::unique_ptr<IProviderFrontend> _providerFrontend;
		IActionFrontend* _currentActionFrontend = nullptr;

		std::shared_ptr<ActionSet> _set;
	};
}