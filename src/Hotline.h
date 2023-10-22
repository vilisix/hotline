#pragma once

#include <memory>
#include <string>
#include "imgui.h"
#include "ActionSet.h"
#include "IActionFrontend.h"

namespace hotline {
    class ActionSet;

    struct Config {
        //  main
        ImGuiKey toggleKey = ImGuiKey_F1;
        bool showRecentActions = true;

        //  window
        float scaleFactor = 1.0f;
        float windowRounding = 10.0f;
        float frameRounding = 5.0f;
        float childRounding = 8.0f;
        ImVec2 windowPos = {0.5f, 0.2f};   // relative to display size
        ImVec2 windowPivot = {0.5f, 0.f};
        ImVec2 windowSize = {0.5f, 0.f};  // relative to display size
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar
                                       | ImGuiWindowFlags_NoMove
                                       | ImGuiWindowFlags_AlwaysAutoResize
                                       | ImGuiWindowFlags_NoScrollbar
                                       | ImGuiWindowFlags_NoNav;
        std::string header = "hotline search";
        std::string listHeaderRecents = "history:";
        std::string listHeaderSearch = "fuzzy search:";
        std::string listHeaderNone = "";
        float windowHeaderScale = 1.0f;
        float windowFontScale = 1.6f;
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
        const ImVec4 bgColor = {0.15f,0.15f,0.15f,1.f};
        const ImVec4 inputBgColor = {0.15f,0.15f,0.15f,1.f};
        const ImVec4 headerColor = {0.4f,0.4f,0.4f,1.f};
    };

    static Config hotlineConfig;

    class Hotline : public IActionFrontend {
    public:
		~Hotline() override = default;

        void Draw(ActionSet& set) override;
		void Reset() override;
        void SetExitCallback(std::function<void()> callback) override;
    protected:
        std::vector<ActionVariant> &GetCurrentVariantContainer();

        std::string &GetHeader();

        void HandleKeyInput(ActionSet& set);

        void HandleTextInput(const std::string &input, ActionSet& set);

        void HandleApplyCommand(ActionSet& set);

        void DrawVariants(const std::vector<ActionVariant> variants);

        void DrawVariant(const ActionVariant &variant);

        char _inputBuffer[128] = "";
    private:
        int _selectionIndex = 0;

        std::string _input;
        std::string _prevActionName;
        std::string _currentActionName;
        std::vector<std::string> _actionArguments;

        std::vector<ActionVariant> _queryVariants;
        std::vector<ActionVariant> _recentActions;

        std::function<void()> _onExitCallback;

        void SplitInput();

        void OnPreWindow() const;

        void OnPostWindow() const;

        void OnWindowBegin() const;

        void OnTextInput();

        void OnWindowEnd() const;

        void ExecuteRecentAction(ActionSet &set);

        void ExecuteSearchAction(ActionSet &set);
    };
}