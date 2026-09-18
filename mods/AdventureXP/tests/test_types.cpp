#include "AdventureXP/Types.h"
#include "AdventureXP/XpMath.h"

#include <cstdlib>
#include <iostream>

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

    void expect_near(float got, float want, float eps, const char* what)
    {
        const float delta = got > want ? got - want : want - got;
        if (delta > eps) {
            std::cerr << "FAIL: " << what << " got " << got << " want " << want << "\n";
            ++g_failed;
        }
    }
}

int main()
{
    using namespace AdventureXP;

    expect(ClassifyQuestType(1) == QuestKind::Main, "type 1 is main");
    expect(ClassifyQuestType(3) == QuestKind::Guild, "thieves guild is guild");
    expect(ClassifyQuestType(7) == QuestKind::Daedric, "type 7 is daedric");
    expect(ClassifyQuestType(6) == QuestKind::Misc, "type 6 is misc");
    expect(ClassifyQuestType(10) == QuestKind::DLC, "Dawnguard type is DLC");

    QuestTypeWeights q{};
    q.daedric = 130.0f;
    q.misc = 40.0f;
    expect_near(WeightFor(QuestKind::Daedric, q), 130.0f, 0.01f, "daedric slider");
    expect_near(WeightFor(QuestKind::Hidden, q), 0.0f, 0.01f, "hidden quests grant nothing");

    expect(ClassifyPlaceKeyword("LocTypeDragonLair") == PlaceKind::Dragon, "dragon lair");
    expect(ClassifyPlaceKeyword("LocTypeNordicRuin") == PlaceKind::Nordic, "nordic");
    expect(ClassifyPlaceKeyword("LocTypeCity") == PlaceKind::City, "city");
    expect(ClassifyPlaceKeyword("LocTypeDwemerRuin") == PlaceKind::Dwemer, "dwemer substring");
    expect(Stronger(PlaceKind::City, PlaceKind::Dungeon) == PlaceKind::Dungeon, "dungeon beats city");
    expect(Stronger(PlaceKind::Dragon, PlaceKind::Dungeon) == PlaceKind::Dragon, "dragon beats dungeon");

    PlaceTypeWeights p{};
    p.city = 40.0f;
    p.dragon = 140.0f;
    const float cityAward = Math::ApplyGlobalPercent(30.0f, WeightFor(PlaceKind::City, p));
    const float dragonAward = Math::ApplyGlobalPercent(30.0f, WeightFor(PlaceKind::Dragon, p));
    expect(cityAward < 30.0f && dragonAward > 30.0f, "place sliders change discovery XP");

    if (g_failed) {
        std::cerr << g_failed << " assertion(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "adventurexp_type_tests: ok\n";
    return EXIT_SUCCESS;
}
