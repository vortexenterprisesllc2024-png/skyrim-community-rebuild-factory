#include "AdventureXP/Awards.h"

#include "AdventureXP/Config.h"
#include "AdventureXP/Version.h"

#include <cmath>
#include <cstdio>

#ifdef ADVENTUREXP_WITH_GAME
#	include <RE/Skyrim.h>
#	include <SKSE/SKSE.h>
#endif

namespace AdventureXP::Awards {
namespace {

float g_pool = 0.f;
int g_level = 1;

#ifdef ADVENTUREXP_WITH_GAME
void Notify(std::string_view text)
{
	if (!Config::Get().showMessages || text.empty()) {
		return;
	}
	// CommonLib v8 does not export RE::DebugNotification. Resolve the
	// Address Library AE/SE IDs on the main thread so the eager
	// REL::Relocation constructor cannot run during SKSEPluginLoad.
	const std::string copy{ text };
	if (const auto* tasks = SKSE::GetTaskInterface()) {
		tasks->AddTask([copy]() {
			using func_t = void (*)(const char*, const char*, bool);
			const REL::Relocation<func_t> notify{ REL::RelocationID(52050, 52933) };
			notify(copy.c_str(), nullptr, true);
		});
	}
}

RE::PlayerCharacter::PlayerSkills* LiveSkills()
{
	auto* player = RE::PlayerCharacter::GetSingleton();
	if (!player) {
		return nullptr;
	}
	return player->GetPlayerRuntimeData().skills;
}

void WritePercentGlobal(float percent)
{
	if (auto* data = RE::TESDataHandler::GetSingleton()) {
		if (auto* glob = data->LookupForm<RE::TESGlobal>(0x800, "AdventureXP.esl")) {
			glob->value = percent;
		}
	}
}

void SyncPercentFromSkills(RE::PlayerCharacter::PlayerSkills* skills)
{
	if (!skills || !skills->data) {
		return;
	}
	g_pool = skills->data->xp;
	const float next = skills->data->levelThreshold;
	WritePercentGlobal(next > 0.f ? (g_pool / next) * 100.f : 0.f);
}

void TryLevelUp(RE::PlayerCharacter::PlayerSkills* skills)
{
	if (!skills || !skills->data) {
		return;
	}

	auto* data = skills->data;
	const auto& cfg = Config::Get();
	auto* player = RE::PlayerCharacter::GetSingleton();
	const auto level = player ? player->GetLevel() : 1;
	g_level = level;

	if (cfg.maxLevel > 0 && level >= static_cast<std::uint16_t>(cfg.maxLevel)) {
		return;
	}

	int guard = 0;
	while (data->levelThreshold > 0.0f && data->xp >= data->levelThreshold && guard++ < 16) {
		if (cfg.maxLevel > 0 && player && player->GetLevel() >= static_cast<std::uint16_t>(cfg.maxLevel)) {
			break;
		}
		const float leftover = data->xp - data->levelThreshold;
		skills->AdvanceLevel(true);
		data->xp = (std::max)(0.0f, leftover);
		g_pool = data->xp;
		if (player) {
			g_level = player->GetLevel();
		}
		Notify("You have advanced a level.");
		RE::PlaySound("UILevelUp");
	}
}
#else
void Notify(std::string_view) {}
#endif

}  // namespace

float ThresholdForLevel(int level)
{
	const auto& cfg = Config::Get();
	const float lv = static_cast<float>((std::max)(1, level));
	return cfg.xpBase + cfg.xpPerLevel * std::pow(lv, cfg.xpExponent);
}

float Scale(float base, Category category)
{
	const auto& cfg = Config::Get();
	if (!cfg.enabled || base <= 0.f) {
		return 0.f;
	}
	if (category == Category::Combat && !cfg.awardKilling) {
		return 0.f;
	}
	if (category == Category::Reading && !cfg.awardReading) {
		return 0.f;
	}
	const float cat = cfg.CategoryWeight(category) / 100.f;
	const float global = cfg.globalXPPercent / 100.f;
	return base * cat * global;
}

void SyncFromPlayer()
{
#ifdef ADVENTUREXP_WITH_GAME
	if (auto* player = RE::PlayerCharacter::GetSingleton()) {
		g_level = player->GetLevel();
	}
	SyncPercentFromSkills(LiveSkills());
#endif
}

void Give(float amount, std::string_view reason)
{
	if (amount <= 0.f) {
		return;
	}

#ifdef ADVENTUREXP_WITH_GAME
	// Vanilla HUD reads PlayerSkills XP. The private curve (g_pool /
	// ThresholdForLevel) is not the meter Jo sees — write the stock page.
	auto* skills = LiveSkills();
	if (!skills || !skills->data) {
		SKSE::log::warn("Cannot award XP: PlayerSkills unavailable");
	} else {
		skills->data->xp += amount;
		g_pool = skills->data->xp;
		if (auto* player = RE::PlayerCharacter::GetSingleton()) {
			g_level = player->GetLevel();
		}
		TryLevelUp(skills);
		SyncPercentFromSkills(skills);
	}
#else
	auto& cfg = Config::Get();
	g_pool += amount;
	while (g_level < cfg.maxLevel && g_pool >= ThresholdForLevel(g_level)) {
		g_pool -= ThresholdForLevel(g_level);
		++g_level;
	}
#endif

	char buf[160];
	if (reason.empty()) {
		std::snprintf(buf, sizeof(buf), "+%.0f XP", amount);
	} else {
		std::snprintf(buf, sizeof(buf), "+%.0f XP (%.*s)", amount, static_cast<int>(reason.size()), reason.data());
	}
	Notify(buf);
}

}  // namespace AdventureXP::Awards
