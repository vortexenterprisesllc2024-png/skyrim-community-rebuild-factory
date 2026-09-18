#pragma once

#include <cstdint>
#include <string_view>

namespace AdventureXP
{
    enum class AwardSource : std::uint8_t
    {
        QuestStage,
        QuestComplete,
        Discovery,
        DungeonClear,
        Kill,
        Reading,
        Crafting,
        SkillUp
    };

    const char* ToString(AwardSource source);

    class PlayerXp
    {
    public:
        static PlayerXp& Get();

        void Award(AwardSource source, float rawXP, std::string_view detail);
        void SyncGlobal();
        float CurrentPercent() const;

    private:
        void TryLevelUp();
    };
}
