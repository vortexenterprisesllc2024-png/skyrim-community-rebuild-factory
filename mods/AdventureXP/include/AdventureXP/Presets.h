#pragma once

// Play-style packs. Header-only so host tests and the SKSE plugin share one
// table. ApplyPreset writes category + quest-type + place-type scales.
// Newly written; not derived from any other mod.
//
// Design: each pack pays well for its fantasy and near-zero for what fights it.

#include "AdventureXP/Types.h"

#include <array>
#include <cstddef>
#include <string_view>

namespace AdventureXP
{
    struct CategoryWeights
    {
        float quests{ 100.0f };
        float discovery{ 100.0f };
        float clears{ 100.0f };
        float combat{ 0.0f };
        float reading{ 0.0f };
        float crafting{ 0.0f };
        float skillups{ 0.0f };

        friend constexpr bool operator==(const CategoryWeights&, const CategoryWeights&) = default;
    };

    struct PackFlavor
    {
        float mainQuestMultiplier{ 1.5f };
        bool skipMiscQuests{ false };
        float undeadCombatBonus{ 0.0f };
        float stealthCombatBonus{ 0.0f };
        float beastCombatBonus{ 0.0f };
    };

    struct PlayStylePack
    {
        const char* id;
        const char* displayName;
        const char* blurb;
        CategoryWeights weights;
        PackFlavor flavor;
        QuestTypeWeights questTypes;
        PlaceTypeWeights placeTypes;
    };

    // QuestTypeWeights: main, guild, daedric, side, misc, civilWar, dlc, other
    // PlaceTypeWeights: def, city, dungeon, cave, nordic, dwemer, fort, camp, dragon, mine

