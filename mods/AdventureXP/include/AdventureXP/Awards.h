#pragma once

#include "AdventureXP/Types.h"

#include <string_view>

namespace AdventureXP::Awards {

void Give(float amount, std::string_view reason);
// Reading and Combat: base * (categoryWeight/100). Other categories also
// multiply by (globalXPPercent/100).
float Scale(float base, Category category);
// floor(sqrt(max(0, goldValue)) * readingMult). Notes/letters with gold 0
// stay 0. BookSink passes this through Scale(..., Category::Reading).
float ReadingBaseXP(int goldValue, float readingMult);
float ThresholdForLevel(int level);

}  // namespace AdventureXP::Awards
