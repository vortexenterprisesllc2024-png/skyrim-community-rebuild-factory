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
    using namespace AdventureXP::Math;

    expect_near(ApplyGlobalPercent(100.0f, 100.0f), 100.0f, 0.001f, "100% leaves XP unchanged");
    expect_near(ApplyGlobalPercent(80.0f, 50.0f), 40.0f, 0.001f, "50% halves XP");
    expect_near(ApplyGlobalPercent(10.0f, 200.0f), 20.0f, 0.001f, "200% doubles XP");
    expect_near(ApplyGlobalPercent(10.0f, -5.0f), 0.0f, 0.001f, "negative percent clamps to 0");

    Curve curve{};
    expect_near(FallbackThresholdForLevel(1, curve), 200.0f, 0.001f, "level 1 threshold is base");
    expect_near(FallbackThresholdForLevel(2, curve), 250.0f, 0.001f, "level 2 adds perLevel");
    expect_near(FallbackThresholdForLevel(0, curve), 200.0f, 0.001f, "level 0 treated as 1");

    expect_near(ProgressPercent(25.0f, 100.0f), 25.0f, 0.001f, "quarter bar");
    expect_near(ProgressPercent(0.0f, 0.0f), 0.0f, 0.001f, "zero threshold");
    expect_near(ProgressPercent(200.0f, 100.0f), 100.0f, 0.001f, "over-full clamps");

    expect(WouldLevel(90.0f, 15.0f, 100.0f), "90+15 crosses 100");
    expect(!WouldLevel(90.0f, 5.0f, 100.0f), "90+5 stays under 100");

    if (g_failed) {
        std::cerr << g_failed << " assertion(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "adventurexp_math_tests: ok\n";
    return EXIT_SUCCESS;
}
