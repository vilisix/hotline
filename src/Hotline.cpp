#include "Hotline.h"

#include <memory>
#include "imgui.h"
#include "ActionSet.h"

namespace Hotline{

Hotline::Hotline(std::shared_ptr<ActionSet> set)
: _set(set)
{}

void Hotline::Update()
{
    HandleKeyInput();

    if(!_isActive){
        return;
    }

    auto io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.25f), ImGuiCond_Always, ImVec2(0.5f,0.0f));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.5f, 0.f));
	ImGui::Begin("HotlineWindow", 0, ImGuiWindowFlags_NoTitleBar 
														| ImGuiWindowFlags_NoMove 
														| ImGuiWindowFlags_AlwaysAutoResize
														| ImGuiWindowFlags_NoScrollbar);
    ImGui::SetKeyboardFocusHere();
    ImGui::Text("hotline");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::SetWindowFontScale(1.4f);
    ImGui::InputText("hotline", _inputBuffer, IM_ARRAYSIZE(_inputBuffer), ImGuiInputTextFlags_EscapeClearsAll 
															| ImGuiInputTextFlags_AlwaysOverwrite);

    HandleTextInput(_inputBuffer);
    DrawVariants();
    ImGui::End();
}

void Hotline::Toggle()
{
    _isActive = !_isActive;
    if(_isActive){
        Reset();
    }
}

void Hotline::Reset()
{
    _textInput.clear();
    _selectionIndex = 0;
    _variants = _set->GetActionVariants("");
}

void Hotline::HandleKeyInput()
{
    if(ImGui::IsKeyPressed(ImGuiKey_F1, false)){
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
        if(_selectionIndex >= _variants.size()){
            _selectionIndex = 0;
        }
    }

    if(ImGui::IsKeyPressed(ImGuiKey_UpArrow)
    || (ImGui::IsKeyPressed(ImGuiKey_Tab) && ImGui::IsKeyDown(ImGuiKey_LeftShift))){
        _selectionIndex--;
        if (_selectionIndex < 0){
            _selectionIndex = _variants.size() - 1;
        }
    }

    if(ImGui::IsKeyPressed(ImGuiKey_Enter, false)){
        _set->ExecuteAction(_variants[_selectionIndex].target);
        Toggle();
    }
}
void Hotline::HandleTextInput(const std::string &input)
{
    if(_textInput != input){
        _textInput = input;
        _selectionIndex = 0;
        _variants = _set->GetActionVariants(_textInput);
    }
}
void Hotline::DrawVariants()
{
    for (size_t variantIndex = 0; variantIndex < _variants.size(); variantIndex++){
        if(variantIndex == _selectionIndex){
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.4f, 1.0f));
        }

        ImVec2 textSize = ImGui::CalcTextSize(_variants[variantIndex].target.c_str());
        ImGui::BeginChild(("variant" + std::to_string(variantIndex)).c_str(), {ImGui::GetContentRegionAvail().x, textSize.y * 1.25f}, 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize);
        if(variantIndex == _selectionIndex){
            ImGui::PopStyleColor();
        }

        ImVec2 textPosition{5.f, (ImGui::GetContentRegionAvail().y - textSize.y) * 0.5f};
        ImGui::SetCursorPos(textPosition);
        DrawVariant(_variants[variantIndex]);
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
                ImGui::TextColored(ImVec4(0.996f, 0.447f, 0.298f, 1.0f), scoreBuf);
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
