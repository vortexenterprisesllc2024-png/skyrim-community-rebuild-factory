#include "AdventureXP/Types.h"
#include "AdventureXP/Version.h"

#include <cassert>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

using namespace AdventureXP;

int main()
{
	assert(ADVENTUREXP_VERSION_MAJOR == 4);
	assert(ADVENTUREXP_VERSION_MINOR == 2);
	assert(ADVENTUREXP_VERSION_PATCH == 9);
	assert(std::string_view(ADVENTUREXP_VERSION_STRING) == "4.2.9");
	assert(kPlaceKeys.size() == static_cast<std::size_t>(PlaceKind::Count));
	assert(kQuestKeys.size() == static_cast<std::size_t>(QuestKind::Count));
	assert(kSkillKeys.size() == static_cast<std::size_t>(SkillKind::Count));
	assert(kPlaceKeys.size() == 39);
	assert(kQuestKeys.size() == 13);
	assert(kSkillKeys.size() == 18);

	for (std::size_t i = 0; i < kPlaceKeys.size(); ++i) {
		const auto parsed = ParsePlaceKind(kPlaceKeys[i]);
		assert(parsed.has_value());
		assert(static_cast<std::size_t>(*parsed) == i);
		assert(DefaultDiscoveryXP(*parsed) >= kPlaceXPMin);
		assert(DefaultDiscoveryXP(*parsed) <= kPlaceXPMax);
		assert(DefaultClearXP(*parsed) >= kPlaceXPMin);
		assert(DefaultClearXP(*parsed) <= kPlaceXPMax);
	}

	for (std::size_t i = 0; i < kQuestKeys.size(); ++i) {
		const auto parsed = ParseQuestKind(kQuestKeys[i]);
		assert(parsed.has_value());
		assert(static_cast<std::size_t>(*parsed) == i);
		assert(DefaultQuestXP(*parsed) >= kQuestXPMin);
		assert(DefaultQuestXP(*parsed) <= kQuestXPMax);
	}

	for (std::size_t i = 0; i < kSkillKeys.size(); ++i) {
		const auto parsed = ParseSkill(kSkillKeys[i]);
		assert(parsed.has_value());
		assert(static_cast<std::size_t>(*parsed) == i);
		assert(kDefaultSkillWeight[i] == 100.f);
	}

	assert(ParseSkill("one-handed") == SkillKind::OneHanded);
	assert(ParseSkill("two_handed") == SkillKind::TwoHanded);
	assert(ParseSkill("marksman") == SkillKind::Archery);
	assert(ParseSkill("heavy armor") == SkillKind::HeavyArmor);
	assert(ParseSkill("light_armor") == SkillKind::LightArmor);
	assert(ParseSkill("speechcraft") == SkillKind::Speech);
	assert(!ParseSkill("Health"));

	assert(SkillFromActorValue(6) == SkillKind::OneHanded);
	assert(SkillFromActorValue(18) == SkillKind::Alteration);
	assert(SkillFromActorValue(21) == SkillKind::Illusion);
	assert(SkillFromActorValue(23) == SkillKind::Enchanting);
	assert(!SkillFromActorValue(5));
	assert(!SkillFromActorValue(24));
	assert(SkillDataIndexFromActorValue(6) == 0);
	assert(SkillDataIndexFromActorValue(18) == 12);
	assert(SkillDataIndexFromActorValue(23) == 17);
	assert(!SkillDataIndexFromActorValue(5));
	assert(!SkillDataIndexFromActorValue(24));

	assert(ParsePlaceKind("Dungeon") == PlaceKind::Default);
	assert(ParsePlaceKind("Nordic") == PlaceKind::NordicRuin);
	assert(ParsePlaceKind("Dwemer") == PlaceKind::DwemerRuin);
	assert(ParsePlaceKind("Dragon") == PlaceKind::DragonLair);
	assert(ParseQuestKind("Other") == QuestKind::None);
	assert(!ParseQuestKind("Guild"));

	assert(QuestKindFromCKType(2) == QuestKind::College);
	assert(QuestKindFromCKType(3) == QuestKind::Thieves);
	assert(QuestKindFromCKType(4) == QuestKind::Brotherhood);
	assert(QuestKindFromCKType(5) == QuestKind::Companions);
	assert(QuestKindFromCKType(10) == QuestKind::Dawnguard);
	assert(QuestKindFromCKType(11) == QuestKind::Dragonborn);

	const std::vector<std::string_view> none;
	assert(ClassifyQuest(2, "RandomQuest", none) == QuestKind::College);
	assert(ClassifyQuest(0, "MGRitual01", none) == QuestKind::College);
	assert(ClassifyQuest(0, "TG00", none) == QuestKind::Thieves);
	assert(ClassifyQuest(0, "DB02", none) == QuestKind::Brotherhood);
	assert(ClassifyQuest(0, "C01", none) == QuestKind::Companions);
	assert(ClassifyQuest(0, "DLC1VQ01", none) == QuestKind::Dawnguard);
	assert(ClassifyQuest(0, "DLC2MQ01", none) == QuestKind::Dragonborn);
	assert(ClassifyQuest(0, "MQ101", none) == QuestKind::Main);

	const std::vector<std::string> collegeFac{"CollegeofWinterholdFaction"};
	assert(ClassifyQuest(0, "SomeModQuest", collegeFac) == QuestKind::College);

	const std::vector<std::string_view> cityKw{"LocTypeHold", "LocTypeCity"};
	assert(ClassifyPlace(cityKw, std::nullopt, "") == PlaceKind::City);

	const std::vector<std::string_view> caveKw{"LocTypeDungeon", "LocTypeCave"};
	assert(ClassifyPlace(caveKw, std::nullopt, "") == PlaceKind::Cave);

	const std::vector<std::string_view> emptyKw;
	assert(ClassifyPlace(emptyKw, static_cast<std::uint16_t>(MapMarker::Altar), "") == PlaceKind::Altar);
	assert(ClassifyPlace(emptyKw, static_cast<std::uint16_t>(MapMarker::ImperialTower), "") == PlaceKind::ImperialTower);
	assert(ClassifyPlace(emptyKw, static_cast<std::uint16_t>(MapMarker::CastleKarstaag), "") == PlaceKind::CastleKarstaag);
	assert(ClassifyPlace(emptyKw, static_cast<std::uint16_t>(MapMarker::AllMakerStone), "") == PlaceKind::Doomstone);
	assert(ClassifyPlace(emptyKw, static_cast<std::uint16_t>(MapMarker::Shrine), "") == PlaceKind::DaedricShrine);
	assert(ClassifyPlace(emptyKw, static_cast<std::uint16_t>(MapMarker::WhiterunCapitol), "") == PlaceKind::City);
	assert(ClassifyPlace(emptyKw, static_cast<std::uint16_t>(MapMarker::WhiterunCastle), "") == PlaceKind::Castle);

	assert(ClassifyPlace(emptyKw, std::nullopt, "WhiterunStable") == PlaceKind::Stable);
	assert(ClassifyPlace(emptyKw, std::nullopt, "KatariahSmelter") == PlaceKind::Smelter);
	assert(ClassifyPlace(emptyKw, std::nullopt, "RiverwoodWheatMill") == PlaceKind::WheatMill);
	assert(ClassifyPlace(emptyKw, std::nullopt, "BleakFallsWatchtower") == PlaceKind::NordicTower);
	assert(ClassifyPlace(emptyKw, std::nullopt, "DoomstoneGuardian") == PlaceKind::Doomstone);

	// Keyword wins over generic marker when both exist.
	assert(ClassifyPlace(cityKw, static_cast<std::uint16_t>(MapMarker::Town), "") == PlaceKind::City);

	std::cout << "host_types: ok (" << kPlaceKeys.size() << " places, " << kQuestKeys.size()
			  << " quest kinds, " << kSkillKeys.size() << " skills)\n";
	return 0;
}
