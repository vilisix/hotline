#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <type_traits>
#include <imgui.h>

template<typename T>
struct ArgProviderBase {
    T _arg;
    std::string _name;
    ArgumentProvidingState _state = InProgress;
    bool _canCaptureInput = false;
    ImVec4 _colorSelected = {0.749f, 0.855f, 0.655f, 0.6f};
    ImVec4 _colorDefault = {0.749f, 0.855f, 0.655f, 0.3f};

    explicit ArgProviderBase(std::string name) : _name(std::move(name)) {}

    virtual ~ArgProviderBase() = default;

    ArgumentProvidingState Provide() {
        if (_state == Provided) return _state;

        ImGui::Text(("\nProvide value for " + _name).c_str());
        auto windowSize = ImGui::GetContentRegionAvail();
        ImGui::BeginChild(_name.c_str(), {windowSize.x, windowSize.y * 0.8f}, true);
        OnGuiProvide();
        ImGui::EndChild();
        ImGui::Text("");
        ImGui::Separator();
        auto spaceLeft = ImGui::GetContentRegionAvail();
        if ((_canCaptureInput && ImGui::IsKeyPressed(ImGuiKey_Escape)) ||
            ImGui::Button("Cancel [esc]", {spaceLeft.x * 0.4f, spaceLeft.y * 0.9f})) {
            _state = Cancelled;
        }
        ImGui::SameLine(spaceLeft.x * 0.6, 0.f);
        if ((_canCaptureInput && ImGui::IsKeyPressed(ImGuiKey_Enter)) ||
            ImGui::Button("Apply [enter]", {spaceLeft.x * 0.4f, spaceLeft.y * 0.9f})) {
            _state = Provided;
            OnApply();
        }

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
                auto color = (i == _presetCol && j == _presetRow) ? this->_colorSelected : this->_colorDefault;
                ImGui::PushStyleColor(ImGuiCol_Button, color);
                if (ImGui::Button(ToString(_presets[i][j]).c_str(), buttonSize)) {
                    _presetCol = i;
                    _presetRow = j;
                }
                ImGui::PopStyleColor();
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
        _presets = {{1,  100,  1000,  10000},
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

// template<>
// struct ArgProvider<std::string> {
//     std::string _arg;
//     std::string _name;
//     bool _provided = false;
//
//     explicit ArgProvider(std::string name) : _name(std::move(name)) {}
//
//     ArgumentProvidingState Provide() {
//         if(_provided) return ArgumentProvidingState::Provided;
//
//         if (ImGui::Button("provide hello")) {
// 	        _arg = "hello";
//             _provided = true;
//             return ArgumentProvidingState::Provided;
//         }
//
//         return ArgumentProvidingState::InProgress;
//     }
//
//     void ProvideStr(const std::string &str) {
//         _arg = str;
//     }
//
//     operator std::string() const {
//         return _arg;
//     }
// };
//
// template<>
// struct ArgProvider<bool> {
//     bool _arg;
//     std::string _name;
//     bool _provided = false;
//
//     ArgProvider(const std::string &name) : _name(name) {}
//
//     ArgumentProvidingState Provide() {
//         if(_provided) return ArgumentProvidingState::Provided;
//
//         if (ImGui::Button("provide true")) {
// 	        _arg = true;
//             _provided = true;
//             return ArgumentProvidingState::Provided;
//         }
//
//         return ArgumentProvidingState::InProgress;
//     }
//
//     void ProvideStr(const std::string &str) {
//         _arg = (str == "true" || str == "1");
//     }
//
//     operator bool() const {
//         return _arg;
//     }
// };