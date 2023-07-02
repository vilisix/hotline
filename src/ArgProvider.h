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
    std::vector<T> _presets;
    int _presetIdx = 0;
    bool _canCaptureInput = false;

    ImVec4 _colorSelected = {0.749f, 0.855f, 0.655f, 0.6f};
    ImVec4 _colorDefault = {0.749f, 0.855f, 0.655f, 0.3f};

    explicit ArgProviderBase(std::string name) : _name(std::move(name)) {}
    virtual ~ArgProviderBase() = default;

	ArgumentProvidingState Provide() {
        if(_state == Provided) return _state;

        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)
            || (ImGui::IsKeyPressed(ImGuiKey_Tab) && !ImGui::IsKeyDown(ImGuiKey_LeftShift))) {
            _presetIdx++;
            if (_presetIdx >= _presets.size()) {
                _presetIdx = 0;
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)
            || (ImGui::IsKeyPressed(ImGuiKey_Tab) && ImGui::IsKeyDown(ImGuiKey_LeftShift))) {
            _presetIdx--;
            if (_presetIdx < 0) {
                _presetIdx = _presets.size() - 1;
            }
        }

        ImGui::Text(("\nProvide value for " + _name).c_str());
        auto windowSize = ImGui::GetContentRegionAvail();
        ImGui::BeginChild(_name.c_str(), {windowSize.x, windowSize.y * 0.8f}, true);
        for (int i = 0; i < _presets.size(); i++) {
            auto color = i == _presetIdx ? _colorSelected : _colorDefault;
            ImGui::PushStyleColor(ImGuiCol_Button, color);
            if (ImGui::Button(ToString(_presets[i]).c_str(), {-FLT_MIN, 40.f})) {
                _presetIdx = i;
            }
            ImGui::PopStyleColor();
	    }
        ImGui::EndChild();
        auto spaceLeft = ImGui::GetContentRegionAvail();
        if ((_canCaptureInput && ImGui::IsKeyPressed(ImGuiKey_Escape)) || ImGui::Button("Cancel [esc]", {spaceLeft.x * 0.4f, spaceLeft.y * 0.9f})) {
            _state = Cancelled;
        }
        ImGui::SameLine(spaceLeft.x * 0.6);
        if ((_canCaptureInput && ImGui::IsKeyPressed(ImGuiKey_Enter)) || ImGui::Button("Done [enter]", {spaceLeft.x * 0.4f, spaceLeft.y * 0.9f})) {
            _state = Provided;
            _arg = _presets[_presetIdx];
        }

        //todo handle input properly
        if (!_canCaptureInput && !ImGui::IsKeyDown(ImGuiKey_Enter) && !ImGui::IsKeyDown(ImGuiKey_Escape)) {
            _canCaptureInput = true;
        }

        return _state;
    }

	// virtual void OnGuiProvide() = 0;
	virtual std::string ToString(const T& arg) = 0;
    virtual void ProvideFromString(const std::string &str) = 0;

    void Reset() {
	    _state = InProgress;
        _presetIdx = 0;
        _canCaptureInput = false;
    }
    operator T() const {
	    return _arg;
    }
};

template<typename T>
struct ArgProvider : public ArgProviderBase<T> {
	explicit ArgProvider(const std::string& name) : ArgProviderBase<T>(name) {}
};


template<>
struct ArgProvider<int> : public ArgProviderBase<int>{
    explicit ArgProvider(std::string name) : ArgProviderBase<int>(name) {
	    _presets = {0,10,20,30};
    }
    std::string ToString(const int& arg) override {
	    return std::to_string(arg);
    }
	void ProvideFromString(const std::string& str) override {
	    _arg = atoi(str.c_str());
    }
};

template<>
struct ArgProvider<std::string> : public ArgProviderBase<std::string>{
    explicit ArgProvider(std::string name) : ArgProviderBase<std::string>(name) {
	    _presets = {"one","two","three","four","five"};
    }
    std::string ToString(const std::string& arg) override {
	    return arg;
    }
	void ProvideFromString(const std::string& str) override {
	    _arg = str;
    }
};

template<>
struct ArgProvider<bool> : public ArgProviderBase<bool>{
    explicit ArgProvider(std::string name) : ArgProviderBase<bool>(name) {
	    _presets = {true, false};
    }
    std::string ToString(const bool& arg) override {
	    return std::to_string(arg);
    }
	void ProvideFromString(const std::string& str) override {
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