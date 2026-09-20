#include "AdventureXP/Awards.h"
#include "AdventureXP/Config.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

int main()
{
	using namespace AdventureXP;

	const auto dir = std::filesystem::temp_directory_path() / "adventurexp-host-config";
	std::filesystem::create_directories(dir);
	const auto path = dir / "AdventureXP.ini";
	std::filesystem::remove(path);

	auto& cfg = Config::Get();
	cfg.Load(path);
	assert(cfg.enabled);
	assert(cfg.QuestXP(QuestKind::Main) == DefaultQuestXP(QuestKind::Main));
	assert(cfg.DiscoveryXP(PlaceKind::City) == DefaultDiscoveryXP(PlaceKind::City));
	assert(cfg.ClearXP(PlaceKind::Cave) == DefaultClearXP(PlaceKind::Cave));
	assert(cfg.SkillWeight(SkillKind::Illusion) == 100.f);
	assert(cfg.SkillWeight(SkillKind::Sneak) == 100.f);
	assert(cfg.readingMult == 1.f);
	assert(cfg.skillUpLevelScale == 10.f);
	assert(cfg.awardQuestStages);
	assert(cfg.awardQuestComplete);
	assert(!cfg.awardSilentQuestStages);

	cfg.SetQuestXP(QuestKind::College, 77);
	cfg.SetDiscoveryXP(PlaceKind::Mine, 41);
	cfg.SetClearXP(PlaceKind::Mine, 88);
	cfg.SetCategoryWeight(Category::Combat, 55.f);
	cfg.SetSkillWeight(SkillKind::Illusion, 25.f);
	cfg.SetSkillWeight(SkillKind::Sneak, 0.f);
	cfg.awardKilling = true;
	cfg.awardSilentQuestStages = true;
	cfg.readingMult = 1.5f;
	cfg.skillUpLevelScale = 25.f;
	cfg.Save();

	Config again;
	again.Load(path);
	assert(again.QuestXP(QuestKind::College) == 77);
	assert(again.DiscoveryXP(PlaceKind::Mine) == 41);
	assert(again.ClearXP(PlaceKind::Mine) == 88);
	assert(again.CategoryWeight(Category::Combat) == 55.f);
	assert(again.SkillWeight(SkillKind::Illusion) == 25.f);
	assert(again.SkillWeight(SkillKind::Sneak) == 0.f);
	assert(again.SkillWeight(SkillKind::Destruction) == 100.f);
	assert(again.readingMult == 1.5f);
	assert(again.skillUpLevelScale == 25.f);
	assert(again.awardSilentQuestStages);

	assert(Awards::Scale(100.f, Category::Quest) >= 0.f);
	cfg.enabled = false;
	assert(Awards::Scale(100.f, Category::Quest) == 0.f);
	cfg.enabled = true;
	cfg.awardKilling = false;
	cfg.SetCategoryWeight(Category::Combat, 100.f);
	assert(Awards::Scale(10.f, Category::Combat) == 0.f);

	cfg.SetCategoryWeight(Category::SkillUp, 0.f);
	cfg.SetSkillWeight(SkillKind::Illusion, 100.f);
	assert(Awards::Scale(cfg.skillUpXP, Category::SkillUp) == 0.f);
	cfg.SetCategoryWeight(Category::SkillUp, 100.f);
	assert(Awards::Scale(cfg.skillUpXP * (cfg.SkillWeight(SkillKind::Illusion) / 100.f), Category::SkillUp) > 0.f);

	assert(cfg.ApplyPreset("Thief"));
	assert(cfg.preset == "Thief");
	assert(cfg.CategoryWeight(Category::Discovery) == 100.f);
	assert(cfg.QuestXP(QuestKind::Thieves) == DefaultQuestXP(QuestKind::Thieves));
	assert(cfg.SkillWeight(SkillKind::Sneak) == 100.f);
	assert(cfg.SkillWeight(SkillKind::Lockpicking) == 100.f);
	assert(cfg.SkillWeight(SkillKind::Pickpocket) == 100.f);
	assert(cfg.SkillWeight(SkillKind::Speech) == 100.f);
	assert(cfg.SkillWeight(SkillKind::OneHanded) == 10.f);

	std::ifstream in(path);
	std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	assert(text.find("iCollege=77") != std::string::npos || text.find("[QuestXP]") != std::string::npos);
	assert(text.find("iMine=") != std::string::npos);

	std::cout << "host_config: ok\n";
	return 0;
}
