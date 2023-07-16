#pragma once
#include <imgui.h>

#include "IActionFrontend.h"

namespace hotline {
	struct ProviderConfig {
		//  window
        float scaleFactor = 1.0f;
        float windowRounding = 10.0f;
        float frameRounding = 5.0f;
        float childRounding = 8.0f;

        float windowFontScale = 1.6f;
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar
                                       | ImGuiWindowFlags_NoMove
                                       | ImGuiWindowFlags_AlwaysAutoResize
                                       | ImGuiWindowFlags_NoScrollbar
                                       | ImGuiWindowFlags_NoNav;

		//provider window
        ImVec2 providerWindowPos = {0.5f, 0.5f};   // relative to display size
        ImVec2 providerWindowPivot = {0.5f, 0.5f};
        ImVec2 providerWindowSize = {0.7f, 0.7f};  // relative to display size
	};

	static ProviderConfig providerConfig;

	class ProviderWindow : public IProviderFrontend {
	public:
		~ProviderWindow() override = default;

		void Draw(ActionSet& set) override;
	};
}

