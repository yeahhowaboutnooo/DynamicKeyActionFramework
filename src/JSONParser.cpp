#include "JSONParser.h"
#include "ActionManager.h"
#include "Utils.h"

using namespace RE;

void JSONParser::LoadJSONFiles() {
	namespace fs = std::filesystem;
	fs::path jsonPath = fs::current_path();
	jsonPath += "\\Data\\SKSE\\Plugins\\DKAF";
	std::stringstream stream;
	fs::directory_entry jsonEntry{ jsonPath };
	if (jsonEntry.exists()) {
		for (auto& it : fs::directory_iterator(jsonEntry)) {
			if (it.path().extension().compare(".json") == 0) {
				stream << it.path().filename();
				logger::info(FMT_STRING("Loading key-action data {}"), stream.str().c_str());
				stream.str(std::string());
				ParseKeyActionJSON(it.path());
			}
		}
	}
}

void JSONParser::ParseKeyActionJSON(const std::filesystem::path path) {
	std::ifstream reader;
	reader.open(path);
	nlohmann::json keyActionData;
	reader >> keyActionData;
	for (auto actionIt = keyActionData.begin(); actionIt != keyActionData.end(); ++actionIt) {
		logger::info("Action: {}", actionIt.key().c_str());
		Action action;
		auto lookup = (*actionIt).find("Keys");
		if (lookup != (*actionIt).end()) {
			std::string keys = lookup.value().get<std::string>();
			logger::info("Keys: {}", keys.c_str());
			std::string keyStr = Utils::SplitString(keys, ",", keys);
			while (keyStr.length() > 0) {
				action.keys.push_back(std::stoi(keyStr));
				logger::info("\tAdded {}", std::stoi(keyStr));
				keyStr = Utils::SplitString(keys, ",", keys);
			}
		}
		lookup = (*actionIt).find("GamepadKeys");
		if (lookup != (*actionIt).end()) {
			std::string keys = lookup.value().get<std::string>();
			logger::info("GamepadKeys: {}", keys.c_str());
			std::string keyStr = Utils::SplitString(keys, ",", keys);
			while (keyStr.length() > 0) {
				action.gamepadKeys.push_back(std::stoi(keyStr));
				logger::info("\tAdded {}", std::stoi(keyStr));
				keyStr = Utils::SplitString(keys, ",", keys);
			}
		}
		action.pairedSearchMode = Action::SEARCH_MODE::kNone;
		lookup = (*actionIt).find("PairedSearchMode");
		if (lookup != (*actionIt).end()) {
			action.pairedSearchMode = (Action::SEARCH_MODE)lookup.value().get<int>();
		}
		action.pairedSearchDist = 0;
		lookup = (*actionIt).find("PairedSearchDist");
		if (lookup != (*actionIt).end()) {
			action.pairedSearchDist = lookup.value().get<float>();
			if (action.pairedSearchDist <= 0) {
				logger::warn("Invalid value for PairedSearchDist. Search disabled.");
				action.pairedSearchMode = Action::SEARCH_MODE::kNone;
			}
		}
		lookup = (*actionIt).find("PairedTargetKeywords");
		if (lookup != (*actionIt).end()) {
			std::string keywords = lookup.value().get<std::string>();
			logger::info("PairedTargetKeywords: {}", keywords.c_str());
			std::string keywordStr = Utils::SplitString(keywords, ",", keywords);
			while (keywordStr.length() > 0) {
				TESForm* form = Utils::GetFormFromConfigString(keywordStr);
				if (form && form->formType.get() == FormType::Keyword) {
					action.pairedTargetKeywords.push_back(reinterpret_cast<BGSKeyword*>(form));
					logger::info("\tAdded {}", form->GetFormEditorID());
				}
				keywordStr = Utils::SplitString(keywords, ",", keywords);
			}
		}
		lookup = (*actionIt).find("ActionOrIdle");
		if (lookup != (*actionIt).end()) {
			TESForm* form = Utils::GetFormFromConfigString(lookup.value().get<std::string>());
			if (form && (form->formType.get() == FormType::Idle || form->formType.get() == FormType::Action)) {
				logger::info("ActionOrIdle: {}", form->GetFormEditorID());
				action.actionOrIdle = form;
			}
		}
		else {
			logger::warn("ActionOrIdle not found in the data.");
		}
		action.actionAssocIdle = DEFAULT_OBJECT::kActionIdle;
		lookup = (*actionIt).find("ActionAssociatedWithIdle");
		if (lookup != (*actionIt).end()) {
			int defObj = lookup.value().get<int>();
			if (defObj >= 0 && defObj < DEFAULT_OBJECT::kTotal) {
				action.actionAssocIdle = (DEFAULT_OBJECT)defObj;
				logger::info("ActionAssociatedWithIdle: {}", defObj);
			}
			else {
				logger::warn("ActionAssociatedWithIdle out of range! Defaulting to ActionIdle");
			}
		}
		action.pressDuration = 0;
		lookup = (*actionIt).find("PressDuration");
		if (lookup != (*actionIt).end()) {
			action.pressDuration = lookup.value().get<float>();
		}
		logger::info("PressDuration: {}", action.pressDuration);
		action.priority = 0;
		lookup = (*actionIt).find("Priority");
		if (lookup != (*actionIt).end()) {
			action.priority = lookup.value().get<int>();
		}
		logger::info("Priority: {}", action.priority);
		action.triggersOnRelease = false;
		lookup = (*actionIt).find("triggersOnRelease");
		if (lookup != (*actionIt).end()) {
			action.triggersOnRelease = lookup.value().get<bool>();
		}
		logger::info("triggersOnRelease: {}", action.triggersOnRelease);
		if (ActionManager::GetSingleton()->AddAction(action)) {
			logger::info("Action successfully added");
		}
		else {
			logger::info("Action not added.");
		}
	}
	reader.close();
}
