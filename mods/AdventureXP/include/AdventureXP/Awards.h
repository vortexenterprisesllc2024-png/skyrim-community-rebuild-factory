#pragma once

#include "AdventureXP/Types.h"

#include <string_view>

namespace AdventureXP::Awards {

void Give(float amount, std::string_view reason);
void SyncFromPlayer();
float Scale(float base, Category category);
float ThresholdForLevel(int level);

}  // namespace AdventureXP::Awards
