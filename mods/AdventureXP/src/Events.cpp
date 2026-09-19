#include "AdventureXP/Events.h"

#include "AdventureXP/Awards.h"
#include "AdventureXP/Config.h"
#include "AdventureXP/Types.h"

#include <cmath>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <vector>

#ifdef ADVENTUREXP_WITH_GAME
#	include <RE/Skyrim.h>
#endif

namespace AdventureXP::Events {
namespace {

#ifdef ADVENTUREXP_WITH_GAME

template <class T>
std::uint16_t MarkerEnumValue(const T& type)
{
	if constexpr (requires { type.get(); }) {
		return static_cast<std::uint16_t>(type.get());
	} else if constexpr (requires { type.underlying(); }) {
		return static_cast<std::uint16_t>(type.underlying());
	} else {
		return static_cast<std::uint16_t>(type);
	}
}

std::vector<std::string> LocationKeywords(RE::BGSLocation* location)
{
	std::vector<std::string> names;
	if (!location) {
		return names;
	}
	// LookupByEditorID + HasKeyword is the v8-safe path. ForEachKeyword
	// overloads and keyword-array iteration differ across CommonLib pins.
	for (const auto& entry : kLocTypeMap) {
		const auto* keyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(entry.keyword.data());
		if (keyword && location->HasKeyword(keyword)) {
			names.emplace_back(entry.keyword);
		}
	}
	return names;
}

std::optional<std::uint16_t> MarkerTypeFor(RE::BGSLocation* location)
{
	if (!location) {
		return std::nullopt;
	}
	RE::TESObjectREFR* marker = nullptr;
	if (location->worldLocMarker) {
		// v8: worldLocMarker.get() is NiPointer<TESObjectREFR>.
		marker = location->worldLocMarker.get().get();
	}
	if (!marker) {
		return std::nullopt;
	}
	if (auto* extra = marker->extraList.GetByType<RE::ExtraMapMarker>()) {
		if (extra->mapData) {
			return MarkerEnumValue(extra->mapData->type);
		}
	}
	return std::nullopt;
}

bool HasKeywordEDID(RE::TESObjectREFR* refr, const char* edid)
{
	auto* keyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(edid);
	if (!keyword || !refr) {
		return false;
	}
	if (auto* actor = refr->As<RE::Actor>()) {
		return actor->HasKeyword(keyword);
	}
	return false;
}

PlaceKind PlaceFor(RE::BGSLocation* location)
{
	const auto keywords = LocationKeywords(location);
	const auto marker = MarkerTypeFor(location);
	const char* edid = location ? location->GetFormEditorID() : "";
	return ClassifyPlace(keywords, marker, edid ? edid : "");
}

std::uint8_t QuestCKType(RE::TESQuest* quest)
{
	if (!quest) {
		return 0;
	}
	return static_cast<std::uint8_t>(quest->GetType());
}

QuestKind KindFor(RE::TESQuest* quest)
{
	const char* edid = quest ? quest->GetFormEditorID() : "";
	const std::vector<std::string_view> none;
	return ClassifyQuest(QuestCKType(quest), edid ? edid : "", none);
}

bool ShouldSkipQuest(RE::TESQuest* quest)
{
	const auto& cfg = Config::Get();
	if (!quest) {
		return true;
	}
	if (cfg.skipHiddenQuests) {
		const auto* name = quest->GetName();
		if (!name || name[0] == '\0') {
			return true;
		}
	}
	if (cfg.skipMiscQuests && KindFor(quest) == QuestKind::Misc) {
		return true;
	}
	return false;
}

std::unordered_set<RE::FormID> g_cleared;
std::unordered_set<RE::FormID> g_clearedAtStart;

void RememberAlreadyCleared()
{
	g_clearedAtStart.clear();
	if (auto* player = RE::PlayerCharacter::GetSingleton()) {
		if (auto* location = player->GetCurrentLocation(); location && location->IsCleared()) {
			g_clearedAtStart.insert(location->GetFormID());
		}
	}
}

void MaybeAwardClear(RE::BGSLocation* location)
{
	if (!location || !location->IsCleared()) {
		return;
	}
	const auto id = location->GetFormID();
	if (g_cleared.contains(id)) {
		return;
	}
	if (g_clearedAtStart.contains(id)) {
		g_cleared.insert(id);
		return;
	}
	g_cleared.insert(id);
	const auto kind = PlaceFor(location);
	const float xp = Awards::Scale(static_cast<float>(Config::Get().ClearXP(kind)), Category::Clear);
	Awards::Give(xp, Key(kind));
}

class QuestSink : public RE::BSTEventSink<RE::TESQuestStageEvent> {
public:
	RE::BSEventNotifyControl ProcessEvent(
		const RE::TESQuestStageEvent* event,
		RE::BSTEventSource<RE::TESQuestStageEvent>*) override
	{
		if (!event || !Config::Get().enabled) {
			return RE::BSEventNotifyControl::kContinue;
		}
		auto* quest = RE::TESForm::LookupByID<RE::TESQuest>(event->formID);
		if (ShouldSkipQuest(quest)) {
			return RE::BSEventNotifyControl::kContinue;
		}

		auto& cfg = Config::Get();
		if (cfg.awardQuestStages) {
			Awards::Give(
				Awards::Scale(static_cast<float>(cfg.QuestXP(QuestKind::Objectives)), Category::Quest),
				"quest objective");
		}
		if (cfg.awardQuestComplete && quest && quest->IsCompleted()) {
			const auto kind = KindFor(quest);
			float amount = static_cast<float>(cfg.QuestXP(kind));
			if (kind == QuestKind::Main) {
				amount *= cfg.mainQuestMultiplier;
			}
			Awards::Give(Awards::Scale(amount, Category::Quest), Key(kind));
		}
		return RE::BSEventNotifyControl::kContinue;
	}
};

class DiscoverSink : public RE::BSTEventSink<RE::LocationDiscovery::Event> {
public:
	RE::BSEventNotifyControl ProcessEvent(
		const RE::LocationDiscovery::Event*,
		RE::BSTEventSource<RE::LocationDiscovery::Event>*) override
	{
		if (!Config::Get().enabled) {
			return RE::BSEventNotifyControl::kContinue;
		}
		auto* player = RE::PlayerCharacter::GetSingleton();
		auto* location = player ? player->GetCurrentLocation() : nullptr;
		const auto kind = PlaceFor(location);
		Awards::Give(
			Awards::Scale(static_cast<float>(Config::Get().DiscoveryXP(kind)), Category::Discovery),
			Key(kind));
		return RE::BSEventNotifyControl::kContinue;
	}
};

class DeathSink : public RE::BSTEventSink<RE::TESDeathEvent> {
public:
	RE::BSEventNotifyControl ProcessEvent(
		const RE::TESDeathEvent* event,
		RE::BSTEventSource<RE::TESDeathEvent>*) override
	{
		auto& cfg = Config::Get();
		if (!event || !cfg.enabled) {
			return RE::BSEventNotifyControl::kContinue;
		}

		auto* player = RE::PlayerCharacter::GetSingleton();
		if (player) {
			MaybeAwardClear(player->GetCurrentLocation());
		}

		if (!cfg.awardKilling || !player || event->actorKiller.get() != player) {
			return RE::BSEventNotifyControl::kContinue;
		}

		auto* victim = event->actorDying.get();
		float base = cfg.killXP;
		if (victim) {
			if (const auto* edid = victim->GetFormEditorID(); edid && ContainsI(edid, "Boss")) {
				base = cfg.bossKillXP;
			}
		}
		float bonus = 1.f;
		if (victim && cfg.undeadCombatBonus > 0.f && HasKeywordEDID(victim, "ActorTypeUndead")) {
			bonus += cfg.undeadCombatBonus / 100.f;
		}
		if (cfg.stealthCombatBonus > 0.f && player->IsSneaking()) {
			bonus += cfg.stealthCombatBonus / 100.f;
		}
		if (victim && cfg.beastCombatBonus > 0.f && HasKeywordEDID(victim, "ActorTypeAnimal")) {
			bonus += cfg.beastCombatBonus / 100.f;
		}
		Awards::Give(Awards::Scale(base * bonus, Category::Combat), "kill");
		return RE::BSEventNotifyControl::kContinue;
	}
};

// 4.2.5 crashed here: PlayerCharacter::GetBaseActorValue is a virtual
// ActorValueOwner call (vtable+0x18). C++ inheritance adjusts `this` with
// the compile-time AVO offset, which is wrong on 1.7.104 (RAX garbage,
// RCX="d"). Prefer event->player, then PlayerSkills via GetPlayerRuntimeData
// (same path Awards::BumpVanillaLevel already uses), then versioned
// AsActorValueOwner(). If the read fails, award flat fSkillUpXP — no crash.
RE::PlayerCharacter* EventPlayer(const RE::SkillIncrease::Event* event)
{
	auto* singleton = RE::PlayerCharacter::GetSingleton();
	if (event->player && (!singleton || event->player == singleton)) {
		return event->player;
	}
	return singleton;
}

bool PlausibleSkillLevel(float raw, int& outLevel)
{
	if (!std::isfinite(raw)) {
		return false;
	}
	const int level = static_cast<int>(raw);
	if (level < 1 || level > 252) {
		return false;
	}
	outLevel = level;
	return true;
}

bool TryReadSkillLevel(const RE::SkillIncrease::Event* event, int& outLevel)
{
	auto* player = EventPlayer(event);
	if (!player) {
		return false;
	}

	if (auto* skills = player->GetPlayerRuntimeData().skills) {
		if (auto* data = skills->data) {
			if (const auto idx = SkillDataIndexFromActorValue(static_cast<std::int32_t>(event->actorValue))) {
				if (PlausibleSkillLevel(data->skills[*idx].level, outLevel)) {
					return true;
				}
			}
		}
	}

	if (auto* avo = player->AsActorValueOwner()) {
		if (PlausibleSkillLevel(avo->GetBaseActorValue(event->actorValue), outLevel)) {
			return true;
		}
	}
	return false;
}

class SkillSink : public RE::BSTEventSink<RE::SkillIncrease::Event> {
public:
	RE::BSEventNotifyControl ProcessEvent(
		const RE::SkillIncrease::Event* event,
		RE::BSTEventSource<RE::SkillIncrease::Event>*) override
	{
		auto& cfg = Config::Get();
		if (!event || !cfg.enabled) {
			return RE::BSEventNotifyControl::kContinue;
		}
		const auto skill = SkillFromActorValue(static_cast<std::int32_t>(event->actorValue));
		if (!skill) {
			return RE::BSEventNotifyControl::kContinue;
		}
		int skillLevel = 0;
		float baseXP = cfg.skillUpXP;
		if (TryReadSkillLevel(event, skillLevel)) {
			baseXP = Awards::SkillUpBaseXP(cfg.skillUpXP, skillLevel, cfg.skillUpLevelScale);
		}
		const float weight = cfg.SkillWeight(*skill) / 100.f;
		Awards::Give(Awards::Scale(baseXP * weight, Category::SkillUp), Key(*skill));
		return RE::BSEventNotifyControl::kContinue;
	}
};

class BookSink : public RE::BSTEventSink<RE::BooksRead::Event> {
public:
	RE::BSEventNotifyControl ProcessEvent(
		const RE::BooksRead::Event* event,
		RE::BSTEventSource<RE::BooksRead::Event>*) override
	{
		auto& cfg = Config::Get();
		if (!event || !cfg.enabled || !cfg.awardReading) {
			return RE::BSEventNotifyControl::kContinue;
		}
		auto* book = event->book;
		if (!book) {
			return RE::BSEventNotifyControl::kContinue;
		}
		const float baseXP = Awards::ReadingBaseXP(book->GetGoldValue(), cfg.readingMult);
		Awards::Give(Awards::Scale(baseXP, Category::Reading), "reading");
		return RE::BSEventNotifyControl::kContinue;
	}
};

QuestSink g_quest;
DiscoverSink g_discover;
DeathSink g_death;
BookSink g_book;
SkillSink g_skill;

#endif  // ADVENTUREXP_WITH_GAME

}  // namespace

void Register()
{
#ifdef ADVENTUREXP_WITH_GAME
	if (auto* src = RE::ScriptEventSourceHolder::GetSingleton()) {
		src->AddEventSink(&g_quest);
		src->AddEventSink(&g_death);
	}
	if (auto* src = RE::LocationDiscovery::GetEventSource()) {
		src->AddEventSink(&g_discover);
	}
	if (auto* src = RE::BooksRead::GetEventSource()) {
		src->AddEventSink(&g_book);
	}
	if (auto* src = RE::SkillIncrease::GetEventSource()) {
		src->AddEventSink(&g_skill);
	}
	RememberAlreadyCleared();
#endif
}

void Unregister()
{
#ifdef ADVENTUREXP_WITH_GAME
	if (auto* src = RE::ScriptEventSourceHolder::GetSingleton()) {
		src->RemoveEventSink(&g_quest);
		src->RemoveEventSink(&g_death);
	}
	if (auto* src = RE::LocationDiscovery::GetEventSource()) {
		src->RemoveEventSink(&g_discover);
	}
	if (auto* src = RE::BooksRead::GetEventSource()) {
		src->RemoveEventSink(&g_book);
	}
	if (auto* src = RE::SkillIncrease::GetEventSource()) {
		src->RemoveEventSink(&g_skill);
	}
#endif
}

}  // namespace AdventureXP::Events
