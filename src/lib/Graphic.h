#pragma once

#include "d3d11.h"

// Sourced from dTry's Wheeler and Llama's Tiny Hud. Adapted to NG.
// Huge gratitude for their public works.

class GraphicManager
{
private:
public:
	struct Image
	{
		ID3D11ShaderResourceView* texture = nullptr;
		int32_t width = 0;
		int32_t height = 0;
	};

	static inline std::map<std::string, GraphicManager::Image> image_library;
	static inline std::map<std::string, ImFont*> font_library;

	static void DrawImage(Image& a_texture, ImVec2 a_center);

	static bool GetD3D11Texture(const char* filename, ID3D11ShaderResourceView** out_srv, int& out_width,
		int& out_height);

	static void LoadImagesFromFilepath(std::string a_path, std::map<std::string, Image>& out_struct);

	static void LoadFontsFromDirectory(std::string a_path, std::map<std::string, ImFont*>& out_struct);

	static void Init();

	static inline std::atomic_bool initialized = false;

	[[nodiscard]] static ImFont* GetFont(std::string a_font)
	{
		auto found = font_library.find(a_font);
		if (found != font_library.end()) {
			return found->second;
		}

		logger::warn("Font Reference not found: {}", a_font);
		return ImGui::GetFont();
	}

	[[nodiscard]] static std::string GetFontName(ImFont* a_font)
	{
		// Note to self: return font_library[key] adds key value if not valid.
		for (const auto& [key, value] : font_library) {
			if (value == a_font) {
				return key;
			}
		}

		return "Default";
	}

	[[nodiscard]] static Image GetImage(std::string a_name)
	{
		auto found = image_library.find(a_name);
		if (found != image_library.end()) {
			return found->second;
		}

		logger::warn("Image Reference not found: {}", a_name);
		return Image();
	}

	[[nodiscard]] static std::string GetImageName(Image a_image)
	{
		// Note to self: return image_library[key] adds key value if not valid.
		for (const auto& [key, value] : image_library) {
			if (value.texture == a_image.texture) {
				return key;
			}
		}

		logger::warn("Image name not found");
		return "None";
	}
};