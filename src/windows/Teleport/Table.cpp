#include "Teleport.h"

#include "Console.h"
#include "Menu.h"
#include "Utils/Util.h"
#include "Windows/Persistent.h"

namespace Modex
{
	// Draws a Copy to Clipboard button on Context popup.
	void TeleportWindow::ShowTeleportContextMenu(Cell& a_cell)
	{
		constexpr auto flags = ImGuiSelectableFlags_DontClosePopups;
		ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

		if (ImGui::Selectable(_T("GENERAL_COPY_WORLDSPACE_NAME"), false, flags)) {
			ImGui::LogToClipboard();
			ImGui::LogText(a_cell.space.c_str());
			ImGui::LogFinish();
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::Selectable(_T("GENERAL_COPY_ZONE_NAME"), false, flags)) {
			ImGui::LogToClipboard();
			ImGui::LogText(a_cell.zone.c_str());
			ImGui::LogFinish();
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::Selectable(_T("GENERAL_COPY_CELL_NAME"), false, flags)) {
			ImGui::LogToClipboard();
			ImGui::LogText(a_cell.cellName.c_str());
			ImGui::LogFinish();
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::Selectable(_T("GENERAL_COPY_EDITOR_ID"), false, flags)) {
			ImGui::LogToClipboard();
			ImGui::LogText(a_cell.editorid.c_str());
			ImGui::LogFinish();
			ImGui::CloseCurrentPopup();
		}

		ImGui::PopStyleVar(1);
	}

	// Draw the table of items
	void TeleportWindow::ShowFormTable(Settings::Style& a_style, Settings::Config& a_config)
	{
		(void)a_config;

		auto results = std::string(_T("Results")) + std::string(" (") + std::to_string(cellList.size()) + std::string(")");
		ImGui::SeparatorText(results.c_str());

		auto rowBG = a_style.showTableRowBG ? ImGuiTableFlags_RowBg : 0;

		ImVec2 table_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
		if (ImGui::BeginTable("##TeleportWindow::Table", columnList.GetTotalColumns(), TeleportTableFlags | rowBG, table_size)) {
			ImGui::TableSetupScrollFreeze(1, 1);
			for (auto& column : columnList.columns) {
				ImGui::TableSetupColumn(column.name.c_str(), column.flags, column.width, column.key);
			}

			ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
			int column_n = 0;
			for (auto& column : columnList.columns) {
				ImGui::TableSetColumnIndex(column_n);
				ImGui::PushID(column.key + 10);
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				ImGui::TableHeader(column.name.c_str());
				ImGui::PopID();

				column_n++;
			}

			if (dirty) {
				ImGui::TableGetSortSpecs()->SpecsDirty = true;
			}

			// Sort our data if sort specs have been changed!
			if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs()) {
				if (sort_specs->SpecsDirty) {
					SortColumnsWithSpecs<std::vector<Cell*>, Cell>(cellList, sort_specs);
					sort_specs->SpecsDirty = false;
					dirty = false;
				}
			}

			ImGuiListClipper clipper;
			ImGuiContext& g = *ImGui::GetCurrentContext();
			ImGuiTable* table = g.CurrentTable;

			int count = 0;
			clipper.Begin(static_cast<int>(cellList.size()));
			while (clipper.Step()) {
				for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
					auto& cell = cellList[row];

					count++;
					auto table_id = std::string("##TeleportWindow::TableIndex-") + std::to_string(count);
					ImGui::PushID(table_id.c_str());

					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					// Overwrite color to hide ugly imgui backdrop on image.
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

					if (ImGui::DisabledCheckbox("##TeleportWindow::FavoriteCheckbox", b_ClickToFavorite, cell->favorite)) {
						PersistentData::GetSingleton()->UpdatePersistentData<Cell*>(cell);
					}

					ImGui::PopStyleColor(3);
					ImGui::PopStyleVar(2);

					bool _itemSelected = false;

					//	Plugin
					ImGui::TableNextColumn();
					ImGui::Text(cell->GetPluginName().data());

					// Space
					ImGui::TableNextColumn();
					ImGui::Text(cell->GetSpace().data());

					// Zone
					ImGui::TableNextColumn();
					ImGui::Text(cell->GetZone().data());

					// CellName
					ImGui::TableNextColumn();
					ImGui::Text(cell->GetCellName().data());

					// EditorID
					ImGui::TableNextColumn();
					ImGui::Text(cell->GetEditorID().data());

					// Input Handlers
					auto curRow = ImGui::TableGetHoveredRow();
					if (curRow == ImGui::TableGetRowIndex()) {
						if (ImGui::IsMouseClicked(0)) {
							_itemSelected = true;
							selectedCell = cell;

							if (b_ClickToTeleport) {
								Console::Teleport(cell->editorid);
								Console::StartProcessThread();
								Menu::GetSingleton()->Close();

								_itemSelected = false;
								selectedCell = nullptr;
							}

							if (b_ClickToFavorite) {
								cell->favorite = !cell->favorite;
								PersistentData::GetSingleton()->UpdatePersistentData<Cell*>(cell);
							}
						}

						if (ImGui::IsMouseClicked(1, true)) {
							ImGui::OpenPopup("ShowTeleportContextMenu");
						}
					}

					if (ImGui::BeginPopup("ShowTeleportContextMenu")) {
						ShowTeleportContextMenu(*cell);
						ImGui::EndPopup();
					}

					// https://github.com/ocornut/imgui/issues/6588#issuecomment-1634424774
					// Sloppy way to handle row highlighting since ImGui natively doesn't support it.
					ImRect row_rect(
						table->WorkRect.Min.x,
						table->RowPosY1,
						table->WorkRect.Max.x,
						table->RowPosY2);
					row_rect.ClipWith(table->BgClipRect);

					bool bHover =
						ImGui::IsMouseHoveringRect(row_rect.Min, row_rect.Max, false) &&
						ImGui::IsWindowHovered(ImGuiHoveredFlags_None) &&
						!ImGui::IsAnyItemHovered();  // optional

					if (bHover || selectedCell == cell) {
						table->RowBgColor[1] = ImGui::GetColorU32(ImGuiCol_Border);
					}

					ImGui::PopID();
				}
			}

			ImGui::EndTable();
		}
	}
}