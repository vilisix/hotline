#include "ActionManager.h"
#include <cassert>
#include <imgui.h>

#include "ActionSet.h"

Hotline::ActionManager::ActionManager(std::shared_ptr<ActionSet> set)
: _set(std::move(set)) {}

void Hotline::ActionManager::Update() {
	auto state = _set->GetState();
	if (state == InProgress) {
		ActionUpdate();
		return;
	}

	if (state == Provided) {
		_currentFrontend->Reset();
		_currentFrontend = nullptr;
	}

	if (state != None) {
		_set->Reset();
	}

	if (_currentFrontend) {
		_currentFrontend->Draw(*_set);
	}
}

void Hotline::ActionManager::EnableFrontend(const std::string& name) {
	if (_set->GetState() == InProgress) return;

	const auto found = _frontends.find(name);
	assert(found != _frontends.end());

	if (found->second.get() != _currentFrontend) {
		if (_currentFrontend) {
			_currentFrontend->Reset();
		}
		_currentFrontend = found->second.get();
	}
}

void Hotline::ActionManager::Close() {
	if (_set->GetState() == InProgress) return;

	if (_currentFrontend) {
		_currentFrontend->Reset();
		_currentFrontend = nullptr;
	}
}

void Hotline::ActionManager::AddFrontend(const std::string& name, std::unique_ptr<IActionFrontend> frontend) {
	const auto found = _frontends.find(name);
	assert(found == _frontends.end());

	frontend->SetExitCallback([&](){ Close(); });
	_frontends[name] = std::move(frontend);
}

void Hotline::ActionManager::ActionUpdate() {
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, providerConfig.childRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, providerConfig.frameRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, providerConfig.windowRounding);

    auto io = ImGui::GetIO();
    ImVec2 position{io.DisplaySize.x * providerConfig.providerWindowPos.x, io.DisplaySize.y * providerConfig.providerWindowPos.y};
    ImVec2 size{io.DisplaySize.x * providerConfig.providerWindowSize.x, io.DisplaySize.y * providerConfig.providerWindowSize.y};
    ImGui::SetNextWindowPos(position, ImGuiCond_Always, providerConfig.providerWindowPivot);
    ImGui::SetNextWindowSize(size);
    ImGui::Begin("ArgProviderWindow", 0, providerConfig.windowFlags);
	ImGui::SetWindowFontScale(providerConfig.windowFontScale * providerConfig.scaleFactor);
    _set->Update();
    ImGui::End();

    ImGui::PopStyleVar(3);
}