    inline constexpr std::array kPlayStylePacks{
        // Adventurer — balanced road life; no dump stats
        PlayStylePack{
            "adventurer",
            "Adventurer",
            "Quests, discovery, and clears carry the level. Optional sources stay off.",
            CategoryWeights{ 100.0f, 100.0f, 100.0f, 0.0f, 0.0f, 0.0f, 0.0f },
            PackFlavor{ 1.5f, false, 0.0f, 0.0f, 0.0f },
            QuestTypeWeights{ 100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f },
            PlaceTypeWeights{ 100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f },
        },
        // Vigilant — Stendarr: barrows/undead yes; Daedric never
        PlayStylePack{
            "vigilant",
            "Vigilant",
            "Pays: barrow clears, undead, temple work, scripture. Denies: Daedric.",
            CategoryWeights{ 100.0f, 55.0f, 100.0f, 85.0f, 75.0f, 10.0f, 25.0f },
            PackFlavor{ 1.35f, true, 80.0f, 0.0f, 0.0f },
            QuestTypeWeights{ 100.0f, 35.0f, 0.0f, 85.0f, 15.0f, 40.0f, 70.0f, 40.0f },
            PlaceTypeWeights{ 50.0f, 25.0f, 90.0f, 95.0f, 100.0f, 70.0f, 55.0f, 30.0f, 90.0f, 45.0f },
        },
        // Conjurer (id summoner) — College study; minions fight
        PlayStylePack{
            "summoner",
            "Conjurer",
            "Pays: College, reading, skill-ups. Denies: open war and heavy clears.",
            CategoryWeights{ 70.0f, 45.0f, 35.0f, 15.0f, 100.0f, 40.0f, 90.0f },
            PackFlavor{ 1.15f, true, 0.0f, 0.0f, 0.0f },
            QuestTypeWeights{ 60.0f, 100.0f, 40.0f, 50.0f, 20.0f, 10.0f, 55.0f, 40.0f },
            PlaceTypeWeights{ 40.0f, 70.0f, 40.0f, 35.0f, 30.0f, 50.0f, 20.0f, 15.0f, 25.0f, 20.0f },
        },
        // Thief — markers and guild; not a soldier
        PlayStylePack{
            "thief",
            "Thief",
            "Pays: discovery, Thieves/guild work, skill-ups. Denies: Civil War, big clears.",
            CategoryWeights{ 55.0f, 100.0f, 20.0f, 35.0f, 10.0f, 30.0f, 95.0f },
            PackFlavor{ 1.05f, false, 0.0f, 55.0f, 0.0f },
            QuestTypeWeights{ 35.0f, 100.0f, 25.0f, 70.0f, 80.0f, 5.0f, 40.0f, 50.0f },
            PlaceTypeWeights{ 60.0f, 100.0f, 50.0f, 55.0f, 40.0f, 45.0f, 35.0f, 30.0f, 15.0f, 40.0f },
        },
        // Assassin — contracts and quiet steel
        PlayStylePack{
            "assassin",
            "Assassin",
            "Pays: Brotherhood-style guild, stealth kills. Denies: tourism and books.",
            CategoryWeights{ 60.0f, 30.0f, 45.0f, 100.0f, 0.0f, 5.0f, 70.0f },
            PackFlavor{ 1.1f, true, 0.0f, 90.0f, 0.0f },
            QuestTypeWeights{ 40.0f, 100.0f, 50.0f, 55.0f, 70.0f, 15.0f, 45.0f, 40.0f },
            PlaceTypeWeights{ 40.0f, 70.0f, 60.0f, 50.0f, 55.0f, 40.0f, 50.0f, 45.0f, 30.0f, 35.0f },
        },
        // Paladin — oaths and cleansing; rejects Daedra
        PlayStylePack{
            "paladin",
            "Paladin",
            "Pays: Main quest, clears, undead. Denies: Daedric, thieves, Brotherhood.",
            CategoryWeights{ 100.0f, 50.0f, 100.0f, 90.0f, 60.0f, 20.0f, 30.0f },
            PackFlavor{ 1.7f, true, 70.0f, 0.0f, 0.0f },
            QuestTypeWeights{ 100.0f, 25.0f, 0.0f, 75.0f, 10.0f, 55.0f, 80.0f, 35.0f },
            PlaceTypeWeights{ 45.0f, 40.0f, 95.0f, 90.0f, 100.0f, 75.0f, 70.0f, 25.0f, 100.0f, 40.0f },
        },
        // Warrior — storm the hold; books are for the wounded
        PlayStylePack{
            "warrior",
            "Warrior",
            "Pays: clears, combat, forge. Denies: reading and College life.",
            CategoryWeights{ 50.0f, 40.0f, 100.0f, 100.0f, 0.0f, 65.0f, 55.0f },
            PackFlavor{ 1.2f, true, 0.0f, 0.0f, 0.0f },
            QuestTypeWeights{ 70.0f, 55.0f, 30.0f, 50.0f, 15.0f, 90.0f, 60.0f, 40.0f },
            PlaceTypeWeights{ 50.0f, 30.0f, 100.0f, 80.0f, 90.0f, 85.0f, 100.0f, 70.0f, 100.0f, 60.0f },
        },
        // Mage — College and grimoires
        PlayStylePack{
            "mage",
            "Mage",
            "Pays: College/guild, reading, skill-ups, enchanting. Denies: front-line clears.",
            CategoryWeights{ 75.0f, 40.0f, 25.0f, 20.0f, 100.0f, 85.0f, 100.0f },
            PackFlavor{ 1.25f, false, 0.0f, 0.0f, 0.0f },
            QuestTypeWeights{ 55.0f, 100.0f, 70.0f, 45.0f, 25.0f, 10.0f, 65.0f, 40.0f },
            PlaceTypeWeights{ 40.0f, 80.0f, 35.0f, 30.0f, 25.0f, 70.0f, 20.0f, 15.0f, 30.0f, 25.0f },
        },
        // Ranger — the map is the quest
        PlayStylePack{
            "ranger",
            "Ranger",
            "Pays: discovery, camps, wilderness, hunting. Denies: city busywork.",
            CategoryWeights{ 45.0f, 100.0f, 70.0f, 80.0f, 15.0f, 25.0f, 50.0f },
            PackFlavor{ 1.05f, true, 0.0f, 20.0f, 25.0f },
            QuestTypeWeights{ 40.0f, 30.0f, 20.0f, 70.0f, 25.0f, 35.0f, 50.0f, 45.0f },
            PlaceTypeWeights{ 70.0f, 15.0f, 60.0f, 90.0f, 75.0f, 40.0f, 55.0f, 100.0f, 85.0f, 70.0f },
        },
        // Bard — stories and the road
        PlayStylePack{
            "bard",
            "Bard",
            "Pays: side tales, discovery, speech skill-ups. Denies: dungeon grinding.",
            CategoryWeights{ 90.0f, 95.0f, 15.0f, 15.0f, 80.0f, 15.0f, 100.0f },
            PackFlavor{ 1.4f, false, 0.0f, 0.0f, 0.0f },
            QuestTypeWeights{ 70.0f, 40.0f, 35.0f, 100.0f, 90.0f, 30.0f, 55.0f, 70.0f },
            PlaceTypeWeights{ 80.0f, 100.0f, 25.0f, 30.0f, 35.0f, 30.0f, 25.0f, 50.0f, 20.0f, 25.0f },
        },
        // Merchant — deals not blood
        PlayStylePack{
            "merchant",
            "Merchant",
            "Pays: craft, skill-ups, jobs. Denies: combat and clears.",
            CategoryWeights{ 70.0f, 55.0f, 5.0f, 0.0f, 35.0f, 100.0f, 100.0f },
            PackFlavor{ 1.05f, false, 0.0f, 0.0f, 0.0f },
            QuestTypeWeights{ 40.0f, 35.0f, 15.0f, 60.0f, 100.0f, 10.0f, 40.0f, 55.0f },
            PlaceTypeWeights{ 70.0f, 100.0f, 10.0f, 15.0f, 10.0f, 20.0f, 15.0f, 20.0f, 5.0f, 40.0f },
        },
        // Summoner (id necromancer) — tombs and treatises; Jo display name
        PlayStylePack{
            "necromancer",
            "Summoner",
            "Pays: tomb clears, reading, craft. Denies: sunny tourism and Civil War.",
            CategoryWeights{ 45.0f, 50.0f, 100.0f, 40.0f, 100.0f, 80.0f, 85.0f },
            PackFlavor{ 1.15f, true, 40.0f, 0.0f, 0.0f },
            QuestTypeWeights{ 35.0f, 70.0f, 85.0f, 40.0f, 20.0f, 5.0f, 55.0f, 45.0f },
            PlaceTypeWeights{ 40.0f, 20.0f, 90.0f, 95.0f, 100.0f, 80.0f, 40.0f, 25.0f, 50.0f, 35.0f },
        },
        // Beastblood — the hunt
        PlayStylePack{
            "beastblood",
            "Beastblood",
            "Pays: kills, clears, the hunt. Denies: books and benches.",
            CategoryWeights{ 25.0f, 60.0f, 95.0f, 100.0f, 0.0f, 5.0f, 20.0f },
            PackFlavor{ 1.0f, true, 0.0f, 0.0f, 70.0f },
            QuestTypeWeights{ 20.0f, 25.0f, 30.0f, 40.0f, 15.0f, 35.0f, 40.0f, 30.0f },
            PlaceTypeWeights{ 55.0f, 10.0f, 80.0f, 90.0f, 70.0f, 40.0f, 65.0f, 100.0f, 85.0f, 50.0f },
        },
        // Spellsword — steel and school
        PlayStylePack{
            "spellsword",
            "Spellsword",
            "Pays: balanced war-mage diet across combat, clears, and College.",
            CategoryWeights{ 80.0f, 55.0f, 85.0f, 90.0f, 70.0f, 45.0f, 85.0f },
            PackFlavor{ 1.35f, false, 0.0f, 0.0f, 0.0f },
            QuestTypeWeights{ 85.0f, 85.0f, 50.0f, 60.0f, 30.0f, 50.0f, 70.0f, 50.0f },
            PlaceTypeWeights{ 55.0f, 50.0f, 85.0f, 75.0f, 80.0f, 70.0f, 75.0f, 45.0f, 85.0f, 50.0f },
        },
        // Monk — pilgrimage and restraint
        PlayStylePack{
            "monk",
            "Monk",
            "Pays: pilgrimage, training, restraint. Denies: crime guilds and crafting greed.",
            CategoryWeights{ 70.0f, 90.0f, 40.0f, 25.0f, 70.0f, 0.0f, 100.0f },
            PackFlavor{ 1.25f, true, 0.0f, 0.0f, 0.0f },
            QuestTypeWeights{ 75.0f, 20.0f, 15.0f, 70.0f, 25.0f, 20.0f, 55.0f, 50.0f },
            PlaceTypeWeights{ 70.0f, 40.0f, 45.0f, 60.0f, 55.0f, 40.0f, 35.0f, 50.0f, 40.0f, 35.0f },
        },
    };

