#include "Window.h"
#include "Settings.h"

// Main Draw function for AddItem, called by Frame::Draw()
void AddItemWindow::Draw(Settings::Style& a_style, Settings::Config& a_config)
{
	constexpr auto _flags = ImGuiOldColumnFlags_NoResize;
	ImGui::BeginColumns("##HorizontalSplit", 2, _flags);

	const float width = ImGui::GetWindowWidth();
	ImGui::SetColumnWidth(0, width * 0.75f);

	// Left Column
	ShowSearch(a_style, a_config);
	ShowModSelection(a_style, a_config);
	ShowAdvancedOptions(a_style, a_config);
	ShowFormTable(a_style, a_config);
	ImGui::NextColumn();
	ShowActions(a_style, a_config);
	ImGui::EndColumns();

	if (openBook != nullptr) {
		ShowBookPreview();
	}
}

void AddItemWindow::Init()
{
	g_DescriptionFrameworkInterface = DescriptionFrameworkAPI::GetDescriptionFrameworkInterface001();
}