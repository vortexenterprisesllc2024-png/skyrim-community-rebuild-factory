#include "AdventureXP/Awards.h"
#include "AdventureXP/Config.h"

#include <cassert>
#include <cmath>
#include <iostream>

namespace {

bool Near(float a, float b)
{
	return std::fabs(a - b) < 1e-4f;
}

void ResetScaleState()
{
	auto& cfg = AdventureXP::Config::Get();
	cfg.enabled = true;
	cfg.awardKilling = true;
	cfg.awardReading = true;
	cfg.globalXPPercent = 2.f;
	for (int i = 0; i < static_cast<int>(AdventureXP::Category::Count); ++i) {
		cfg.SetCategoryWeight(static_cast<AdventureXP::Category>(i), 100.f);
	}
}

}  // namespace

int main()
{
	using namespace AdventureXP;

	ResetScaleState();
	auto& cfg = Config::Get();

	// Jo: fGlobalXPPercent=2, reading at 100%. Old Scale: 5 * 1.0 * 0.02 = 0.1
	// (+%.0f toast = +0). Reading/Combat must ignore global.
	assert(Near(Awards::Scale(5.f, Category::Reading), 5.f));
	assert(Near(Awards::Scale(cfg.readingXP, Category::Reading), 5.f));
	assert(Near(Awards::Scale(10.f, Category::Combat), 10.f));
	assert(Near(Awards::Scale(cfg.killXP, Category::Combat), 2.f));
	assert(Near(Awards::Scale(cfg.bossKillXP, Category::Combat), 25.f));

	// Category weight still scales Reading and Combat.
	cfg.SetCategoryWeight(Category::Reading, 50.f);
	cfg.SetCategoryWeight(Category::Combat, 40.f);
	assert(Near(Awards::Scale(5.f, Category::Reading), 2.5f));
	assert(Near(Awards::Scale(10.f, Category::Combat), 4.f));
	cfg.SetCategoryWeight(Category::Reading, 100.f);
	cfg.SetCategoryWeight(Category::Combat, 100.f);

	// Quests, discovery, clears, skill-ups, crafting keep global.
	const float withGlobal = 100.f * 1.f * (2.f / 100.f);
	assert(Near(Awards::Scale(100.f, Category::Quest), withGlobal));
	assert(Near(Awards::Scale(100.f, Category::Discovery), withGlobal));
	assert(Near(Awards::Scale(100.f, Category::Clear), withGlobal));
	assert(Near(Awards::Scale(100.f, Category::SkillUp), withGlobal));
	assert(Near(Awards::Scale(100.f, Category::Crafting), withGlobal));

	// Global 100% still multiplies those categories.
	cfg.globalXPPercent = 100.f;
	assert(Near(Awards::Scale(100.f, Category::Quest), 100.f));
	assert(Near(Awards::Scale(5.f, Category::Reading), 5.f));
	assert(Near(Awards::Scale(10.f, Category::Combat), 10.f));

	// Enable flags still gate Reading/Combat.
	cfg.globalXPPercent = 2.f;
	cfg.awardReading = false;
	assert(Near(Awards::Scale(5.f, Category::Reading), 0.f));
	cfg.awardReading = true;
	cfg.awardKilling = false;
	assert(Near(Awards::Scale(10.f, Category::Combat), 0.f));
	cfg.awardKilling = true;
	assert(Near(Awards::Scale(5.f, Category::Reading), 5.f));
	assert(Near(Awards::Scale(10.f, Category::Combat), 10.f));

	// Reading XP = floor(sqrt(gold) * fReadingMult); then Scale (still ignores global).
	assert(Near(Awards::ReadingBaseXP(0, 1.f), 0.f));
	assert(Near(Awards::ReadingBaseXP(-12, 1.f), 0.f));
	assert(Near(Awards::ReadingBaseXP(69, 1.f), 8.f));    // Sparks
	assert(Near(Awards::ReadingBaseXP(240, 1.f), 15.f));  // Turn Lesser Undead
	assert(Near(Awards::ReadingBaseXP(1769, 1.f), 42.f)); // Repel Undead
	assert(Near(Awards::ReadingBaseXP(69, 2.f), 16.f));
	cfg.globalXPPercent = 2.f;
	cfg.readingMult = 1.f;
	assert(Near(Awards::Scale(Awards::ReadingBaseXP(69, cfg.readingMult), Category::Reading), 8.f));
	assert(Near(Awards::Scale(Awards::ReadingBaseXP(240, cfg.readingMult), Category::Reading), 15.f));
	assert(Near(Awards::Scale(Awards::ReadingBaseXP(1769, cfg.readingMult), Category::Reading), 42.f));
	assert(Near(Awards::Scale(Awards::ReadingBaseXP(0, cfg.readingMult), Category::Reading), 0.f));
	cfg.SetCategoryWeight(Category::Reading, 50.f);
	assert(Near(Awards::Scale(Awards::ReadingBaseXP(69, 1.f), Category::Reading), 4.f));
	cfg.SetCategoryWeight(Category::Reading, 100.f);
	// Flat fReadingXP is leftover only; awards use ReadingBaseXP, not readingXP.
	assert(Near(cfg.readingXP, 5.f));
	assert(!Near(Awards::ReadingBaseXP(69, 1.f), cfg.readingXP));

	std::cout << "host_awards: Scale ignores global for Reading and Combat only\n";
	std::cout << "host_awards: ReadingBaseXP sqrt(gold)*fReadingMult\n";
	return 0;
}
