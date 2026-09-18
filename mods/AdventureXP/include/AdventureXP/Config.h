#pragma once

#include "AdventureXP/Presets.h"
#include "AdventureXP/Types.h"
#include "AdventureXP/XpMath.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace AdventureXP
{
    struct Config
    {
        bool enabled{ true };
        float globalXPPercent{ 100.0f };
        bool showXPMessages{ true };
        bool ignoreSkillLeveling{ true };
        std::string presetId{ kDefaultPresetId };

        bool awardQuestStages{ true };
        bool awardQuestComplete{ true };
        bool skipHiddenQuests{ true };
        bool skipMiscQuests{ false };
        float questStageXP{ 12.0f };
        float questCompleteXP{ 80.0f };
        float mainQuestMultiplier{ 1.5f };

        float discoveryXP{ 30.0f };
        float dungeonClearXP{ 100.0f };

        float killXP{ 2.0f };
        float bossKillXP{ 25.0f };

        float readingXP{ 5.0f };
        float craftingXP{ 8.0f };
        float skillUpXP{ 4.0f };

        CategoryWeights weights{};
        PackFlavor flavor{};
        QuestTypeWeights questTypes{};
        PlaceTypeWeights placeTypes{};

        Math::Curve curve{};

        static Config& Get();
        bool Load();
        bool ApplyPreset(std::string_view id);

        bool AwardDiscovery() const { return weights.discovery > 0.0f; }
        bool AwardClears() const { return weights.clears > 0.0f; }
        bool AwardKills() const { return weights.combat > 0.0f; }
        bool AwardReading() const { return weights.reading > 0.0f; }
        bool AwardCrafting() const { return weights.crafting > 0.0f; }
        bool AwardSkillUps() const { return weights.skillups > 0.0f; }

    private:
        std::string path_;
    };
}
