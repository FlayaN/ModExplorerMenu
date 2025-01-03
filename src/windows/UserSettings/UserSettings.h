#pragma once

namespace Modex
{
	class SettingsWindow
	{
	public:
		static void Draw();
		static void DrawGeneralSettings();
		static void DrawThemeSelector();
		static void DrawTeleportSettings();
		static void DrawAddItemSettings();
		static void DrawPopped();
		static void Init();

		static inline std::atomic_bool changes = false;
		static inline std::atomic_bool file_changes = false;
		static inline char savePath[MAX_PATH] = { 0 };
	};
}