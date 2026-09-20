#include "AdventureXP/PCH.h"
#include "AdventureXP/SettingsHook.h"
#include "AdventureXP/Config.h"

namespace AdventureXP
{
    void ApplySkillLevelingSetting()
    {
        const auto& cfg = Config::Get();
        if (!cfg.ignoreSkillLeveling) {
            logger::info("Vanilla skill-rank player XP is left unchanged (bIgnoreSkillLeveling=0)");
            return;
        }

        auto* collection = RE::GameSettingCollection::GetSingleton();
        if (!collection) {
            logger::warn("GameSettingCollection missing; cannot zero fXPPerSkillRank");
            return;
        }

        auto* setting = collection->GetSetting("fXPPerSkillRank");
        if (!setting) {
            logger::warn("fXPPerSkillRank not found");
            return;
        }

        const float previous = setting->data.f;
        setting->data.f = 0.0f;
        logger::info("Set fXPPerSkillRank {} -> 0 so skill-ups no longer feed player level XP", previous);
    }
}
