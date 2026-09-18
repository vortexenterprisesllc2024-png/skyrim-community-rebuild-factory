#pragma once

// Engine-free XP helpers. Safe to compile on any host for unit tests.
// The live plugin still writes into vanilla PlayerSkills so the stock
// level-up / perk-point flow can run.

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace AdventureXP::Math
{
    struct Curve
    {
        float base{ 200.0f };
        float perLevel{ 50.0f };
        float exponent{ 1.0f };
        std::int32_t maxLevel{ 81 };
    };

    inline float ClampPercent(float percent)
    {
        return std::clamp(percent, 0.0f, 1000.0f);
    }

    inline float ApplyGlobalPercent(float rawXP, float globalPercent)
    {
        return rawXP * (ClampPercent(globalPercent) / 100.0f);
    }

    // Independent curve used only when the plugin cannot read the live
    // vanilla threshold (shouldn't happen in-game). EverQuest-style bump:
    // base + perLevel * (level-1)^exponent.
    inline float FallbackThresholdForLevel(std::int32_t level, const Curve& curve)
    {
        const std::int32_t n = (std::max)(1, level);
        return curve.base + curve.perLevel * std::pow(static_cast<float>(n - 1), curve.exponent);
    }

    inline float ProgressPercent(float currentXP, float threshold)
    {
        if (threshold <= 0.0f) {
            return 0.0f;
        }
        return std::clamp((currentXP / threshold) * 100.0f, 0.0f, 100.0f);
    }

    inline bool WouldLevel(float currentXP, float award, float threshold)
    {
        return threshold > 0.0f && (currentXP + award) >= threshold;
    }
}
