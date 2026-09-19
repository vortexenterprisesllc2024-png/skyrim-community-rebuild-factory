#include "AdventureXP/Awards.h"

#include "AdventureXP/Config.h"
#include "AdventureXP/Version.h"

#include <algorithm>
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

void BumpVanillaLevel(int)
{
	auto* player = RE::PlayerCharacter::GetSingleton();
	if (!player) {
		return;
	}
	// v8 PlayerCharacter has no SetLevel. Advance the vanilla skill
	// XP page the same way the factory 4.0 DLL did.
	if (auto* skills = player->GetPlayerRuntimeData().skills) {
		skills->AdvanceLevel(true);
	}
}

void WritePercentGlobal(float percent)
{
	if (auto* data = RE::TESDataHandler::GetSingleton()) {
		if (auto* glob = data->LookupForm<RE::TESGlobal>(0x800, "AdventureXP.esl")) {
			glob->value = percent;
		}
	}
}
#else
void Notify(std::string_view) {}
void BumpVanillaLevel(int) {}
void WritePercentGlobal(float) {}
#endif

}  // namespace

float ThresholdForLevel(int level)
{
	const auto& cfg = Config::Get();
	const float lv = static_cast<float>((std::max)(1, level));
	return cfg.xpBase + cfg.xpPerLevel * std::pow(lv, cfg.xpExponent);
}

float ReadingBaseXP(int goldValue, float readingMult)
{
	const float gold = (std::max)(0.f, static_cast<float>(goldValue));
	return std::floor(std::sqrt(gold) * readingMult);
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
	// Reading and Combat ignore fGlobalXPPercent. Jo runs global at 2% so
	// quests/discovery stay slow; 5 reading XP * 1.0 * 0.02 = 0.1, which the
	// +%.0f toast prints as +0. Category weight still applies.
	if (category == Category::Reading || category == Category::Combat) {
		return base * cat;
	}
	const float global = cfg.globalXPPercent / 100.f;
	return base * cat * global;
}

void Give(float amount, std::string_view reason)
{
	if (amount <= 0.f) {
		return;
	}

	auto& cfg = Config::Get();
	g_pool += amount;
	while (g_level < cfg.maxLevel && g_pool >= ThresholdForLevel(g_level)) {
		g_pool -= ThresholdForLevel(g_level);
		++g_level;
		BumpVanillaLevel(g_level);
	}

	const float next = ThresholdForLevel(g_level);
	const float percent = next > 0.f ? (g_pool / next) * 100.f : 0.f;
	WritePercentGlobal(percent);

	char buf[160];
	if (reason.empty()) {
		std::snprintf(buf, sizeof(buf), "+%.0f XP", amount);
	} else {
		std::snprintf(buf, sizeof(buf), "+%.0f XP (%.*s)", amount, static_cast<int>(reason.size()), reason.data());
	}
	Notify(buf);
}

}  // namespace AdventureXP::Awards
