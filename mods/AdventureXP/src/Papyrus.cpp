#include "AdventureXP/Papyrus.h"

#include "AdventureXP/Config.h"
#include "AdventureXP/Types.h"
#include "AdventureXP/Version.h"

#ifdef ADVENTUREXP_WITH_GAME
#	include <RE/Skyrim.h>
#endif

namespace AdventureXP::Papyrus {
namespace {

#ifdef ADVENTUREXP_WITH_GAME

std::string GetVersion(RE::StaticFunctionTag*)
{
	return ADVENTUREXP_VERSION_STRING;
}

int GetPresetCount(RE::StaticFunctionTag*)
{
	return static_cast<int>(kPresets.size());
}

std::string GetPresetId(RE::StaticFunctionTag*, int index)
{
	if (index < 0 || index >= static_cast<int>(kPresets.size())) {
		return "";
	}
	return kPresets[static_cast<std::size_t>(index)].id;
}

std::string GetPresetName(RE::StaticFunctionTag*, int index)
{
	if (index < 0 || index >= static_cast<int>(kPresets.size())) {
		return "";
	}
	return kPresets[static_cast<std::size_t>(index)].name;
}

std::string GetActivePreset(RE::StaticFunctionTag*)
{
	return Config::Get().preset;
}

bool ApplyPreset(RE::StaticFunctionTag*, RE::BSFixedString id)
{
	return Config::Get().ApplyPreset(id.c_str());
}

float GetCategoryWeight(RE::StaticFunctionTag*, RE::BSFixedString category)
{
	if (const auto parsed = ParseCategory(category.c_str())) {
		return Config::Get().CategoryWeight(*parsed);
	}
	return 0.f;
}

void SetCategoryWeight(RE::StaticFunctionTag*, RE::BSFixedString category, float weight)
{
	if (const auto parsed = ParseCategory(category.c_str())) {
		Config::Get().SetCategoryWeight(*parsed, weight);
	}
}

float GetGlobalXPPercent(RE::StaticFunctionTag*)
{
	return Config::Get().globalXPPercent;
}

void SetGlobalXPPercent(RE::StaticFunctionTag*, float percent)
{
	Config::Get().globalXPPercent = ClampPercent(percent);
	Config::Get().Save();
}

bool GetEnabled(RE::StaticFunctionTag*)
{
	return Config::Get().enabled;
}

void SetEnabled(RE::StaticFunctionTag*, bool enabled)
{
	Config::Get().enabled = enabled;
	Config::Get().Save();
}

bool GetShowMessages(RE::StaticFunctionTag*)
{
	return Config::Get().showMessages;
}

void SetShowMessages(RE::StaticFunctionTag*, bool show)
{
	Config::Get().showMessages = show;
	Config::Get().Save();
}

bool GetAwardKilling(RE::StaticFunctionTag*)
{
	return Config::Get().awardKilling;
}

void SetAwardKilling(RE::StaticFunctionTag*, bool enabled)
{
	Config::Get().awardKilling = enabled;
	Config::Get().Save();
}

bool GetAwardReading(RE::StaticFunctionTag*)
{
	return Config::Get().awardReading;
}

void SetAwardReading(RE::StaticFunctionTag*, bool enabled)
{
	Config::Get().awardReading = enabled;
	Config::Get().Save();
}

int GetQuestXP(RE::StaticFunctionTag*, RE::BSFixedString name)
{
	if (const auto kind = ParseQuestKind(name.c_str())) {
		return Config::Get().QuestXP(*kind);
	}
	return 0;
}

void SetQuestXP(RE::StaticFunctionTag*, RE::BSFixedString name, int amount)
{
	if (const auto kind = ParseQuestKind(name.c_str())) {
		Config::Get().SetQuestXP(*kind, amount);
	}
}

int GetDiscoveryXP(RE::StaticFunctionTag*, RE::BSFixedString name)
{
	if (const auto kind = ParsePlaceKind(name.c_str())) {
		return Config::Get().DiscoveryXP(*kind);
	}
	return 0;
}

void SetDiscoveryXP(RE::StaticFunctionTag*, RE::BSFixedString name, int amount)
{
	if (const auto kind = ParsePlaceKind(name.c_str())) {
		Config::Get().SetDiscoveryXP(*kind, amount);
	}
}

int GetClearXP(RE::StaticFunctionTag*, RE::BSFixedString name)
{
	if (const auto kind = ParsePlaceKind(name.c_str())) {
		return Config::Get().ClearXP(*kind);
	}
	return 0;
}

void SetClearXP(RE::StaticFunctionTag*, RE::BSFixedString name, int amount)
{
	if (const auto kind = ParsePlaceKind(name.c_str())) {
		Config::Get().SetClearXP(*kind, amount);
	}
}

// Legacy percent helpers: still 0–100 so older scripts keep compiling.
float GetQuestTypeWeight(RE::StaticFunctionTag*, RE::BSFixedString name)
{
	if (EqI(name.c_str(), "Guild")) {
		const auto& cfg = Config::Get();
		const float sum = static_cast<float>(
			cfg.QuestXP(QuestKind::College) + cfg.QuestXP(QuestKind::Thieves) +
			cfg.QuestXP(QuestKind::Brotherhood) + cfg.QuestXP(QuestKind::Companions));
		return sum / 4.f;
	}
	if (EqI(name.c_str(), "DLC")) {
		const auto& cfg = Config::Get();
		return static_cast<float>(cfg.QuestXP(QuestKind::Dawnguard) + cfg.QuestXP(QuestKind::Dragonborn)) / 2.f;
	}
	if (const auto kind = ParseQuestKind(name.c_str())) {
		return static_cast<float>(Config::Get().QuestXP(*kind));
	}
	return 0.f;
}

void SetQuestTypeWeight(RE::StaticFunctionTag*, RE::BSFixedString name, float weight)
{
	const int amount = ClampInt(static_cast<int>(weight), kQuestXPMin, kQuestXPMax);
	if (EqI(name.c_str(), "Guild")) {
		Config::Get().SetQuestXP(QuestKind::College, amount);
		Config::Get().SetQuestXP(QuestKind::Thieves, amount);
		Config::Get().SetQuestXP(QuestKind::Brotherhood, amount);
		Config::Get().SetQuestXP(QuestKind::Companions, amount);
		return;
	}
	if (EqI(name.c_str(), "DLC")) {
		Config::Get().SetQuestXP(QuestKind::Dawnguard, amount);
		Config::Get().SetQuestXP(QuestKind::Dragonborn, amount);
		return;
	}
	if (const auto kind = ParseQuestKind(name.c_str())) {
		Config::Get().SetQuestXP(*kind, amount);
	}
}

float GetPlaceTypeWeight(RE::StaticFunctionTag*, RE::BSFixedString name)
{
	if (const auto kind = ParsePlaceKind(name.c_str())) {
		return static_cast<float>(Config::Get().DiscoveryXP(*kind));
	}
	return 0.f;
}

void SetPlaceTypeWeight(RE::StaticFunctionTag*, RE::BSFixedString name, float weight)
{
	if (const auto kind = ParsePlaceKind(name.c_str())) {
		Config::Get().SetDiscoveryXP(*kind, ClampInt(static_cast<int>(weight), kPlaceXPMin, kPlaceXPMax));
	}
}

float GetSkillWeight(RE::StaticFunctionTag*, RE::BSFixedString name)
{
	if (const auto kind = ParseSkill(name.c_str())) {
		return Config::Get().SkillWeight(*kind);
	}
	return 0.f;
}

void SetSkillWeight(RE::StaticFunctionTag*, RE::BSFixedString name, float weight)
{
	if (const auto kind = ParseSkill(name.c_str())) {
		Config::Get().SetSkillWeight(*kind, weight);
	}
}

#endif  // ADVENTUREXP_WITH_GAME

}  // namespace

bool Register(RE::BSScript::IVirtualMachine* vm)
{
#ifdef ADVENTUREXP_WITH_GAME
	if (!vm) {
		return false;
	}
	constexpr const char* cls = "AdventureXP";
	vm->RegisterFunction("GetVersion", cls, GetVersion);
	vm->RegisterFunction("GetPresetCount", cls, GetPresetCount);
	vm->RegisterFunction("GetPresetId", cls, GetPresetId);
	vm->RegisterFunction("GetPresetName", cls, GetPresetName);
	vm->RegisterFunction("GetActivePreset", cls, GetActivePreset);
	vm->RegisterFunction("ApplyPreset", cls, ApplyPreset);
	vm->RegisterFunction("GetCategoryWeight", cls, GetCategoryWeight);
	vm->RegisterFunction("SetCategoryWeight", cls, SetCategoryWeight);
	vm->RegisterFunction("GetGlobalXPPercent", cls, GetGlobalXPPercent);
	vm->RegisterFunction("SetGlobalXPPercent", cls, SetGlobalXPPercent);
	vm->RegisterFunction("GetEnabled", cls, GetEnabled);
	vm->RegisterFunction("SetEnabled", cls, SetEnabled);
	vm->RegisterFunction("GetShowMessages", cls, GetShowMessages);
	vm->RegisterFunction("SetShowMessages", cls, SetShowMessages);
	vm->RegisterFunction("GetAwardKilling", cls, GetAwardKilling);
	vm->RegisterFunction("SetAwardKilling", cls, SetAwardKilling);
	vm->RegisterFunction("GetAwardReading", cls, GetAwardReading);
	vm->RegisterFunction("SetAwardReading", cls, SetAwardReading);
	vm->RegisterFunction("GetQuestXP", cls, GetQuestXP);
	vm->RegisterFunction("SetQuestXP", cls, SetQuestXP);
	vm->RegisterFunction("GetDiscoveryXP", cls, GetDiscoveryXP);
	vm->RegisterFunction("SetDiscoveryXP", cls, SetDiscoveryXP);
	vm->RegisterFunction("GetClearXP", cls, GetClearXP);
	vm->RegisterFunction("SetClearXP", cls, SetClearXP);
	vm->RegisterFunction("GetQuestTypeWeight", cls, GetQuestTypeWeight);
	vm->RegisterFunction("SetQuestTypeWeight", cls, SetQuestTypeWeight);
	vm->RegisterFunction("GetPlaceTypeWeight", cls, GetPlaceTypeWeight);
	vm->RegisterFunction("SetPlaceTypeWeight", cls, SetPlaceTypeWeight);
	vm->RegisterFunction("GetSkillWeight", cls, GetSkillWeight);
	vm->RegisterFunction("SetSkillWeight", cls, SetSkillWeight);
	return true;
#else
	(void)vm;
	return false;
#endif
}

}  // namespace AdventureXP::Papyrus
