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
bool g_synced = false;

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

void WritePercentGlobal(float percent)
{
	if (auto* data = RE::TESDataHandler::GetSingleton()) {
		if (auto* glob = data->LookupForm<RE::TESGlobal>(0x800, "AdventureXP.esl")) {
			glob->value = percent;
		}
	}
}

RE::PlayerCharacter::PlayerSkills::Data* PlayerSkillData(RE::PlayerCharacter* player)
{
	if (!player) {
		return nullptr;
	}
	if (auto* skills = player->GetPlayerRuntimeData().skills) {
		return skills->data;
	}
	return nullptr;
}

int PlayerLevel(RE::PlayerCharacter* player)
{
	if (!player) {
		return 0;
	}
	const auto level = static_cast<int>(player->GetLevel());
	return level >= 1 ? level : 0;
}

// Jo's HUD is the vanilla XP meter (PlayerSkills::Data::xp / levelThreshold).
// The ESL global is only for third-party HUDs. Writing the vanilla floats is
// what makes the bar she watches move. Do not call GetBaseActorValue here.
bool FeedVanillaXP(float amount)
{
	auto* player = RE::PlayerCharacter::GetSingleton();
	auto* data = PlayerSkillData(player);
	if (!data) {
		return false;
	}

	if (amount > 0.f) {
		data->xp += amount;
	}
	if (!std::isfinite(data->xp) || data->xp < 0.f) {
		data->xp = 0.f;
	}

	if (const int level = PlayerLevel(player); level >= 1) {
		g_level = level;
	}
	g_pool = data->xp;

	const float next = data->levelThreshold > 0.f ? data->levelThreshold : ThresholdForLevel(g_level);
	const float percent = next > 0.f ? (g_pool / next) * 100.f : 0.f;
	WritePercentGlobal(percent);

	// When the vanilla bar fills, let the game grant the level / perk point
	// the same way a skill-up would. AdvanceLevel(false) recomputes the
	// threshold without adding it a second time on top of xp we just wrote.
	if (auto* skills = player->GetPlayerRuntimeData().skills) {
		int guard = 0;
		while (g_level < Config::Get().maxLevel && skills->CanLevelUp() && guard < 8) {
			skills->AdvanceLevel(false);
			if (const int level = PlayerLevel(player); level >= 1) {
				g_level = level;
			} else {
				++g_level;
			}
			g_pool = data->xp;
			++guard;
		}
	}

	const float after = data->levelThreshold > 0.f ? data->levelThreshold : ThresholdForLevel(g_level);
	WritePercentGlobal(after > 0.f ? (data->xp / after) * 100.f : 0.f);
	return true;
}

void BumpVanillaLevel(int)
{
	auto* player = RE::PlayerCharacter::GetSingleton();
	if (!player) {
		return;
	}
	if (auto* skills = player->GetPlayerRuntimeData().skills) {
		skills->AdvanceLevel(true);
	}
}
#else
void Notify(std::string_view) {}
void WritePercentGlobal(float) {}
bool FeedVanillaXP(float) { return false; }
void BumpVanillaLevel(int) {}
#endif

void WritePluginPercent()
{
	const float next = ThresholdForLevel(g_level);
	const float percent = next > 0.f ? (g_pool / next) * 100.f : 0.f;
	WritePercentGlobal(percent);
}

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

float SkillUpBaseXP(float skillUpXP, int skillLevel, float levelScale)
{
	const float level = static_cast<float>((std::max)(1, skillLevel));
	const float scale = (std::max)(1.f, levelScale);
	return skillUpXP * level / scale;
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
	// Quest (stage + complete), Reading, Combat, and Skill-ups ignore
	// fGlobalXPPercent. Jo runs global at ~2% so discovery/clears stay slow;
	// skill training uses only fSkillUpWeight + per-skill [Skills] weights
	// (and fSkillUpXP). Give() still accumulates fractional awards, but skips
	// the toast when lround(amount) < 1 so "+0 XP" never appears.
	if (category == Category::Quest || category == Category::Reading
		|| category == Category::Combat || category == Category::SkillUp) {
		return base * cat;
	}
	const float global = cfg.globalXPPercent / 100.f;
	return base * cat * global;
}

void SetState(int level, float pool)
{
	g_level = (std::max)(1, level);
	g_pool = (std::max)(0.f, pool);
	g_synced = true;
}

int CurrentLevel()
{
	return g_level;
}

float CurrentPool()
{
	return g_pool;
}

void SyncFromPlayer()
{
#ifdef ADVENTUREXP_WITH_GAME
	auto* player = RE::PlayerCharacter::GetSingleton();
	if (!player) {
		return;
	}
	if (const int level = PlayerLevel(player); level >= 1) {
		g_level = level;
	}
	if (auto* data = PlayerSkillData(player)) {
		g_pool = data->xp;
		const float next = data->levelThreshold > 0.f ? data->levelThreshold : ThresholdForLevel(g_level);
		WritePercentGlobal(next > 0.f ? (g_pool / next) * 100.f : 0.f);
	}
	g_synced = true;
#else
	g_synced = true;
#endif
}

void Give(float amount, std::string_view reason)
{
	if (amount <= 0.f) {
		return;
	}

	auto& cfg = Config::Get();
	if (!g_synced) {
		SyncFromPlayer();
	}

	// Prefer the vanilla XP page so the HUD meter Jo watches actually moves.
	// Fall back to the plugin-only pool if PlayerSkills is not ready (host
	// tests, or a load where the player pointer is still null).
	if (!FeedVanillaXP(amount)) {
		g_pool += amount;
		while (g_level < cfg.maxLevel && g_pool >= ThresholdForLevel(g_level)) {
			g_pool -= ThresholdForLevel(g_level);
			++g_level;
			BumpVanillaLevel(g_level);
		}
		WritePluginPercent();
	}

	// Fractional awards (quest stage * low global, tiny kill * low combat) must not toast "+0 XP".
	const int shown = static_cast<int>(std::lround(static_cast<double>(amount)));
	if (shown < 1) {
		return;
	}

	char buf[160];
	if (reason.empty()) {
		std::snprintf(buf, sizeof(buf), "+%d XP", shown);
	} else {
		std::snprintf(buf, sizeof(buf), "+%d XP (%.*s)", shown, static_cast<int>(reason.size()), reason.data());
	}
	Notify(buf);
}

}  // namespace AdventureXP::Awards
