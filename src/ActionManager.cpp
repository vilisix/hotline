#include "ActionManager.h"
#include <cassert>
#include <imgui.h>

#include "ActionSet.h"

hotline::ActionManager::ActionManager(std::shared_ptr<ActionSet> set)
: _set(std::move(set)) {}

void hotline::ActionManager::Update() {
	auto state = _set->GetState();
	if (state == InProgress) {
		assert(_providerFrontend);
		_providerFrontend->Draw(*_set);
		return;			
	}

	if (state == Provided) {
		_currentActionFrontend->Reset();
		_currentActionFrontend = nullptr;
	}

	if (state != None) {
		_set->Reset();
	}

	if (_currentActionFrontend) {
		_currentActionFrontend->Draw(*_set);
	}
}

void hotline::ActionManager::EnableFrontend(const std::string& name) {
	if (_set->GetState() == InProgress) return;

	const auto found = _actionFrontends.find(name);
	assert(found != _actionFrontends.end());

	if (found->second.get() != _currentActionFrontend) {
		if (_currentActionFrontend) {
			_currentActionFrontend->Reset();
		}
		_currentActionFrontend = found->second.get();
	}
}

void hotline::ActionManager::Close() {
	if (_set->GetState() == InProgress) return;

	if (_currentActionFrontend) {
		_currentActionFrontend->Reset();
		_currentActionFrontend = nullptr;
	}
}

void hotline::ActionManager::AddActionFrontend(const std::string& name, std::unique_ptr<IActionFrontend> frontend) {
	const auto found = _actionFrontends.find(name);
	assert(found == _actionFrontends.end());

	frontend->SetExitCallback([&](){ Close(); });
	_actionFrontends[name] = std::move(frontend);
}

void hotline::ActionManager::SetProviderFrontend(std::unique_ptr<IProviderFrontend> frontend) {
	_providerFrontend = std::move(frontend);
}