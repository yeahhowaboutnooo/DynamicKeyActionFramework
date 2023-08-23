#pragma once

#include <unordered_map>
#include <vector>

struct Action {
public:
	enum SEARCH_MODE {
		kNone = 0,
		kRayFromActor = 1,
		kRayFromCamera = 2,
		kRadius = 3
	};
	std::vector<uint32_t> keys;
	std::vector<uint32_t> gamepadKeys;
	SEARCH_MODE pairedSearchMode;
	float pairedSearchDist;
	std::vector<RE::BGSKeyword*> pairedTargetKeywords;
	RE::TESForm* actionOrIdle;
	RE::DEFAULT_OBJECT actionAssocIdle;
	float pressDuration;
	int priority;
	bool triggersOnRelease;

	bool operator==(const Action& other) {
		if (this->keys == other.keys
			&& this->gamepadKeys == other.gamepadKeys
			&& this->pairedSearchMode == other.pairedSearchMode
			&& this->pairedSearchDist == other.pairedSearchDist
			&& this->pairedTargetKeywords == other.pairedTargetKeywords
			&& this->actionOrIdle == other.actionOrIdle
			&& this->pressDuration == other.pressDuration
			&& this->priority == other.priority
			&& this->triggersOnRelease == other.triggersOnRelease)
			return true;
		return false;
	}
};

class ActionManager {
protected:
	static ActionManager* instance;
	std::vector<Action> actions;
	std::unordered_map<uint32_t, float> pressedKeys;
	std::unordered_map<uint32_t, float> unfilteredPressedKeys;
	std::unordered_set<uint32_t> filterKeys;
	bool actionQueued = false;
public:
	ActionManager() = default;
	ActionManager(ActionManager&) = delete;
	void operator=(const ActionManager&) = delete;
	static ActionManager* GetSingleton() {
		if (!instance)
			instance = new ActionManager();
		return instance;
	}
	bool AddAction(Action action) {
		if ((action.keys.size() == 0
			 && action.gamepadKeys.size() == 0)
			|| !action.actionOrIdle)
			return false;
		for (auto actionIt = actions.begin(); actionIt != actions.end(); ++actionIt) {
			if (actionIt->operator==(action)) {
				return false;
			}
		}
		bool inserted = false;
		for (auto actionIt = actions.begin(); actionIt != actions.end(); ++actionIt) {
			if (action.priority >= actionIt->priority) {
				actions.insert(actionIt, action);
				inserted = true;
				break;
			}
		}
		if (!inserted)
			actions.push_back(action);
		return true;
	}
	void ClearActions() {
		actions.clear();
	}
	void EvaluateAndPerformActions(bool triggersOnRelease = false);

	void OnKeyPressed(uint32_t keyCode, float duration) {
		auto found = unfilteredPressedKeys.find(keyCode);
		if (found != unfilteredPressedKeys.end()) {
			found->second = duration;
		}
		else {
			unfilteredPressedKeys.insert(std::pair<uint32_t, float>(keyCode, duration));
			//logger::debug("Key pressed unfiltered {}", keyCode);
		}
		auto filter = filterKeys.find(keyCode);
		if (filter != filterKeys.end())
			return;
		found = pressedKeys.find(keyCode);
		if (found != pressedKeys.end()) {
			found->second = duration;
		}
		else {
			pressedKeys.insert(std::pair<uint32_t, float>(keyCode, duration));
			//logger::debug("Key pressed {}", keyCode);
		}
	}
	void OnKeyReleased(uint32_t keyCode) {
		RemoveFilterKey(keyCode);
		auto found = pressedKeys.find(keyCode);
		if (found != pressedKeys.end()) {
			pressedKeys.erase(found);
			//logger::debug("Key released {}", keyCode);
		}
		found = unfilteredPressedKeys.find(keyCode);
		if (found != unfilteredPressedKeys.end()) {
			unfilteredPressedKeys.erase(found);
			//logger::debug("Key released unfiltered {}", keyCode);
		}
	}
	void OnPostPressAction(uint32_t keyCode) {
		auto found = pressedKeys.find(keyCode);
		if (found != pressedKeys.end()) {
			pressedKeys.erase(found);
			//logger::debug("Key triggered Press Action {}", keyCode);
		}
		AddFilterKey(keyCode);
	}
	void ClearPressedKeys() {
		unfilteredPressedKeys.clear();
		pressedKeys.clear();
	}
	void ClearFilterKeys() {
		filterKeys.clear();
	}
	void AddFilterKey(uint32_t keyCode) {
		filterKeys.insert(keyCode);
	}
	void RemoveFilterKey(uint32_t keyCode) {
		auto filter = filterKeys.find(keyCode);
		if (filter != filterKeys.end()) {
			filterKeys.erase(filter);
		}
	}

	void SetActionQueued(bool b) {
		actionQueued = b;
	}
};
