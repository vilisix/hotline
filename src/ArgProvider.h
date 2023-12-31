#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <type_traits>
#include <imgui.h>

struct ArgProviderConfig {
    float scaleFactor = 1.0f;
    float windowFontScale = 1.2f;
    float windowHotkeyScale = 0.7f;

	ImVec4 colorSelected = {0.2f, 0.2f, 0.4f, 0.9f};
    ImVec4 colorDefault = {0.25f, 0.25f, 0.25f, 0.75f};
    ImVec4 colorHovered = {0.3f, 0.3f, 0.3f, 0.9f};
    ImVec4 exitButtonColor = {0.8f, 0.2f, 0.25f, 0.8f};
	ImVec4 exitButtonHoveredColor = {0.8f, 0.2f, 0.25f, 0.95f};
    ImVec4 applyButtonColor = {0.31f, 0.8f, 0.36f, 0.6f};
    ImVec4 applyButtonHoveredColor = {0.31f, 0.8f, 0.36f, 0.8f};
	ImVec4 hotkeyColor = {0.8f, 0.8f, 0.8f, 1.0f};
    float customProviderButtonWidth = 0.4f;
    ImGuiKey customProviderButtonKey = ImGuiKey_Tab;
    ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_EscapeClearsAll;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar
                                       | ImGuiWindowFlags_NoMove
                                       | ImGuiWindowFlags_AlwaysAutoResize
                                       | ImGuiWindowFlags_NoScrollbar
                                       | ImGuiWindowFlags_NoNav;
};

static ArgProviderConfig argConfig;

template<typename T>
struct ArgProviderBase {
    T _arg;
    std::string _name;
    ArgumentProvidingState _state = InProgress;
    bool _canCaptureInput = false;
    char _inputBuffer[128] = "";

    explicit ArgProviderBase(std::string name) : _name(std::move(name)) {}

    virtual ~ArgProviderBase() = default;

    ArgumentProvidingState Provide() {
        if (_state == Provided) return _state;
        ImGui::SameLine();
        ImGui::Text("Provide value for ");
        ImGui::SameLine(0,0);
        ImGui::TextColored(argConfig.applyButtonColor, _name.c_str());
        auto windowSize = ImGui::GetContentRegionAvail();
        ImGui::BeginChild(_name.c_str(), {windowSize.x, windowSize.y * 0.7f}, true, argConfig.windowFlags);
        OnGuiProvide();
        ImGui::EndChild();
        ImGui::BeginChild((_name + "_custom").c_str(), {windowSize.x, windowSize.y * 0.1f}, true, argConfig.windowFlags);
        OnCustomProvide();
        ImGui::EndChild();

        auto spaceForFooter = ImGui::GetContentRegionAvail();
        ImGui::BeginChild("footer", spaceForFooter, true, argConfig.windowFlags);
        auto spaceLeft = ImGui::GetContentRegionAvail();

        auto cursorBeforeButton = ImGui::GetCursorPos();

        ImGui::PushStyleColor(ImGuiCol_Button, argConfig.exitButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, argConfig.exitButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, argConfig.exitButtonHoveredColor);
        if ((_canCaptureInput && ImGui::IsKeyPressed(ImGuiKey_Escape)) ||
            ImGui::Button("Cancel", {spaceLeft.x * 0.25f, spaceLeft.y})) {
            _state = Cancelled;
        }
        ImGui::PopStyleColor(3);

        auto cursorPointAfterButton = ImGui::GetCursorPos();

		ImGui::SetWindowFontScale(argConfig.windowHotkeyScale * argConfig.scaleFactor);
		ImGui::SetCursorPos(cursorBeforeButton);
		ImGui::TextColored(argConfig.hotkeyColor, ("[esc]"));
		ImGui::SetWindowFontScale(argConfig.windowFontScale * argConfig.scaleFactor);
		ImGui::SetCursorPos(cursorPointAfterButton);

        ImGui::SameLine(spaceLeft.x * 0.75, 0.f);
        cursorBeforeButton = ImGui::GetCursorPos();

        ImGui::PushStyleColor(ImGuiCol_Button, argConfig.applyButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, argConfig.applyButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, argConfig.applyButtonHoveredColor);
        if ((_canCaptureInput && ImGui::IsKeyPressed(ImGuiKey_Enter)) ||
            ImGui::Button("Apply", {spaceLeft.x * 0.25f, spaceLeft.y})) {
            _state = Provided;
            OnApply();
        }
        ImGui::PopStyleColor(3);

        cursorPointAfterButton = ImGui::GetCursorPos();

		ImGui::SetWindowFontScale(argConfig.windowHotkeyScale * argConfig.scaleFactor);
		ImGui::SetCursorPos(cursorBeforeButton);
		ImGui::TextColored(argConfig.hotkeyColor, ("[enter]"));
		ImGui::SetWindowFontScale(argConfig.windowFontScale * argConfig.scaleFactor);
		ImGui::SetCursorPos(cursorPointAfterButton);

        ImGui::EndChild();

        //todo handle input properly
        if (!_canCaptureInput && !ImGui::IsKeyDown(ImGuiKey_Enter) && !ImGui::IsKeyDown(ImGuiKey_Escape)) {
            _canCaptureInput = true;
        }

        return _state;
    }

