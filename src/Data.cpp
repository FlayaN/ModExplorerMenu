#include "Data.h"
#include "lib/Worldspace.h"

using _GetFormEditorID = const char* (*)(std::uint32_t);
std::string getFormEditorID(const RE::TESForm* a_form)
{
	switch (a_form->GetFormType()) {
	case RE::FormType::Keyword:
	case RE::FormType::LocationRefType:
	case RE::FormType::Action:
	case RE::FormType::MenuIcon:
	case RE::FormType::Global:
	case RE::FormType::HeadPart:
	case RE::FormType::Race:
	case RE::FormType::Sound:
	case RE::FormType::Script:
	case RE::FormType::Navigation:
	case RE::FormType::Cell:
	case RE::FormType::WorldSpace:
	case RE::FormType::Land:
	case RE::FormType::NavMesh:
	case RE::FormType::Dialogue:
	case RE::FormType::Quest:
	case RE::FormType::Idle:
	case RE::FormType::AnimatedObject:
	case RE::FormType::ImageAdapter:
	case RE::FormType::VoiceType:
	case RE::FormType::Ragdoll:
	case RE::FormType::DefaultObject:
	case RE::FormType::MusicType:
	case RE::FormType::StoryManagerBranchNode:
	case RE::FormType::StoryManagerQuestNode:
	case RE::FormType::StoryManagerEventNode:
	case RE::FormType::SoundRecord:
		return a_form->GetFormEditorID();
	default:
		{
			static auto tweaks = GetModuleHandleA("po3_Tweaks");
			static auto func = reinterpret_cast<_GetFormEditorID>(GetProcAddress(tweaks, "GetFormEditorID"));
			if (func) {
				return func(a_form->formID);
			}
			return {};
		}
	}
}

template <class T>
void MEMData::CacheItems(RE::TESDataHandler* a_data)
{
	for (auto form : a_data->GetFormArray<T>()) {
		const char* name = form->GetFullName();
		RE::FormID _formid = form->GetFormID();
		std::string formid = fmt::format("{:08x}", _formid);
		std::string editorid = getFormEditorID(form);
		RE::FormType formType = form->GetFormType();
		std::string typeName = static_cast<std::string>(RE::FormTypeToString(formType));

		std::int32_t goldValue = 0;
		bool non_playable = false;

		//logger::info("FormType: {}", typeName);

		if (formType == RE::FormType::NPC) {
			auto npc = form->As<RE::TESNPC>();
			if (npc->IsPlayerRef()) {
				continue;
			}

			// npc specific stuff
			//logger::info("NPC: {}", form->GetFullName());
		} else if (formType == RE::FormType::WorldSpace) {
			// GetFullName is the worldspace not cell.
			//logger::info("CELL: {}", form->GetFullName());
		} else {
			goldValue = form->GetGoldValue();

			if (formType == RE::FormType::Weapon) {
				non_playable = form->As<RE::TESObjectWEAP>()->weaponData.flags.any(RE::TESObjectWEAP::Data::Flag::kNonPlayable);
			}
		}

		RE::TESFile* mod = form->GetFile();

		_cache.push_back({ name, formid, form, editorid, formType, typeName, goldValue, mod, non_playable });

		//Add mod file to list.
		if (!_modList.contains(mod)) {
			_modList.insert(mod);
			// .first->GetFilename().data()
		}

		// write a logger::info() here to detail the size of _cache in memory and the amount of items added
	}
}

