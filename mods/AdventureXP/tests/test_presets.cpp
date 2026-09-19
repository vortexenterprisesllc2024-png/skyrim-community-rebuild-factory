#include "AdventureXP/Presets.h"
#include "AdventureXP/Version.h"
#include "AdventureXP/XpMath.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>
#include <string>
#include <string_view>

namespace
{
    int g_failed = 0;

    void expect(bool ok, const char* what)
    {
        if (!ok) {
            std::cerr << "FAIL: " << what << "\n";
            ++g_failed;
        }
    }
}

int main()
{
    using namespace AdventureXP;

    expect(std::strcmp(kVersionString, "4.0.0") == 0, "product version is 4.0.0");
    expect(kVersionMajor == 4 && kVersionMinor == 0, "major.minor is 4.0");
    expect(kPlayStylePackCount >= 12, "at least 12 play-style packs");

    const char* required[] = {
        "vigilant",
        "summoner",
        "thief",
        "assassin",
        "paladin",
        "warrior",
        "mage",
        "ranger",
        "bard",
        "merchant",
        "necromancer",
        "beastblood",
        "spellsword",
        "monk",
        "adventurer",
    };
    for (const auto* id : required) {
        expect(FindPack(id) != nullptr, id);
    }
    expect(FindPack("CUSTOM") == nullptr, "Custom is not a baked pack");
    expect(FindPack("Vigilant") == FindPack("vigilant"), "pack lookup is case-insensitive");

    std::set<std::string> ids;
    std::set<std::string> fingerprints;
    for (const auto& pack : kPlayStylePacks) {
        expect(ids.insert(pack.id).second, "pack ids are unique");
        const auto fp = std::to_string(pack.weights.quests) + "|" + std::to_string(pack.weights.discovery) + "|" +
                        std::to_string(pack.weights.clears) + "|" + std::to_string(pack.weights.combat) + "|" +
                        std::to_string(pack.weights.reading) + "|" + std::to_string(pack.weights.crafting) + "|" +
                        std::to_string(pack.weights.skillups);
        expect(fingerprints.insert(fp).second, "each pack has a distinct weight signature");
    }

    const auto* thief = FindPack("thief");
    const auto* warrior = FindPack("warrior");
    const auto* mage = FindPack("mage");
    const auto* adventurer = FindPack("adventurer");
    expect(thief && warrior && mage && adventurer, "core packs present");
    if (thief && warrior) {
        expect(thief->weights.discovery > warrior->weights.discovery, "Thief emphasizes discovery over Warrior");
        expect(warrior->weights.combat > thief->weights.combat, "Warrior emphasizes combat over Thief");
    }
    if (mage && adventurer) {
        expect(mage->weights.reading > adventurer->weights.reading, "Mage turns reading on vs Adventurer default");
        expect(mage->weights.skillups > adventurer->weights.skillups, "Mage turns training XP on");
    }
    if (const auto* vigilant = FindPack("vigilant")) {
        expect(vigilant->flavor.undeadCombatBonus > 0.0f, "Vigilant undead bonus is mechanical, not a label");
        expect(vigilant->questTypes.daedric == 0.0f, "Vigilant denies Daedric quest XP");
    }
    if (const auto* paladin = FindPack("paladin")) {
        expect(paladin->questTypes.daedric == 0.0f, "Paladin denies Daedric quest XP");
        expect(paladin->flavor.undeadCombatBonus > 0.0f, "Paladin undead bonus is mechanical");
    }
    if (const auto* assassin = FindPack("assassin")) {
        expect(assassin->flavor.stealthCombatBonus > 0.0f, "Assassin stealth bonus is mechanical");
    }
    if (const auto* beast = FindPack("beastblood")) {
        expect(beast->flavor.beastCombatBonus > 0.0f, "Beastblood hunt bonus is mechanical");
        expect(beast->weights.reading == 0.0f, "Beastblood turns reading off");
    }

    const float quest = 80.0f;
    expect(
        AdventureXP::Math::ApplyGlobalPercent(quest, thief ? thief->weights.quests : 0.0f) <
            AdventureXP::Math::ApplyGlobalPercent(quest, adventurer ? adventurer->weights.quests : 100.0f),
        "Thief quest slider actually reduces quest XP vs Adventurer");

    if (g_failed) {
        std::cerr << g_failed << " assertion(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "adventurexp_preset_tests: ok (" << kPlayStylePackCount << " packs, v" << kVersionString << ")\n";
    return EXIT_SUCCESS;
}
