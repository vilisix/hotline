#include "Hotline.h"

#include <memory>
#include <sstream>
#include <iostream>
#include "ActionSet.h"

namespace Hotline {

    Hotline::Hotline(std::shared_ptr<ActionSet> set)
            : _set(std::move(set)), _config(std::make_unique<Config>()) {}

    Hotline::Hotline(std::shared_ptr<ActionSet> set, std::unique_ptr<Config> config)
            : _set(std::move(set)), _config(std::move(config)) {}

    void Hotline::Update() {
        HandleKeyInput();

        if (!_isActive) {
            return;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, _config->childRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, _config->frameRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, _config->windowRounding);

        auto io = ImGui::GetIO();
        ImVec2 position{io.DisplaySize.x * _config->windowPos.x, io.DisplaySize.y * _config->windowPos.y};
        ImVec2 size{io.DisplaySize.x * _config->windowSize.x, io.DisplaySize.y * _config->windowSize.y};
        ImGui::SetNextWindowPos(position, ImGuiCond_Always, _config->windowPivot);
        ImGui::SetNextWindowSize(size);
        ImGui::Begin("HotlineWindow", 0, _config->windowFlags);
        ImGui::SetKeyboardFocusHere();
        ImGui::Text(_config->header.c_str());
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::SetWindowFontScale(_config->windowFontScale);
        ImGui::InputText("hotlineInput", _inputBuffer, IM_ARRAYSIZE(_inputBuffer), _config->inputTextFlags);

        ImGui::SetWindowFontScale(1.0f);
        ImGui::Text(GetHeader().c_str());
        ImGui::SetWindowFontScale(_config->windowFontScale);

        HandleTextInput(_inputBuffer);
        DrawVariants(GetCurrentVariantContainer());

        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleVar(3);
        ImGui::End();
    }

    void Hotline::Toggle() {
        _isActive = !_isActive;
        Reset();
    }

    std::vector<ActionVariant> &Hotline::GetCurrentVariantContainer() {
        if (!_config->showRecentActions) {
            return _queryVariants;
        }
        return _input.empty() ? _recentCommands : _queryVariants;
    }

	std::string& Hotline::GetHeader() {
        if(!_queryVariants.empty()) return _config->listHeaderSearch;
        if(_config->showRecentActions && !_recentCommands.empty()) return _config->listHeaderRecents;
        return _config->listHeaderNone;
	}

	void Hotline::Reset() {
        _input.clear();
        _prevActionName.clear();
        _currentActionName.clear();
        _actionArguments.clear();
        _selectionIndex = 0;
        _inputBuffer[0] = '\0';
        _queryVariants = _set->FindVariants("");
    }