    inline constexpr std::size_t kPlayStylePackCount = kPlayStylePacks.size();

    inline int CompareIgnoreCase(std::string_view a, std::string_view b)
    {
        const auto n = a.size() < b.size() ? a.size() : b.size();
        for (std::size_t i = 0; i < n; ++i) {
            const auto ca = static_cast<unsigned char>(a[i]);
            const auto cb = static_cast<unsigned char>(b[i]);
            const auto la = (ca >= 'A' && ca <= 'Z') ? static_cast<char>(ca - 'A' + 'a') : static_cast<char>(ca);
            const auto lb = (cb >= 'A' && cb <= 'Z') ? static_cast<char>(cb - 'A' + 'a') : static_cast<char>(cb);
            if (la != lb) {
                return la < lb ? -1 : 1;
            }
        }
        if (a.size() == b.size()) {
            return 0;
        }
        return a.size() < b.size() ? -1 : 1;
    }

    inline const PlayStylePack* FindPack(std::string_view id)
    {
        if (id.empty() || CompareIgnoreCase(id, "custom") == 0) {
            return nullptr;
        }
        for (const auto& pack : kPlayStylePacks) {
            if (CompareIgnoreCase(id, pack.id) == 0 || CompareIgnoreCase(id, pack.displayName) == 0) {
                return &pack;
            }
        }
        return nullptr;
    }

    inline const PlayStylePack* PackAt(std::size_t index)
    {
        return index < kPlayStylePacks.size() ? &kPlayStylePacks[index] : nullptr;
    }

    inline constexpr const char* kDefaultPresetId = "adventurer";
}