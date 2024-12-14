#include "Frame.h"
#include "Graphic.h"
#include "Menu.h"
#include "Windows/AddItem/AddItem.h"
#include "Windows/Home/Home.h"
#include "Windows/NPC/NPC.h"
#include "Windows/Object/Object.h"
#include "Windows/Teleport/Teleport.h"
#include "Windows/UserSettings/UserSettings.h"

namespace ModExplorerMenu
{
	Frame::ActiveWindow Frame::_activeWindow = Frame::ActiveWindow::Home;

	struct Properties
	{
		ImVec2 screenSize;
		float sidebar_w;
		float sidebar_h;
		float panel_w;
		float panel_h;

		Properties()
		{
			screenSize = ImGui::GetMainViewport()->Size;
			sidebar_w = screenSize.x * 0.10f;
			sidebar_h = screenSize.y * 0.75f;
			panel_w = screenSize.x * 0.60f;
			panel_h = screenSize.y * 0.75f;
		}
	};

	void Frame::Draw(bool is_settings_popped)
	{
		auto& style = Settings::GetSingleton()->GetStyle();
		auto& config = Settings::GetSingleton()->GetConfig();

		if (is_settings_popped) {
			SettingsWindow::DrawPopped();
		}

		Properties window;
		const float center_x = window.screenSize.x * 0.5f;
		const float center_y = (window.screenSize.y * 0.5f) - (window.panel_h * 0.5f);
		const float panel_x = center_x - (window.panel_w * 0.5f) + (window.sidebar_w * 0.5f);
		const float sidebar_x = panel_x - (window.sidebar_w);

		// Draw Sidebar Frame
		static constexpr ImGuiWindowFlags sidebar_flag = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
		auto noFocus = is_settings_popped ? ImGuiWindowFlags_NoBringToFrontOnFocus : 0;
		ImGui::SetNextWindowSize(ImVec2(window.sidebar_w, window.sidebar_h));
		ImGui::SetNextWindowPos(ImVec2(sidebar_x, center_y));

		// ImGui::GetFontSize() * 1.5f

		ImGui::PushFont(style.font.large);
		ImGui::PushStyleColor(ImGuiCol_Header, style.button);
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, style.buttonActive);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, style.buttonHovered);
		ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
		if (ImGui::Begin("##AddItemMenuSideBar", NULL, sidebar_flag + noFocus)) {
			auto iWidth = ImGui::GetContentRegionAvail().x;
			auto iHeight = ImGui::GetWindowSize().y / 12;
			ImGui::Image(GraphicManager::image_library["logo"].texture, ImVec2(iWidth, iHeight));

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

			if (ImGui::Selectable("Home", &b_Home, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5f))) {
				_activeWindow = ActiveWindow::Home;
				ResetSelectable();
			}

			if (ImGui::Selectable("Add Item", &b_AddItem, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5f))) {
				_activeWindow = ActiveWindow::AddItem;
				ResetSelectable();
			}

			if (ImGui::Selectable("Object", &b_Object, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5f))) {
				_activeWindow = ActiveWindow::Object;
				ResetSelectable();
			}

			if (ImGui::Selectable("NPC", &b_NPC, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5f))) {
				_activeWindow = ActiveWindow::NPC;
				ResetSelectable();
			}

			if (ImGui::Selectable("Teleport", &b_Teleport, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5f))) {
				_activeWindow = ActiveWindow::Teleport;
				ResetSelectable();
			}

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

			if (ImGui::Selectable("Settings", &b_Settings, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5f))) {
				_activeWindow = ActiveWindow::Settings;
				ResetSelectable();
			}

			if (ImGui::Selectable("Exit", false, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5f))) {
				Menu::GetSingleton()->Toggle();
			}

			ImGui::End();
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(1);

		// Draw Panel Frame conditionally
		static constexpr ImGuiWindowFlags panel_flag = sidebar_flag;
		ImGui::SetNextWindowSize(ImVec2(window.panel_w, window.panel_h));
		ImGui::SetNextWindowPos(ImVec2(panel_x, center_y));

		ImGui::PushFont(style.font.nano);
		if (ImGui::Begin("##AddItemMenuPanel", NULL, sidebar_flag + noFocus + ImGuiWindowFlags_NoScrollbar + ImGuiWindowFlags_NoScrollWithMouse)) {
			switch (_activeWindow) {
			case ActiveWindow::Home:
				HomeWindow::Draw();
				break;
			case ActiveWindow::AddItem:
				AddItemWindow::Draw(style, config);
				break;
			case ActiveWindow::Object:
				ObjectWindow::Draw(style, config);
				break;
			case ActiveWindow::NPC:
				NPCWindow::Draw(style, config);
				break;
			case ActiveWindow::Teleport:
				TeleportWindow::Draw(style, config);
				break;
			case ActiveWindow::Settings:
				SettingsWindow::Draw();
				break;
			}

			ImGui::End();
		}
		ImGui::PopFont();

		GraphicManager::DrawImage(style.splashImage, ImVec2(window.screenSize.x * 0.5f, window.screenSize.y * 0.5f));
	}

	void Frame::Install()
	{
		// Initalize elements
		GraphicManager::Init();
		AddItemWindow::Init();
		HomeWindow::Init();
		NPCWindow::Init();
		TeleportWindow::Init();
		ObjectWindow::Init();

		RefreshStyle();

		// FIXME: This shouldn't be needed anymore
		Frame::_init.store(true);
	}

	void Frame::RefreshStyle()
	{
		// AddItemWindow::RefreshStyle();
		//HomeWindow::RefreshStyle();
		//SettingsWindow::RefreshStyle();

		// auto& style = Settings::GetSingleton()->GetStyle();
	}
}