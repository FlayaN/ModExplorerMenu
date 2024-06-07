#pragma once

#include <PCH.h>

class Frame
{
public:
	static void Draw(bool is_settings_popped = false);
	static void Install();
	static void RefreshStyle();

	static inline std::atomic_bool _init = false;

	static inline ImFont* text_font;
	static inline ImFont* header_font;
	static inline ImFont* sidebar_font;
	static inline ImFont* button_font;

	enum ActiveWindow
	{
		Home,
		AddItem,
		Lookup,
		NPC,
		Teleport,
		Settings
	};

	static ActiveWindow _activeWindow;
};