// https://github.com/shad0wshayd3-TES5/BakaHelpExtender | License : MIT
// Absolute unit of code here. Super grateful for the author.
void MEMData::CacheCells(RE::TESFile* a_file, std::vector<CachedCell>& a_cellMap)
{
	if (!a_file->OpenTES(RE::NiFile::OpenMode::kReadOnly, false)) {
		logger::warn(FMT_STRING("failed to open file: {:s}"sv), a_file->fileName);
		return;
	}

	do {
		if (a_file->currentform.form == 'LLEC') {
			char edid[512]{ '\0' };
			bool gotEDID{ false };

			std::uint16_t data{ 0 };
			bool gotDATA{ false };

			std::uint32_t cidx{ 0 };
			cidx += a_file->compileIndex << 24;
			cidx += a_file->smallFileCompileIndex << 12;

			do {
				switch (a_file->GetCurrentSubRecordType()) {
				case 'DIDE':
					gotEDID = a_file->ReadData(edid, a_file->actualChunkSize);
					if (gotEDID && gotDATA && ((data & 1) == 0)) {
						//a_map.insert_or_assign(edid, std::make_pair(cidx, a_file->fileName));
						//a_cellMap.insert_or_assign(edid, CachedCell(a_file->fileName, "Unkown", "Unkown", "Unkown", edid, a_file));
						a_cellMap.push_back(CachedCell(a_file->fileName, "Unkown", "Unkown", "Unkown", edid, a_file));
						continue;
					}
					break;

				case 'ATAD':
					gotDATA = a_file->ReadData(&data, a_file->actualChunkSize);
					if (gotEDID && gotDATA && ((data & 1) == 0)) {
						//a_cellMap.insert_or_assign(edid, CachedCell(a_file->fileName, "Unkown", "Unkown", "Unkown", edid, a_file));
						//a_map.insert_or_assign(edid, std::make_pair(cidx, a_file->fileName));
						a_cellMap.push_back(CachedCell(a_file->fileName, "Unkown", "Unkown", "Unkown", edid, a_file));
						continue;
					}
					break;

				default:
					break;
				}
			} while (a_file->SeekNextSubrecord());
		}
	} while (a_file->SeekNextForm(true));

	if (!a_file->CloseTES(false)) {
		logger::warn(FMT_STRING("failed to close file: {:s}"sv), a_file->fileName);
	}
}

void MEMData::Run()
{
	RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

	if (!dataHandler) {
		logger::error("Failed to get data handler.");
		return;
	}

	// AddItem Specific Forms
	CacheItems<RE::TESObjectARMO>(dataHandler);
	CacheItems<RE::TESObjectBOOK>(dataHandler);
	CacheItems<RE::TESObjectWEAP>(dataHandler);
	CacheItems<RE::TESObjectMISC>(dataHandler);
	CacheItems<RE::TESAmmo>(dataHandler);
	CacheItems<RE::AlchemyItem>(dataHandler);
	CacheItems<RE::IngredientItem>(dataHandler);

	// NPC -- TODO: Implement
	CacheItems<RE::TESNPC>(dataHandler);

	// Cells -- TODO:: Implement
	WorldspaceCells cells;

	for (const auto& cell : cells.table) {
		const auto& [_plugin, space, place, name, editorid] = cell;
		std::string plugin = _plugin + ".esm";
		const RE::TESFile* mod = dataHandler->LookupModByName(plugin.c_str());

		_cellCache.push_back(CachedCell(plugin, space, place, name, editorid, mod));
	}

	// Overwrite _cellCache with Baka changes
	for (const auto& file : _modList) {
		CacheCells(file, _cellCache);
	}

	// for (const auto& file : _modList) {
	// 	CacheCells(file, _testMap);
	// }

	// for (const auto& cell : _testMap) {
	// 	const auto& [editorid, pair] = cell;
	// 	const auto& [cidx, fileName] = pair;

	// 	auto _cellPair = std::make_pair(editorid, cidx);
	// 	std::string _fileName = std::string(fileName);

	// 	auto& spaceMap = pluginMap.emplace(_fileName, std::multimap<std::string, std::multimap<std::string, std::pair<std::string, uint32_t>>>())->second;
	// 	auto& zoneMap = spaceMap.emplace("Unkown", std::multimap<std::string, std::pair<std::string, uint32_t>>())->second;
	// 	zoneMap.emplace("Unkown", _cellPair);
	// }

	// for (const auto& pluginPair : pluginMap) {
	// 	const auto& [plugin, spaceMap] = pluginPair;

	// 	for (const auto& spacePair : spaceMap) {
	// 		const auto& [space, zoneMap] = spacePair;

	// 		for (const auto& zonePair : zoneMap) {
	// 			const auto& [zone, cellPair] = zonePair;
	// 			const auto& [cell, cidx] = cellPair;

	// 			logger::info("Plugin: {}, Space: {}, Zone: {}, Cell: {}", plugin, space, zone, cell);
	// 		}
	// 	}
	// }
}