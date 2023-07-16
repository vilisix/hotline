#include "ProviderWindow.h"

#include "ActionSet.h"

void hotline::ProviderWindow::Draw(ActionSet& set) {
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, providerConfig.childRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, providerConfig.frameRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, providerConfig.windowRounding);

    auto io = ImGui::GetIO();
    ImVec2 position{io.DisplaySize.x * providerConfig.providerWindowPos.x, io.DisplaySize.y * providerConfig.providerWindowPos.y};
    ImVec2 size{io.DisplaySize.x * providerConfig.providerWindowSize.x, io.DisplaySize.y * providerConfig.providerWindowSize.y};
    ImGui::SetNextWindowPos(position, ImGuiCond_Always, providerConfig.providerWindowPivot);
    ImGui::SetNextWindowSize(size);
    ImGui::Begin("ArgProviderWindow", 0, providerConfig.windowFlags);
	ImGui::SetWindowFontScale(providerConfig.windowFontScale * providerConfig.scaleFactor);
    set.Update();
    ImGui::End();

    ImGui::PopStyleVar(3);
}