    virtual void OnGuiProvide() = 0;    // imgui code for your custom provider
    virtual void OnCustomProvide() = 0;    // imgui code for your custom provider
    virtual void OnApply() = 0;         // _arg probably should be filled here
    virtual void OnReset() = 0;

    virtual void ProvideFromString(const std::string &str) = 0;

    void Reset() {
        _state = InProgress;
        _canCaptureInput = false;
        OnReset();
    }

    operator T() const {
        return _arg;
    }
};

template<typename T>
struct PresetArgProvider : public ArgProviderBase<T> {
    std::vector<std::vector<T>> _presets;
    int _presetRow = 0;
    int _presetCol = 0;

    explicit PresetArgProvider(const std::string &name) : ArgProviderBase<T>(name) {}

    PresetArgProvider(const std::string &name, std::vector<std::vector<T>> &presetArgs) : ArgProviderBase<T>(name),
                                                                                          _presets(presetArgs) {}

    virtual ~PresetArgProvider() = default;

    void OnGuiProvide() override {
        if(ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_UpArrow) ||ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_LeftArrow)){
            if(_presetCol == -1 && _presetRow == -1){
                _presetCol = 0;
                _presetRow = 0;
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            _presetCol++;
            if (_presetCol >= _presets.size()) {
                _presetCol = 0;
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            _presetCol--;
            if (_presetCol < 0) {
                _presetCol = _presets.size() - 1;
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
            _presetRow--;
            if (_presetRow < 0) {
                _presetRow = _presets[0].size() - 1;
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
            _presetRow++;
            if (_presetRow >= _presets[0].size()) {
                _presetRow = 0;
            }
        }

        auto contextSize = ImGui::GetContentRegionAvail();
        ImVec2 buttonSize{contextSize.x / _presets[0].size() - (2.f * _presets[0].size()),
                          contextSize.y / _presets.size() - (1.f * _presets[0].size())};
        for (int i = 0; i < _presets.size(); i++) {
            for (int j = 0; j < _presets[0].size(); j++) {
                if (j > 0) {
                    ImGui::SameLine();
                }
                auto color = (i == _presetCol && j == _presetRow) ? argConfig.colorSelected : argConfig.colorDefault;
                auto hoveredColor = (i == _presetCol && j == _presetRow) ? argConfig.colorSelected : argConfig.colorHovered;
                ImGui::PushStyleColor(ImGuiCol_Button, color);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
                if (ImGui::Button(ToString(_presets[i][j]).c_str(), buttonSize)) {
                    _presetCol = i;
                    _presetRow = j;
                }
                ImGui::PopStyleColor(3);
            }
        }
    }

    void OnCustomProvide() override {
        auto providerSize = ImGui::GetContentRegionAvail();
        bool isSelected = _presetCol == -1 && _presetRow == -1;
        auto color = isSelected ? argConfig.colorSelected : argConfig.colorDefault;
        auto hoveredColor = isSelected ? argConfig.colorSelected : argConfig.colorHovered;

        auto cursorBeforeButton = ImGui::GetCursorPos();
        ImGui::SetWindowFontScale(argConfig.windowHotkeyScale * argConfig.scaleFactor);
        ImGui::TextColored(argConfig.hotkeyColor, ("[tab]"));
        ImGui::SetWindowFontScale(argConfig.windowFontScale * argConfig.scaleFactor);

        ImGui::SetCursorPos(cursorBeforeButton);

        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
        if (ImGui::IsKeyPressed(argConfig.customProviderButtonKey) || ImGui::Button("Custom:", {providerSize.x * argConfig.customProviderButtonWidth, providerSize.y})){
            _presetRow = -1;
            _presetCol = -1;
        }

        auto flags = argConfig.inputTextFlags;
        if(isSelected){
            flags |= ImGuiInputTextFlags_AlwaysOverwrite;
        }else{
            flags = ImGuiInputTextFlags_ReadOnly;
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (isSelected){
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::InputText("hotlineInput", this->_inputBuffer, IM_ARRAYSIZE(this->_inputBuffer), flags);

        ImGui::PopStyleColor(3);
    }

    virtual void OnApply() override {
        if(_presetRow >= 0 && _presetCol >= 0){
            this->_arg = _presets[_presetCol][_presetRow];
        }else{
            ProvideFromString(this->_inputBuffer);
        }
    }

    virtual void OnReset() override {
        _presetCol = 0;
        _presetRow = 0;
        this->_inputBuffer[0] = '\0';
    }

    virtual std::string ToString(const T &arg) = 0;

    virtual void ProvideFromString(const std::string &str) override = 0;
};

template<typename T>
struct ArgProvider : public PresetArgProvider<T> {
    explicit ArgProvider(const std::string &name) : ArgProviderBase<T>(name) {}
};

template<>
struct ArgProvider<int> : public PresetArgProvider<int> {
    explicit ArgProvider(std::string name) : PresetArgProvider<int>(name) {
        _presets = {{1, 100, 1000, 10000},
						{-1, -100, -1000, -10000}};
    }

    ArgProvider(const std::string &name, std::vector<std::vector<int>> presetArgs) : PresetArgProvider<int>(name){
	    _presets = presetArgs;
    }

    std::string ToString(const int &arg) override {
        return std::to_string(arg);
    }

    void ProvideFromString(const std::string &str) override {
        _arg = atoi(str.c_str());
    }
};

template<>
struct ArgProvider<unsigned> : public PresetArgProvider<unsigned> {
    explicit ArgProvider(std::string name) : PresetArgProvider<unsigned>(name) {
        _presets = {{1,  100, 1000, 10000},
						{10, 500, 5000, 50000}};
    }

    ArgProvider(const std::string &name, std::vector<std::vector<unsigned>> presetArgs) : PresetArgProvider<unsigned>(name){
	    _presets = presetArgs;
    }

    std::string ToString(const unsigned &arg) override {
        return std::to_string(arg);
    }

    void ProvideFromString(const std::string &str) override {
        _arg = atoi(str.c_str());
    }
};

template<>
struct ArgProvider<std::string> : public PresetArgProvider<std::string> {
    explicit ArgProvider(std::string name) : PresetArgProvider<std::string>(name) {
        _presets = {{"one",   "two",   "three"},
                    {"four",  "five",  "six"},
                    {"seven", "eight", "nine"}};
    }

    ArgProvider(const std::string &name, std::vector<std::vector<std::string>> presetArgs) : PresetArgProvider<std::string>(name){
	    _presets = presetArgs;
    }

    std::string ToString(const std::string &arg) override {
        return arg;
    }

    void ProvideFromString(const std::string &str) override {
        _arg = str;
    }
};

template<>
struct ArgProvider<bool> : public PresetArgProvider<bool> {
    explicit ArgProvider(std::string name) : PresetArgProvider<bool>(name) {
        _presets = {{true, false}};
    }

    std::string ToString(const bool &arg) override {
        return arg ? "TRUE" : "FALSE";
    }

    void ProvideFromString(const std::string &str) override {
        if (str == "true" || str == "1") {
            _arg = true;
        } else {
			_arg = false;
        }
    }
};