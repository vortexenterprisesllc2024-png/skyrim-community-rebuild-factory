#pragma once

#include "AdventureXP/Types.h"

#include <string_view>

namespace AdventureXP::Awards {

void Give(float amount, std::string_view reason);
// Quest, Reading, Combat, and SkillUp: base * (categoryWeight/100). Other
// categories also multiply by (globalXPPercent/100).
float Scale(float base, Category category);
// floor(sqrt(max(0, goldValue)) * readingMult). Notes/letters with gold 0
// stay 0. BookSink passes this through Scale(..., Category::Reading).
float ReadingBaseXP(int goldValue, float readingMult);
// fSkillUpXP * max(1, skillLevel) / max(1, fSkillUpLevelScale).
// SkillSink multiplies by (SkillWeight/100) then Scale(..., SkillUp).
float SkillUpBaseXP(float skillUpXP, int skillLevel, float levelScale);
float ThresholdForLevel(int level);

}  // namespace AdventureXP::Awards
