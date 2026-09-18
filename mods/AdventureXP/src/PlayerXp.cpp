#include "AdventureXP/PCH.h"
#include "AdventureXP/PlayerXp.h"
#include "AdventureXP/Config.h"
#include "AdventureXP/Globals.h"
#include "AdventureXP/Messages.h"
#include "AdventureXP/XpMath.h"

namespace AdventureXP
{
    const char* ToString(AwardSource source)
    {
        switch (source) {
        case AwardSource::QuestStage:
            return "quest stage";
        case AwardSource::QuestComplete:
            return "quest complete";
        case AwardSource::Discovery:
            return "discovery";
        case AwardSource::DungeonClear:
            return "cleared";
        case AwardSource::Kill:
            return "defeat";
        case AwardSource::Reading:
            return "reading";
        case AwardSource::Crafting:
            return "crafting";
        case AwardSource::SkillUp:
            return "training";
        }
        return "adventure";
    }

    namespace
    {
        float WeightFor(AwardSource source, const Config& cfg)
        {
            switch (source) {
            case AwardSource::QuestStage:
            case AwardSource::QuestComplete:
                return cfg.weights.quests;
            case AwardSource::Discovery:
                return cfg.weights.discovery;
            case AwardSource::DungeonClear:
                return cfg.weights.clears;
            case AwardSource::Kill:
                return cfg.weights.combat;
            case AwardSource::Reading:
                return cfg.weights.reading;
            case AwardSource::Crafting:
                return cfg.weights.crafting;
            case AwardSource::SkillUp:
                return cfg.weights.skillups;
            }
            return 100.0f;
        }
    }

    PlayerXp& PlayerXp::Get()
    {
        static PlayerXp instance;
        return instance;
    }

    RE::PlayerSkills* PlayerXp::Skills() const
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            return nullptr;
        }
        return player->GetPlayerRuntimeData().skills;
    }

    void PlayerXp::TryLevelUp(RE::PlayerSkills* skills)
    {
        if (!skills || !skills->data) {
            return;
        }

        auto* data = skills->data;
        const auto& curve = Config::Get().curve;
        auto* player = RE::PlayerCharacter::GetSingleton();
        const auto level = player ? player->GetLevel() : 1;

        if (curve.maxLevel > 0 && level >= static_cast<std::uint16_t>(curve.maxLevel)) {
            return;
        }

        int guard = 0;
        while (data->levelThreshold > 0.0f && data->xp >= data->levelThreshold && guard++ < 16) {
            const float leftover = data->xp - data->levelThreshold;
            skills->AdvanceLevel(true);
            data->xp = std::max(0.0f, leftover);
            logger::info("Player leveled up (leftover XP {:.1f})", data->xp);
            Notify("You have advanced a level.");
            RE::PlaySound("UILevelUp");
        }
    }

    float PlayerXp::CurrentPercent() const
    {
        auto* skills = Skills();
        if (!skills || !skills->data) {
            return 0.0f;
        }
        return Math::ProgressPercent(skills->data->xp, skills->data->levelThreshold);
    }

    void PlayerXp::SyncGlobal()
    {
        SetPercentGlobal(CurrentPercent());
    }

    void PlayerXp::Award(AwardSource source, float rawXP, std::string_view detail)
    {
        const auto& cfg = Config::Get();
        if (!cfg.enabled || rawXP <= 0.0f) {
            return;
        }

        auto* skills = Skills();
        if (!skills || !skills->data) {
            logger::warn("Cannot award XP: PlayerSkills unavailable");
            return;
        }

        const float amount = Math::ApplyGlobalPercent(
            Math::ApplyGlobalPercent(rawXP, WeightFor(source, cfg)),
            cfg.globalXPPercent);
        if (amount <= 0.0f) {
            return;
        }

        skills->data->xp += amount;
        TryLevelUp(skills);
        SyncGlobal();

        const float pct = CurrentPercent();
        logger::info("Awarded {:.1f} XP for {} ({}) — now {:.1f}%", amount, ToString(source), detail, pct);

        if (cfg.showXPMessages) {
            const auto text = std::format(
                "Adventure XP: +{:.0f} ({}) — {:.0f}%",
                amount,
                detail.empty() ? ToString(source) : detail,
                pct);
            Notify(text);
        }
    }
}
