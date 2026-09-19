#include "AdventureXP/Config.h"
#include "AdventureXP/Presets.h"

#include <cassert>
#include <filesystem>
#include <iostream>

int main()
{
	using namespace AdventureXP;

	assert(kPresets.size() == 20);
	assert(FindPreset("Summoner") != FindPreset("Necromancer"));
	assert(FindPreset("Conjurer") == FindPreset("Summoner"));
	assert(FindPreset("Illusionist") != nullptr);
	assert(FindPreset("elementalist") != nullptr);
	assert(FindPreset("DestructionMage") == FindPreset("elementalist"));
	assert(FindPreset("Battlemage") != nullptr);
	assert(FindPreset("Crafter") != nullptr);
	assert(FindPreset("Wizard") != nullptr);
	assert(kWizard.categories.reading == 100.f);
	assert(kWizard.categories.combat == 10.f);
	assert(kWizard.skills[SkillKind::Alteration] == 100.f);
	assert(kWizard.skills[SkillKind::Enchanting] == 100.f);
	assert(kWizard.skills[SkillKind::Alchemy] == 0.f);
	assert(kWizard.skills[SkillKind::Speech] == 0.f);
	assert(kWizard.skills[SkillKind::OneHanded] == 0.f);
	assert(kMage.skills[SkillKind::Alchemy] == 80.f);
	assert(kCrafter.categories.crafting == 100.f);
	assert(kCrafter.categories.combat == 15.f);
	assert(kCrafter.skills[SkillKind::Smithing] == 100.f);
	assert(kCrafter.skills[SkillKind::Alchemy] == 100.f);
	assert(kCrafter.skills[SkillKind::Enchanting] == 100.f);
	assert(kCrafter.skills[SkillKind::OneHanded] == 0.f);
	assert(FindPreset("no-such-pack") == nullptr);
	assert(kVigilant.quests.daedric == 0);
	assert(kPaladin.quests.daedric == 0);
	assert(kPaladin.quests.thieves == 0);
	assert(kPaladin.quests.brotherhood == 0);
	assert(kMerchant.categories.combat == 0.f);
	assert(kIllusionist.skills[SkillKind::Illusion] == 100.f);
	assert(kIllusionist.skills[SkillKind::Speech] == 50.f);
	assert(kIllusionist.skills[SkillKind::Alteration] == 0.f);
	assert(kIllusionist.skills[SkillKind::Conjuration] == 0.f);
	assert(kIllusionist.skills[SkillKind::Destruction] == 0.f);
	assert(kIllusionist.skills[SkillKind::Restoration] == 0.f);
	assert(kIllusionist.skills[SkillKind::Enchanting] == 0.f);
	assert(kIllusionist.skills[SkillKind::OneHanded] == 0.f);
	assert(kIllusionist.skills[SkillKind::TwoHanded] == 0.f);
	assert(kIllusionist.skills[SkillKind::Archery] == 0.f);
	assert(kIllusionist.skills[SkillKind::Block] == 0.f);
	assert(kIllusionist.skills[SkillKind::HeavyArmor] == 0.f);
	assert(kIllusionist.skills[SkillKind::Smithing] == 0.f);
	assert(kElementalist.skills[SkillKind::Destruction] == 100.f);
	assert(kElementalist.skills[SkillKind::Illusion] == 0.f);
	assert(kSummoner.skills[SkillKind::Conjuration] == 100.f);

	const auto dir = std::filesystem::temp_directory_path() / "adventurexp-host-presets";
	std::filesystem::create_directories(dir);
	const auto path = dir / "AdventureXP.ini";
	std::filesystem::remove(path);

	auto& cfg = Config::Get();
	cfg.Load(path);

	assert(cfg.ApplyPreset("Adventurer"));
	assert(cfg.CategoryWeight(Category::Quest) == 100.f);
	assert(cfg.CategoryWeight(Category::Combat) == 0.f);
	assert(cfg.QuestXP(QuestKind::Daedric) == DefaultQuestXP(QuestKind::Daedric));
	assert(cfg.DiscoveryXP(PlaceKind::City) == DefaultDiscoveryXP(PlaceKind::City));
	assert(!cfg.awardKilling);

	assert(cfg.ApplyPreset("Vigilant"));
	assert(cfg.QuestXP(QuestKind::Daedric) == 0);
	assert(cfg.QuestXP(QuestKind::Thieves) <= ScaledQuestXP(DefaultQuestXP(QuestKind::Thieves), 10));
	assert(cfg.QuestXP(QuestKind::Brotherhood) <= ScaledQuestXP(DefaultQuestXP(QuestKind::Brotherhood), 5));
	assert(cfg.QuestXP(QuestKind::Main) == DefaultQuestXP(QuestKind::Main));
	assert(cfg.QuestXP(QuestKind::Dawnguard) == DefaultQuestXP(QuestKind::Dawnguard));
	assert(cfg.CategoryWeight(Category::Clear) == 100.f);
	assert(cfg.CategoryWeight(Category::Clear) <= 100.f);
	assert(cfg.undeadCombatBonus >= 50.f);
	assert(cfg.DiscoveryXP(PlaceKind::DaedricShrine) == 0);
	assert(cfg.ClearXP(PlaceKind::DaedricShrine) == 0);
	assert(cfg.mainQuestMultiplier > 1.0f);
	assert(cfg.awardReading);
	assert(cfg.awardKilling);

	assert(cfg.ApplyPreset("Thief"));
	assert(cfg.preset == "Thief");
	assert(cfg.CategoryWeight(Category::Discovery) == 100.f);
	assert(cfg.CategoryWeight(Category::Clear) <= 40.f);
	assert(cfg.QuestXP(QuestKind::Thieves) == DefaultQuestXP(QuestKind::Thieves));
	assert(cfg.QuestXP(QuestKind::Main) < DefaultQuestXP(QuestKind::Main));
	assert(cfg.QuestXP(QuestKind::CivilWar) <= ScaledQuestXP(DefaultQuestXP(QuestKind::CivilWar), 15));
	assert(cfg.stealthCombatBonus >= 40.f);

	assert(cfg.ApplyPreset("Paladin"));
	assert(cfg.QuestXP(QuestKind::Daedric) == 0);
	assert(cfg.QuestXP(QuestKind::Thieves) == 0);
	assert(cfg.QuestXP(QuestKind::Brotherhood) == 0);
	assert(cfg.DiscoveryXP(PlaceKind::DaedricShrine) == 0);
	assert(cfg.ClearXP(PlaceKind::DaedricShrine) == 0);
	assert(cfg.undeadCombatBonus >= 50.f);
	assert(cfg.CategoryWeight(Category::Quest) <= 100.f);

	assert(cfg.ApplyPreset("Merchant"));
	assert(cfg.CategoryWeight(Category::Combat) == 0.f);
	assert(cfg.CategoryWeight(Category::Clear) == 0.f);
	assert(cfg.QuestXP(QuestKind::Misc) == DefaultQuestXP(QuestKind::Misc));
	assert(cfg.ClearXP(PlaceKind::Cave) == 0);
	assert(!cfg.awardKilling);

	assert(cfg.ApplyPreset("Conjurer"));
	assert(cfg.preset == "Summoner");
	assert(cfg.QuestXP(QuestKind::College) == DefaultQuestXP(QuestKind::College));
	assert(cfg.CategoryWeight(Category::Combat) <= 25.f);
	assert(cfg.CategoryWeight(Category::Reading) == 100.f);

	assert(cfg.ApplyPreset("Necromancer"));
	assert(cfg.preset == "Necromancer");
	assert(cfg.CategoryWeight(Category::Clear) == 100.f);
	assert(cfg.QuestXP(QuestKind::Daedric) >= ScaledQuestXP(DefaultQuestXP(QuestKind::Daedric), 80));
	assert(cfg.undeadCombatBonus >= 40.f);

	assert(cfg.ApplyPreset("Beastblood"));
	assert(cfg.skipMiscQuests);
	assert(cfg.beastCombatBonus >= 60.f);
	assert(cfg.CategoryWeight(Category::Reading) == 0.f);

	assert(cfg.ApplyPreset("Assassin"));
	assert(cfg.QuestXP(QuestKind::Brotherhood) == DefaultQuestXP(QuestKind::Brotherhood));
	assert(cfg.CategoryWeight(Category::Reading) == 0.f);
	assert(cfg.CategoryWeight(Category::Crafting) == 0.f);
	assert(cfg.stealthCombatBonus >= 60.f);

	assert(cfg.ApplyPreset("Monk"));
	assert(cfg.QuestXP(QuestKind::Thieves) <= ScaledQuestXP(DefaultQuestXP(QuestKind::Thieves), 10));
	assert(cfg.QuestXP(QuestKind::Brotherhood) <= ScaledQuestXP(DefaultQuestXP(QuestKind::Brotherhood), 5));
	assert(cfg.CategoryWeight(Category::Crafting) == 0.f);
	assert(cfg.CategoryWeight(Category::SkillUp) == 100.f);

	assert(cfg.ApplyPreset("Illusionist"));
	assert(cfg.preset == "Illusionist");
	assert(cfg.SkillWeight(SkillKind::Illusion) == 100.f);
	assert(cfg.SkillWeight(SkillKind::Speech) == 50.f);
	assert(cfg.SkillWeight(SkillKind::Alteration) == 0.f);
	assert(cfg.SkillWeight(SkillKind::Conjuration) == 0.f);
	assert(cfg.SkillWeight(SkillKind::Destruction) == 0.f);
	assert(cfg.SkillWeight(SkillKind::Restoration) == 0.f);
	assert(cfg.SkillWeight(SkillKind::Enchanting) == 0.f);
	assert(cfg.SkillWeight(SkillKind::OneHanded) == 0.f);
	assert(cfg.SkillWeight(SkillKind::TwoHanded) == 0.f);
	assert(cfg.CategoryWeight(Category::SkillUp) == 100.f);
	assert(cfg.QuestXP(QuestKind::Daedric) == ScaledQuestXP(DefaultQuestXP(QuestKind::Daedric), 40));

	assert(cfg.ApplyPreset("Vigilant"));
	assert(cfg.QuestXP(QuestKind::Daedric) == 0);

	assert(cfg.ApplyPreset("elementalist"));
	assert(cfg.SkillWeight(SkillKind::Destruction) == 100.f);
	assert(cfg.SkillWeight(SkillKind::Illusion) == 0.f);

	assert(cfg.ApplyPreset("Battlemage"));
	assert(cfg.SkillWeight(SkillKind::Destruction) == 100.f);
	assert(cfg.SkillWeight(SkillKind::Conjuration) == 100.f);
	assert(cfg.SkillWeight(SkillKind::OneHanded) == 80.f);
	assert(cfg.SkillWeight(SkillKind::HeavyArmor) == 70.f);
	assert(cfg.SkillWeight(SkillKind::Sneak) == 0.f);

	for (const auto& pack : kPresets) {
		assert(CategoriesCapped(pack.categories));
		cfg.ApplyPack(pack);
		for (std::size_t i = 0; i < 7; ++i) {
			assert(cfg.CategoryWeight(static_cast<Category>(i)) <= 100.f);
		}
	}

	assert(cfg.ApplyPreset("Crafter"));
	assert(cfg.preset == "Crafter");
	assert(cfg.CategoryWeight(Category::Crafting) == 100.f);
	assert(cfg.CategoryWeight(Category::Combat) == 15.f);
	assert(cfg.CategoryWeight(Category::Clear) == 25.f);
	assert(cfg.SkillWeight(SkillKind::Smithing) == 100.f);
	assert(cfg.SkillWeight(SkillKind::Alchemy) == 100.f);
	assert(cfg.SkillWeight(SkillKind::Enchanting) == 100.f);
	assert(cfg.SkillWeight(SkillKind::OneHanded) == 0.f);
	assert(cfg.awardKilling);  // combat > 0 enables killing like other low-combat packs

	assert(cfg.ApplyPreset("Wizard"));
	assert(cfg.preset == "Wizard");
	assert(cfg.CategoryWeight(Category::Reading) == 100.f);
	assert(cfg.CategoryWeight(Category::SkillUp) == 100.f);
	assert(cfg.CategoryWeight(Category::Quest) == 90.f);
	assert(cfg.CategoryWeight(Category::Discovery) == 70.f);
	assert(cfg.CategoryWeight(Category::Crafting) == 20.f);
	assert(cfg.CategoryWeight(Category::Combat) == 10.f);
	assert(cfg.CategoryWeight(Category::Clear) == 20.f);
	assert(cfg.SkillWeight(SkillKind::Alteration) == 100.f);
	assert(cfg.SkillWeight(SkillKind::Conjuration) == 100.f);
	assert(cfg.SkillWeight(SkillKind::Destruction) == 100.f);
	assert(cfg.SkillWeight(SkillKind::Illusion) == 100.f);
	assert(cfg.SkillWeight(SkillKind::Restoration) == 100.f);
	assert(cfg.SkillWeight(SkillKind::Enchanting) == 100.f);
	assert(cfg.SkillWeight(SkillKind::Alchemy) == 0.f);
	assert(cfg.SkillWeight(SkillKind::Speech) == 0.f);
	assert(cfg.SkillWeight(SkillKind::Smithing) == 0.f);
	assert(cfg.SkillWeight(SkillKind::OneHanded) == 0.f);
	assert(cfg.SkillWeight(SkillKind::Sneak) == 0.f);
	assert(cfg.awardKilling);

	assert(cfg.ApplyPreset("Mage"));
	assert(cfg.SkillWeight(SkillKind::Alchemy) == 80.f);
	assert(cfg.CategoryWeight(Category::Crafting) == 100.f);

	std::cout << "host_presets: ok\n";
	return 0;
}