    void Hotline::HandleKeyInput() {
        if (ImGui::IsKeyPressed(_config->toggleKey, false)) {
            Toggle();
        }

        if (!_isActive) {
            return;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape, false) && _input.empty()) {
            Toggle();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)
            || (ImGui::IsKeyPressed(ImGuiKey_Tab) && !ImGui::IsKeyDown(ImGuiKey_LeftShift))) {
            _selectionIndex++;
            if (_selectionIndex >= GetCurrentVariantContainer().size()) {
                _selectionIndex = 0;
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)
            || (ImGui::IsKeyPressed(ImGuiKey_Tab) && ImGui::IsKeyDown(ImGuiKey_LeftShift))) {
            _selectionIndex--;
            if (_selectionIndex < 0) {
                _selectionIndex = GetCurrentVariantContainer().size() - 1;
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
            HandleApplyCommand();
        }
    }

    void Hotline::HandleTextInput(const std::string &input) {
        if (_input != input) {
            _input = input;
            SplitInput();
            if (_prevActionName != _currentActionName) {
                _prevActionName = _currentActionName;
                _selectionIndex = 0;
                _queryVariants = _set->FindVariants(_currentActionName);
            }
        }
    }

    void Hotline::HandleApplyCommand() {
        if (_config->showRecentActions && _input.empty() && !_recentCommands.empty()) {
            _set->ExecuteAction(_recentCommands[_selectionIndex].actionName,
                                _recentCommands[_selectionIndex].actionArguments);
            auto currentCommandIter = _recentCommands.begin() + _selectionIndex;
            std::rotate(_recentCommands.begin(), currentCommandIter, currentCommandIter + 1);
        } else if (!_queryVariants.empty()) {
            auto actionName = _queryVariants[_selectionIndex].actionName;
            auto actionArgs = _actionArguments;
            _set->ExecuteAction(actionName, _actionArguments);
            auto executedAction = std::find_if(_recentCommands.begin(), _recentCommands.end(),
                                               [&actionName, &actionArgs](const ActionVariant &variant) {
                                                   return (variant.actionName == actionName) &&
                                                          (variant.actionArguments == actionArgs);
                                               });
            if (executedAction != _recentCommands.end()) {
                std::rotate(_recentCommands.begin(), executedAction, executedAction + 1);
            } else {
                _actionArguments.resize(_queryVariants[_selectionIndex].actionArguments.size());
                _recentCommands.push_back({_queryVariants[_selectionIndex].actionName, _actionArguments});
                std::rotate(_recentCommands.begin(), _recentCommands.end() - 1, _recentCommands.end());
            }
        }
        Toggle();
    }

    void Hotline::SplitInput() {
        std::istringstream iss(_input);
        std::getline(iss, _currentActionName, ' ');

        std::string word;
        int argIndex = 0;
        while (std::getline(iss, word, ' ')) {
            if (argIndex >= _actionArguments.size()) {
                _actionArguments.push_back(word);
            } else if (word != _actionArguments[argIndex]) {
                _actionArguments[argIndex] = word;
            }
            argIndex++;
        }
        while (_actionArguments.size() != argIndex) {
            _actionArguments.pop_back();
        }
    }

    void Hotline::DrawVariants(const std::vector<ActionVariant> variants) {
        for (size_t variantIndex = 0; variantIndex < variants.size(); variantIndex++) {
            if (variantIndex == _selectionIndex) {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, _config->variantBackground);
            }

            ImVec2 textSize = ImGui::CalcTextSize(variants[variantIndex].actionName.c_str());
            ImVec2 childSize = {ImGui::GetContentRegionAvail().x, textSize.y * _config->variantHeightMultiplier};
            ImGui::BeginChild(("action" + std::to_string(variantIndex)).c_str(), childSize, 0, _config->variantFlags);
            if (variantIndex == _selectionIndex) {
                ImGui::PopStyleColor();
            }

            ImVec2 textPosition{_config->variantTextHorOffset, (ImGui::GetContentRegionAvail().y - textSize.y) * 0.5f};
            ImGui::SetCursorPos(textPosition);
            DrawVariant(variants[variantIndex]);
            ImGui::EndChild();
        }
    }

    void Hotline::DrawVariant(const ActionVariant &variant) {
        auto childSize = ImGui::GetContentRegionAvail();
        if (variant.fuzzyResult.positions.empty()) {
            ImGui::Text(variant.actionName.c_str());
        } else {
            int highlightIdx = 0;
            char scoreBuf[3] = "x\0";

            for (size_t i = 0; i < variant.actionName.size(); i++) {
                scoreBuf[0] = variant.actionName[i];
                if (highlightIdx < variant.fuzzyResult.positions.size() &&
                    i == variant.fuzzyResult.positions[highlightIdx]) {
                    ImGui::TextColored(_config->variantMatchLettersColor, scoreBuf);
                    highlightIdx++;
                } else {
                    ImGui::Text(scoreBuf);
                }
                if (i != variant.actionName.size() - 1) {
                    ImGui::SameLine(0, 0);
                }
            }
        }
		if (_currentActionName.size() < _input.size())
		{
			for (int i = 0; i < variant.actionArguments.size(); i++)
			{
				float offsetFromStart = i == 0 ? childSize.x * 0.4f : 0.f;
				ImGui::SameLine(offsetFromStart, 0);
				ImGui::TextColored(_config->variantArgumentsColor, "[");
				ImGui::SameLine(0, 0);
				ImGui::TextColored(_config->variantArgumentsColor, variant.actionArguments[i].c_str());
				if (i < _actionArguments.size())
				{
					ImGui::SameLine(0, 0);
					ImGui::Text(":");
					ImGui::SameLine(0, 0);
					ImGui::TextColored(_config->variantInputColor, _actionArguments[i].c_str());
				}
				ImGui::SameLine(0, 0);
				ImGui::TextColored(_config->variantArgumentsColor, "]");
			}
		}
        
    }
}
