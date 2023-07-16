#include "Hotline.h"

#include <memory>
#include <sstream>
#include <iostream>
#include "ActionSet.h"

namespace Hotline {
	std::vector<ActionVariant> &Hotline::GetCurrentVariantContainer() {
        if (!hotlineConfig.showRecentActions) {
            return _queryVariants;
        }
        return _input.empty() ? _recentCommands : _queryVariants;
    }

	std::string& Hotline::GetHeader() {
        if(!_queryVariants.empty()) return hotlineConfig.listHeaderSearch;
        if(hotlineConfig.showRecentActions && !_recentCommands.empty()) return hotlineConfig.listHeaderRecents;
        return hotlineConfig.listHeaderNone;
	}

	void Hotline::Draw(ActionSet& set) {
        HandleKeyInput(set);

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, hotlineConfig.childRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, hotlineConfig.frameRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, hotlineConfig.windowRounding);

        auto io = ImGui::GetIO();
        ImVec2 position{io.DisplaySize.x * hotlineConfig.windowPos.x, io.DisplaySize.y * hotlineConfig.windowPos.y};
        ImVec2 size{io.DisplaySize.x * hotlineConfig.windowSize.x, io.DisplaySize.y * hotlineConfig.windowSize.y};
        ImGui::SetNextWindowPos(position, ImGuiCond_Always, hotlineConfig.windowPivot);
        ImGui::SetNextWindowSize(size);
        ImGui::Begin("HotlineWindow", 0, hotlineConfig.windowFlags);
        ImGui::SetKeyboardFocusHere();
        ImGui::Text(hotlineConfig.header.c_str());
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::SetWindowFontScale(hotlineConfig.windowFontScale * hotlineConfig.scaleFactor);
        ImGui::InputText("hotlineInput", _inputBuffer, IM_ARRAYSIZE(_inputBuffer), hotlineConfig.inputTextFlags);

        ImGui::SetWindowFontScale(hotlineConfig.windowHeaderScale * hotlineConfig.scaleFactor);
        ImGui::Text(GetHeader().c_str());
        ImGui::SetWindowFontScale(hotlineConfig.windowFontScale * hotlineConfig.scaleFactor);

        HandleTextInput(_inputBuffer, set);
        DrawVariants(GetCurrentVariantContainer());

        ImGui::SetWindowFontScale(hotlineConfig.windowHeaderScale * hotlineConfig.scaleFactor);
        ImGui::PopStyleVar(3);
        ImGui::End();
	}

	void Hotline::Reset() {
        _input.clear();
        _prevActionName.clear();
        _currentActionName.clear();
        _actionArguments.clear();
        _selectionIndex = 0;
        _inputBuffer[0] = '\0';
    }

	void Hotline::SetExitCallback(std::function<void()> callback) {
        _onExitCallback = callback;
	}

	void Hotline::HandleKeyInput(ActionSet& set) {
        if (ImGui::IsKeyPressed(ImGuiKey_Escape, false) && _input.empty()) {
            _onExitCallback();
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
            HandleApplyCommand(set);
        }
    }

    void Hotline::HandleTextInput(const std::string &input, ActionSet& set) {
        if (_input != input) {
            _input = input;
            SplitInput();
            if (_prevActionName != _currentActionName) {
                _prevActionName = _currentActionName;
                _selectionIndex = 0;
                _queryVariants = set.FindVariants(_currentActionName);
            }
        }
    }

    void Hotline::HandleApplyCommand(ActionSet& set) {
        ActionStartResult applyCommandResult = Success;
        if (hotlineConfig.showRecentActions && _input.empty() && !_recentCommands.empty()) {
            _currentActionName = _recentCommands[_selectionIndex].actionName;
            applyCommandResult = set.ExecuteAction(_recentCommands[_selectionIndex].actionName,
                                _recentCommands[_selectionIndex].actionArguments);
            auto currentCommandIter = _recentCommands.begin() + _selectionIndex;
            std::rotate(_recentCommands.begin(), currentCommandIter, currentCommandIter + 1);
        } else if (!_queryVariants.empty()) {
            auto actionName = _queryVariants[_selectionIndex].actionName;
            auto actionArgs = _actionArguments;
            _currentActionName = actionName;
            applyCommandResult = set.ExecuteAction(actionName, _actionArguments);
            auto executedAction = std::find_if(_recentCommands.begin(), _recentCommands.end(),
                                               [&actionName, &actionArgs](const ActionVariant &variant) {
                                                   return (variant.actionName == actionName) &&
                                                          (variant.actionArguments == actionArgs);
                                               });
            if (executedAction != _recentCommands.end()) {
                std::rotate(_recentCommands.begin(), executedAction, executedAction + 1);
            } else {
                if (_actionArguments.size() > _queryVariants[_selectionIndex].actionArguments.size()) {
					_actionArguments.resize(_queryVariants[_selectionIndex].actionArguments.size());
                }
                _recentCommands.push_back({_queryVariants[_selectionIndex].actionName, _actionArguments});
                std::rotate(_recentCommands.begin(), _recentCommands.end() - 1, _recentCommands.end());
            }
        }
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
                ImGui::PushStyleColor(ImGuiCol_ChildBg, hotlineConfig.variantBackground);
            }

            ImVec2 textSize = ImGui::CalcTextSize(variants[variantIndex].actionName.c_str());
            ImVec2 childSize = {ImGui::GetContentRegionAvail().x, textSize.y * hotlineConfig.variantHeightMultiplier};
            ImGui::BeginChild(("action" + std::to_string(variantIndex)).c_str(), childSize, 0, hotlineConfig.variantFlags);
            if (variantIndex == _selectionIndex) {
                ImGui::PopStyleColor();
            }

            ImVec2 textPosition{hotlineConfig.variantTextHorOffset, (ImGui::GetContentRegionAvail().y - textSize.y) * 0.5f};
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
                    ImGui::TextColored(hotlineConfig.variantMatchLettersColor, scoreBuf);
                    highlightIdx++;
                } else {
                    ImGui::Text(scoreBuf);
                }
                if (i != variant.actionName.size() - 1) {
                    ImGui::SameLine(0, 0);
                }
            }
        }
		if (_currentActionName.size() < _input.size() || _queryVariants.empty())
		{
			for (int i = 0; i < variant.actionArguments.size(); i++)
			{
				float offsetFromStart = i == 0 ? childSize.x * 0.4f : 0.f;
				ImGui::SameLine(offsetFromStart, -2);
				ImGui::TextColored(hotlineConfig.variantArgumentsColor, variant.actionArguments[i].c_str());
				if (i < _actionArguments.size())
				{
					ImGui::SameLine(0, 0);
					ImGui::TextColored(hotlineConfig.variantArgumentsColor, ":");
					ImGui::SameLine(0, 0);
					ImGui::TextColored(hotlineConfig.variantInputColor, _actionArguments[i].c_str());
				}
				ImGui::SameLine(0, 0);
			}
		}
        
    }
}
