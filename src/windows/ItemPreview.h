#pragma once
#include "Objects.h"

namespace Modex
{
	template <class Object>
	void ShowItemPreview(Object* a_object)
	{
		if (a_object == nullptr) {
			return;
		}

		ImVec2 barSize = ImVec2(100.0f, ImGui::GetFontSize());
		float maxWidth = ImGui::GetContentRegionAvail().x;

		constexpr auto ProgressColor = [](const double value, const float max_value) -> ImVec4 {
			const float dampen = 0.7f;
			const float ratio = (float)value / max_value;
			const float r = 1.0f - ratio * dampen;
			const float g = ratio * dampen;
			return ImVec4(r, g, 0.0f, 1.0f);
		};

		const auto InlineBar = [maxWidth, barSize, ProgressColor](const char* label, const float value, const float max_value) {
			ImGui::Text(label);
			ImGui::SameLine(maxWidth - barSize.x);
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ProgressColor(value, max_value));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);  // tight fit
			float curr = static_cast<float>(value);
			char buffer[256];
			sprintf(buffer, "%.2f", value);
			ImGui::ProgressBar(curr / max_value, barSize, buffer);
			ImGui::PopStyleColor(1);
			ImGui::PopStyleVar(1);
		};

		const auto InlineInt = [maxWidth, barSize](const char* label, const int value) {
			const auto defaultWidth = maxWidth - ImGui::CalcTextSize(std::to_string(value).c_str()).x;
			const auto width = std::max(defaultWidth, ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string(value).c_str()).x);
			ImGui::Text(label);
			ImGui::SameLine(width);
			ImGui::Text("%d", value);
		};

		const auto InlineText = [maxWidth](const char* label, const char* text) {
			const auto width = std::max(maxWidth - ImGui::CalcTextSize(text).x, ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(text).x);
			ImGui::Text(label);
			ImGui::SameLine(width);
			ImGui::Text(text);
		};

		const auto TruncateText = [](const char* text, const float width) -> std::string {
			if (ImGui::CalcTextSize(text).x > width) {
				std::string truncated = text;
				truncated.resize((size_t)(width / ImGui::CalcTextSize("A").x) - 3);
				truncated += "...";
				return truncated;
			}
			return std::string(text);
		};

		// Name Bar
		const auto name = a_object->GetName();

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		const auto cursor = ImGui::GetCursorScreenPos();
		const auto size = ImGui::GetContentRegionAvail();
		const auto color = ImGui::GetStyleColorVec4(ImGuiCol_Border);
		draw_list->AddRectFilled(cursor, ImVec2(cursor.x + size.x, cursor.y + ImGui::GetFontSize() * 2.5f), ImGui::ColorConvertFloat4ToU32(ImVec4(0.15f, 0.15f, 0.15f, 0.5f)));
		draw_list->AddRect(cursor, ImVec2(cursor.x + size.x, cursor.y + ImGui::GetFontSize() * 2.5f), ImGui::ColorConvertFloat4ToU32(color));

		ImGui::NewLine();
		ImGui::SetCursorPosX(ImGui::GetCenterTextPosX(name.data()));
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetFontSize() / 2);
		ImGui::Text(name.data());
		ImGui::NewLine();

		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
		ImGui::SetCursorPosX(ImGui::GetCenterTextPosX(_T("Stats")));
		ImGui::Text(_T("Stats"));
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

		// NPC Specific Item Card details:
		if constexpr (std::is_same<Object, NPC>::value) {
			auto* npc = a_object->TESForm->As<RE::TESNPC>();

			if (npc == nullptr) {
				return;
			}

			InlineBar(_T("Health") + ":", a_object->GetHealth(), 100);
			InlineBar(_T("Magicka") + ":", a_object->GetMagicka(), 100);
			InlineBar(_T("Stamina") + ":", a_object->GetStamina(), 100);
			InlineBar(_T("Weight") + ":", a_object->GetCarryWeight(), 100);

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
		}

		// constexpr allows for compile-time evaluation of struct passed
		// allowing for member access to be determined by control paths.
		if constexpr (std::is_same<Object, Item>::value) {
			if (a_object->GetFormType() == RE::FormType::Armor) {
				auto* armor = a_object->TESForm->As<RE::TESObjectARMO>();

				if (armor == nullptr) {
					return;
				}

				const auto armorType = Utils::GetArmorType(armor);
				const float armorRating = Utils::CalcBaseArmorRating(armor);
				const float armorRatingMax = Utils::CalcMaxArmorRating(armorRating, 50);
				const auto equipSlot = Utils::GetArmorSlot(armor);

				if (armorRating == 0) {
					// InlineText("Armor Rating:", "None");
					InlineText(_TICONM(ICON_RPG_ARMOR, "Rating", ":"), _T("None"));
				} else {
					// InlineBar("Armor Rating:", armorRating, armorRatingMax);
					InlineBar(_TICONM(ICON_RPG_ARMOR, "Rating", ":"), armorRating, armorRatingMax);
				}

				InlineText(_TICONM(ICON_RPG_ARMOR, "Type", ":"), _T(armorType));
				InlineText(_TICONM(ICON_RPG_ARMOR, "Slot", ":"), _T(equipSlot));
			}

			if (a_object->GetFormType() == RE::FormType::Weapon) {
				auto* weapon = a_object->TESForm->As<RE::TESObjectWEAP>();

				if (weapon == nullptr) {
					return;
				}

				const char* weaponTypes[] = {
					"Hand to Hand",
					"One Handed Sword",
					"One Handed Dagger",
					"One Handed Axe",
					"One Handed Mace",
					"Two Handed Greatsword",
					"Two Handed Battleaxe",
					"Bow",
					"Staff",
					"Crossbow"
				};

				const float damage = Utils::CalcBaseDamage(weapon);
				const float max_damage = Utils::CalcMaxDamage(damage, 50);
				const float speed = weapon->weaponData.speed;
				const int dps = (int)(damage * speed);
				const uint16_t critDamage = weapon->GetCritDamage();
				const RE::ActorValue skill = weapon->weaponData.skill.get();
				const auto type = weaponTypes[static_cast<int>(weapon->GetWeaponType())];

				if (weapon->IsStaff()) {
					// InlineText("Base Damage:", "N/A");
					InlineText(ICON_RPG_ATTACK, "N/A");
				} else if (weapon->IsBow() || weapon->IsCrossbow()) {
					InlineBar(_TICONM(ICON_RPG_ATTACK, "DMG", ":"), damage, max_damage);
					InlineBar(_TICONM(ICON_RPG_ATTACK, "Speed", ":"), speed, 1.5f);
					InlineInt(_TICONM(ICON_RPG_ATTACK, "DPS", ":"), dps);
					ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
					InlineInt(_TICONM(ICON_RPG_ATTACK, "DMG", ":"), critDamage);
					InlineText(_TICONM(ICON_RPG_SKILL, "Skill", ":"), _T(std::to_string(skill).c_str()));
				} else {
					const float reach = (float)(weapon->weaponData.reach);
					const float stagger = weapon->weaponData.staggerValue;
					InlineBar(_TICONM(ICON_RPG_ATTACK, "DMG", ":"), damage, max_damage);
					InlineBar(_TICONM(ICON_RPG_ATTACK, "Speed", ":"), speed, 1.5f);
					InlineInt(_TICONM(ICON_RPG_ATTACK, "DPS", ":"), dps);
					ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
					InlineInt(_TICONM(ICON_RPG_ATTACK, "DMG", ":"), critDamage);
					InlineText(_TICONM(ICON_RPG_SKILL, "Skill", ":"), _T(std::to_string(skill).c_str()));
					InlineBar(_TICONM(ICON_RPG_WEAPON, "Reach", ":"), reach, 1.5f);
					InlineBar(_TICONM(ICON_RPG_WEAPON, "Stagger", ":"), stagger, 2.0f);
				}

				ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
				InlineText(_TICONM(ICON_RPG_WEAPON_TYPE, "Type", ":"), _T(type));
			}

			InlineInt(ICON_RPG_WEIGHT " WT:", (int)a_object->GetWeight());
			InlineInt(_TICONM(ICON_RPG_VALUE, "Value", ":"), a_object->GetValue());

			const std::string desc = Utils::GetItemDescription(a_object->TESForm, g_DescriptionFrameworkInterface);
			if (!desc.empty()) {
				ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
				ImGui::SetCursorPosX(ImGui::GetCenterTextPosX("Description"));
				ImGui::Text(_T("Description"));
				ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
				ImGui::PushTextWrapPos(maxWidth);
				ImGui::TextWrapped(desc.c_str());
				ImGui::PopTextWrapPos();
			}

			if (a_object->GetFormType() == RE::FormType::Book) {
				ImGui::SetCursorPosX(ImGui::GetCenterTextPosX(desc.c_str()));  // Read Me!
			}
		}
	}
}