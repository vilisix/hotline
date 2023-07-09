#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <type_traits>
#include <imgui.h>

struct ArgProviderConfig {
	ImVec4 colorSelected = {0.2f, 0.2f, 0.4f, 1.0f};
    ImVec4 colorDefault = {0.25f, 0.25f, 0.25f, 1.0f};
    ImVec4 colorHovered = {0.3f, 0.3f, 0.3f, 1.0f};
    ImVec4 exitButtonColor = {0.8f, 0.2f, 0.25f, 0.8f};
	ImVec4 exitButtonHoveredColor = {0.8f, 0.2f, 0.25f, 1.0f};
    ImVec4 applyButtonColor = {0.31f, 0.8f, 0.36f, 0.8f};
    ImVec4 applyButtonHoveredColor = {0.31f, 0.8f, 0.36f, 1.0f};

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

    explicit ArgProviderBase(std::string name) : _name(std::move(name)) {}

    virtual ~ArgProviderBase() = default;

    ArgumentProvidingState Provide() {
        if (_state == Provided) return _state;
        ImGui::SameLine();
        ImGui::Text("Provide value for ");
        ImGui::SameLine(0,0);
        ImGui::TextColored(argConfig.applyButtonColor, _name.c_str());
        auto windowSize = ImGui::GetContentRegionAvail();
        ImGui::BeginChild(_name.c_str(), {windowSize.x, windowSize.y * 0.8f}, true, argConfig.windowFlags);
        OnGuiProvide();
        ImGui::EndChild();
        auto spaceForFooter = ImGui::GetContentRegionAvail();
        ImGui::BeginChild("footer", spaceForFooter, true, argConfig.windowFlags);
        auto spaceLeft = ImGui::GetContentRegionAvail();
        ImGui::PushStyleColor(ImGuiCol_Button, argConfig.exitButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, argConfig.exitButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, argConfig.exitButtonHoveredColor);
        if ((_canCaptureInput && ImGui::IsKeyPressed(ImGuiKey_Escape)) ||
            ImGui::Button("Cancel [esc]", {spaceLeft.x * 0.25f, spaceLeft.y})) {
            _state = Cancelled;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine(spaceLeft.x * 0.75, 0.f);
        ImGui::PushStyleColor(ImGuiCol_Button, argConfig.applyButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, argConfig.applyButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, argConfig.applyButtonHoveredColor);
        if ((_canCaptureInput && ImGui::IsKeyPressed(ImGuiKey_Enter)) ||
            ImGui::Button("Apply [enter]", {spaceLeft.x * 0.25f, spaceLeft.y})) {
            _state = Provided;
            OnApply();
        }
        ImGui::PopStyleColor(3);
        ImGui::EndChild();

        //todo handle input properly
        if (!_canCaptureInput && !ImGui::IsKeyDown(ImGuiKey_Enter) && !ImGui::IsKeyDown(ImGuiKey_Escape)) {
            _canCaptureInput = true;
        }

        return _state;
    }

    virtual void OnGuiProvide() = 0;    // imgui code for your custom provider
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

    virtual void OnApply() override {
        this->_arg = _presets[_presetCol][_presetRow];
    }

    virtual void OnReset() override {
        _presetCol = 0;
        _presetRow = 0;
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
        }
        _arg = false;
    }
};