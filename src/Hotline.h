#pragma once

#include <memory>
#include <string>
#include "FuzzyScorer.h"
#include "imgui.h"
#include "ActionSet.h"

namespace Hotline {
    class ActionSet;

    enum State {
	    Normal,
        WaitingForAction
    };

    struct Config {
        //  main
        ImGuiKey toggleKey = ImGuiKey_F1;
        bool showRecentActions = false;

        //  window
        float windowRounding = 10.0f;
        float frameRounding = 5.0f;
        float childRounding = 8.0f;
        ImVec2 windowPos = {0.5f, 0.25f};   // relative to display size
        ImVec2 windowPivot = {0.5f, 0.f};
        ImVec2 windowSize = {0.5f, 0.f};  // relative to display size
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar
                                       | ImGuiWindowFlags_NoMove
                                       | ImGuiWindowFlags_AlwaysAutoResize
                                       | ImGuiWindowFlags_NoScrollbar
    								   | ImGuiWindowFlags_NoNav;
        std::string header = "hotline";
        std::string listHeaderRecents = "history:";
        std::string listHeaderSearch = "fuzzy search:";
        std::string listHeaderNone = "lightweight framework for fast action executing";
        float windowFontScale = 1.4f;
        //  input
        ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_EscapeClearsAll;
//                                             | ImGuiInputTextFlags_AlwaysOverwrite;
        //  variants
        ImVec4 variantBackground = {0.2f, 0.2f, 0.4f, 1.0f};
        float variantHeightMultiplier = 1.25f;
        ImGuiWindowFlags variantFlags = ImGuiWindowFlags_NoScrollbar
                                        | ImGuiWindowFlags_AlwaysAutoResize;
        float variantTextHorOffset = 5.f;
        ImVec4 variantMatchLettersColor = {0.996f, 0.447f, 0.298f, 1.0f};
        ImVec4 variantArgumentsColor = {0.749f, 0.855f, 0.655f, 0.6f};
        ImVec4 variantInputColor = {0.749f, 0.855f, 0.655f, 1.0f};

        //provider window
        ImVec2 providerWindowPos = {0.5f, 0.5f};   // relative to display size
        ImVec2 providerWindowPivot = {0.5f, 0.5f};
        ImVec2 providerWindowSize = {0.5f, 0.5f};  // relative to display size
    };

    class Hotline {
    public:
        Hotline(std::shared_ptr<ActionSet> set);

        Hotline(std::shared_ptr<ActionSet> set, std::unique_ptr<Config> config);

        virtual ~Hotline() = default;

        virtual void Update();

        virtual void Toggle();

    private:
        void NormalUpdate();
        void WaitingForActionUpdate();

        std::vector<ActionVariant> &GetCurrentVariantContainer();
		std::string& GetHeader();

        void Reset();

        void HandleKeyInput();

        void HandleTextInput(const std::string &input);

        void HandleApplyCommand();

        void DrawVariants(const std::vector<ActionVariant> variants);

        void DrawVariant(const ActionVariant &variant);

        State _state = Normal;
        std::shared_ptr<ActionSet> _set;
        std::unique_ptr<Config> _config;

        bool _isActive = false;
        int _selectionIndex = 0;

        char _inputBuffer[128] = "";
        std::string _input;
        std::string _prevActionName;
        std::string _currentActionName;
        std::vector<std::string> _actionArguments;

        std::vector<ActionVariant> _queryVariants;
        std::vector<ActionVariant> _recentCommands;

        void SplitInput();
    };
}