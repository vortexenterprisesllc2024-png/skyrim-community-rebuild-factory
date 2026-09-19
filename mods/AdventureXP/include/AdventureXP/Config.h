#pragma once

#include "AdventureXP/Presets.h"
#include "AdventureXP/Types.h"

#include <array>
#include <filesystem>
#include <string>

namespace AdventureXP {

class Config {
public:
	static Config& Get();

	void Load(const std::filesystem::path& path);
	void Save() const;

	[[nodiscard]] const std::filesystem::path& Path() const { return path_; }

	bool enabled = true;
	float globalXPPercent = 100.f;
	bool showMessages = true;
	bool ignoreSkillLeveling = true;
	bool awardKilling = false;
	bool awardReading = false;
	std::string preset = "Adventurer";

	std::array<float, 7> categoryWeight = kDefaultCategoryWeight;

	bool awardQuestStages = true;
	bool awardQuestComplete = true;
	bool skipHiddenQuests = true;
	bool skipMiscQuests = false;
	float mainQuestMultiplier = 1.0f;

	std::array<int, 13> questXP = kDefaultQuestXP;
	std::array<int, 39> discoveryXP = kDefaultDiscoveryXP;
	std::array<int, 39> clearXP = kDefaultClearXP;
	std::array<float, 18> skillWeight = kDefaultSkillWeight;

	float killXP = 2.f;
	float bossKillXP = 25.f;
	float readingXP = 5.f;
	float craftingXP = 8.f;
	float skillUpXP = 4.f;

	float undeadCombatBonus = 0.f;
	float stealthCombatBonus = 0.f;
	float beastCombatBonus = 0.f;

	float xpBase = 200.f;
	float xpPerLevel = 50.f;
	float xpExponent = 1.f;
	int maxLevel = 81;

	[[nodiscard]] float CategoryWeight(Category category) const;
	void SetCategoryWeight(Category category, float weight);

	[[nodiscard]] int QuestXP(QuestKind kind) const;
	void SetQuestXP(QuestKind kind, int amount);

	[[nodiscard]] int DiscoveryXP(PlaceKind kind) const;
	void SetDiscoveryXP(PlaceKind kind, int amount);

	[[nodiscard]] int ClearXP(PlaceKind kind) const;
	void SetClearXP(PlaceKind kind, int amount);

	[[nodiscard]] float SkillWeight(SkillKind kind) const;
	void SetSkillWeight(SkillKind kind, float weight);

	void ApplyPack(const PlayStylePack& pack);
	bool ApplyPreset(std::string_view id);

private:
	std::filesystem::path path_;
};

}  // namespace AdventureXP
