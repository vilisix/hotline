#include "Hotline.h"

#include <memory>
#include "ActionSet.h"

namespace Hotline{

Hotline::Hotline(std::shared_ptr<ActionSet> set)
: _set(std::move(set)), _config(std::make_unique<Config>())
{}

Hotline::Hotline(std::shared_ptr<ActionSet> set, std::unique_ptr<Config> config)
: _set(std::move(set)), _config(std::move(config))
{}

void Hotline::Update()
{
    HandleKeyInput();

    if(!_isActive){
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
    ImGui::InputText("hotline", _inputBuffer, IM_ARRAYSIZE(_inputBuffer), _config->inputTextFlags);

    HandleTextInput(_inputBuffer);
    DrawVariants(GetCurrentVariantContainer());

    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleVar(3);
    ImGui::End();
}

void Hotline::Toggle()
{
    _isActive = !_isActive;
    Reset();
}

std::vector<FuzzyScore>& Hotline::GetCurrentVariantContainer() {
    if(!_config->showRecentActions){
        return _queryVariants;
    }
    return _textInput.empty() ? _recentCommands : _queryVariants;
}

void Hotline::Reset()
{
    _textInput.clear();
    _selectionIndex = 0;
    _inputBuffer[0] = '\0';
    _queryVariants = _set->GetActionVariants("");
}

void Hotline::HandleKeyInput()
{
    if(ImGui::IsKeyPressed(_config->toggleKey, false)){
        Toggle();
    }

    if(!_isActive){
        return;
    }

    if(ImGui::IsKeyPressed(ImGuiKey_Escape, false) && _textInput.empty()){
        Toggle();
    }

    if(ImGui::IsKeyPressed(ImGuiKey_DownArrow)
    || (ImGui::IsKeyPressed(ImGuiKey_Tab) && !ImGui::IsKeyDown(ImGuiKey_LeftShift))){
        _selectionIndex++;
        if(_selectionIndex >= GetCurrentVariantContainer().size()){
            _selectionIndex = 0;
        }
    }

    if(ImGui::IsKeyPressed(ImGuiKey_UpArrow)
    || (ImGui::IsKeyPressed(ImGuiKey_Tab) && ImGui::IsKeyDown(ImGuiKey_LeftShift))){
        _selectionIndex--;
        if (_selectionIndex < 0){
            _selectionIndex = GetCurrentVariantContainer().size() - 1;
        }
    }

    if(ImGui::IsKeyPressed(ImGuiKey_Enter, false)){
        HandleApplyCommand();
    }
}
void Hotline::HandleTextInput(const std::string &input)
{
    if(_textInput != input){
        _textInput = input;
        _selectionIndex = 0;
        _queryVariants = _set->GetActionVariants(_textInput);
    }
}

void Hotline::HandleApplyCommand() {
    if(_config->showRecentActions && _textInput.empty() && !_recentCommands.empty())
    {
		_set->ExecuteAction(_recentCommands[_selectionIndex].target);
        auto currentCommandIter = _recentCommands.begin() + _selectionIndex;
        std::rotate(_recentCommands.begin(), currentCommandIter, currentCommandIter + 1);
    }else if(!_queryVariants.empty())
    {
        auto action = _queryVariants[_selectionIndex].target;
		_set->ExecuteAction(action);
        auto executedAction = std::find_if(_recentCommands.begin(), _recentCommands.end(), [&action](const FuzzyScore& score)
        {
	        return score.target == action;
        });
        if(executedAction != _recentCommands.end())
        {
	        std::rotate(_recentCommands.begin(), executedAction, executedAction + 1);
        }else
        {
	        _recentCommands.push_back({_queryVariants[_selectionIndex].target});
            std::rotate(_recentCommands.begin(), _recentCommands.end() - 1, _recentCommands.end());
        }
    }
    Toggle();
}

void Hotline::DrawVariants(const std::vector<FuzzyScore> variants)
{
    for (size_t variantIndex = 0; variantIndex < variants.size(); variantIndex++){
        if(variantIndex == _selectionIndex){
            ImGui::PushStyleColor(ImGuiCol_ChildBg, _config->variantBackground);
        }

        ImVec2 textSize = ImGui::CalcTextSize(variants[variantIndex].target.c_str());
        ImVec2 childSize = {ImGui::GetContentRegionAvail().x, textSize.y * _config->variantHeightMultiplier};
        ImGui::BeginChild(("variant" + std::to_string(variantIndex)).c_str(), childSize, 0, _config->variantFlags);
        if(variantIndex == _selectionIndex){
            ImGui::PopStyleColor();
        }

        ImVec2 textPosition{_config->variantTextHorOffset, (ImGui::GetContentRegionAvail().y - textSize.y) * 0.5f};
        ImGui::SetCursorPos(textPosition);
        DrawVariant(variants[variantIndex]);
        ImGui::EndChild();
    }
}
void Hotline::DrawVariant(const FuzzyScore& variant)
{
    if(variant.positions.empty()){
        ImGui::Text(variant.target.c_str());
    }else{
        int highlightIdx = 0;
        char scoreBuf[3] = "x\0";

        for(size_t i = 0; i < variant.target.size(); i++){
            scoreBuf[0] = variant.target[i];
            if(highlightIdx < variant.positions.size() && i == variant.positions[highlightIdx]){
                ImGui::TextColored(_config->variantMatchLettersColor, scoreBuf);
                highlightIdx++;
            }else{
                ImGui::Text(scoreBuf);
            }
            if(i != variant.target.size() - 1){
                ImGui::SameLine(0, 0);
            }
        }
    }
}
}
