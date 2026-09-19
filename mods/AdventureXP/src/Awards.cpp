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

void BumpVanillaLevel(int newLevel)
{
	if (auto* player = RE::PlayerCharacter::GetSingleton()) {
		player->SetLevel(static_cast<std::uint16_t>(newLevel));
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
