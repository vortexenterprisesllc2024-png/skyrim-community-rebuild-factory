#pragma once

// Play-style packs. Header-only so host tests and the SKSE plugin share one
// table. MCM (phase 2) should call ApplyPack() / Papyrus ApplyPreset() — the
// same function INI load uses. Newly written; not derived from any other mod.

#include <array>
#include <cstddef>
#include <cstring>
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
    };

    // Adventurer + the requested fantasies. Weights are live sliders (0–300),
    // not labels: a Thief pack really boosts discovery/skill-ups and cuts clears.
    inline constexpr std::array kPlayStylePacks{
        PlayStylePack{
            "adventurer",
            "Adventurer",
            "EverQuest-style default: quests, discovery, and clears carry the level.",
            CategoryWeights{ 100.0f, 100.0f, 100.0f, 0.0f, 0.0f, 0.0f, 0.0f },
            PackFlavor{ 1.5f, false, 0.0f, 0.0f, 0.0f },
        },
        PlayStylePack{
            "vigilant",
            "Vigilant",
            "Stendarr's hunt: barrow clears, undead steel, temple quests, and scripture.",
            CategoryWeights{ 125.0f, 70.0f, 145.0f, 95.0f, 85.0f, 20.0f, 35.0f },
            PackFlavor{ 1.3f, false, 60.0f, 0.0f, 0.0f },
        },
        PlayStylePack{
            "summoner",
            "Summoner",
            "Grimoires and ranks in the schools; minions fight, you study.",
            CategoryWeights{ 90.0f, 65.0f, 75.0f, 25.0f, 145.0f, 45.0f, 90.0f },
            PackFlavor{ 1.2f, true, 0.0f, 0.0f, 0.0f },
        },
        PlayStylePack{
            "thief",
            "Thief",
            "Map markers, locked doors, and light fingers over pitched battles.",
            CategoryWeights{ 75.0f, 155.0f, 45.0f, 40.0f, 20.0f, 35.0f, 120.0f },
            PackFlavor{ 1.1f, false, 0.0f, 45.0f, 0.0f },
        },
        PlayStylePack{
            "assassin",
            "Assassin",
            "Contracts and quiet kills. Little sightseeing, less scholarship.",
            CategoryWeights{ 85.0f, 55.0f, 70.0f, 150.0f, 10.0f, 10.0f, 95.0f },
            PackFlavor{ 1.2f, true, 0.0f, 85.0f, 0.0f },
        },
        PlayStylePack{
            "paladin",
            "Paladin",
            "Oaths, main quests, and cleansing evil from the dark places.",
            CategoryWeights{ 145.0f, 80.0f, 135.0f, 105.0f, 75.0f, 30.0f, 40.0f },
            PackFlavor{ 1.7f, false, 45.0f, 0.0f, 0.0f },
        },
        PlayStylePack{
            "warrior",
            "Warrior",
            "Hold-storming and the forge. Books are for the wounded.",
            CategoryWeights{ 80.0f, 70.0f, 145.0f, 155.0f, 10.0f, 70.0f, 75.0f },
            PackFlavor{ 1.2f, true, 0.0f, 0.0f, 0.0f },
        },
        PlayStylePack{
            "mage",
            "Mage",
            "College work: reading, enchanting, and school skill-ups.",
            CategoryWeights{ 95.0f, 70.0f, 55.0f, 35.0f, 165.0f, 90.0f, 130.0f },
            PackFlavor{ 1.3f, false, 0.0f, 0.0f, 0.0f },
        },
        PlayStylePack{
            "ranger",
            "Ranger",
            "The map is the quest. Trails, camps, and the long bow.",
            CategoryWeights{ 80.0f, 165.0f, 95.0f, 115.0f, 30.0f, 40.0f, 65.0f },
            PackFlavor{ 1.1f, true, 0.0f, 15.0f, 20.0f },
        },
        PlayStylePack{
            "bard",
            "Bard",
            "Stories, songs, and the road. Speech skill-ups and side tales.",
            CategoryWeights{ 135.0f, 125.0f, 40.0f, 30.0f, 110.0f, 25.0f, 140.0f },
            PackFlavor{ 1.4f, false, 0.0f, 0.0f, 0.0f },
        },
        PlayStylePack{
            "merchant",
            "Merchant",
            "Deals, craft, and jobs. Combat is a failed negotiation.",
            CategoryWeights{ 120.0f, 95.0f, 25.0f, 10.0f, 55.0f, 150.0f, 145.0f },
            PackFlavor{ 1.1f, false, 0.0f, 0.0f, 0.0f },
        },
        PlayStylePack{
            "necromancer",
            "Necromancer",
            "Tombs, treatises, and the dead as both subject and workforce.",
            CategoryWeights{ 70.0f, 80.0f, 155.0f, 75.0f, 155.0f, 95.0f, 105.0f },
            PackFlavor{ 1.2f, true, 35.0f, 0.0f, 0.0f },
        },
        PlayStylePack{
            "beastblood",
            "Beastblood",
            "The hunt. Clears and kills; books and benches can wait.",
            CategoryWeights{ 50.0f, 90.0f, 130.0f, 165.0f, 0.0f, 10.0f, 40.0f },
            PackFlavor{ 1.0f, true, 0.0f, 0.0f, 55.0f },
        },
        PlayStylePack{
            "spellsword",
            "Spellsword",
            "Steel and school in the same breath. Balanced war-mage diet.",
            CategoryWeights{ 105.0f, 80.0f, 115.0f, 115.0f, 85.0f, 55.0f, 110.0f },
            PackFlavor{ 1.4f, false, 0.0f, 0.0f, 0.0f },
        },
        PlayStylePack{
            "monk",
            "Monk",
            "Discipline: pilgrimage, training, and restraint in the kill.",
            CategoryWeights{ 105.0f, 115.0f, 65.0f, 45.0f, 95.0f, 0.0f, 150.0f },
            PackFlavor{ 1.3f, true, 0.0f, 0.0f, 0.0f },
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
