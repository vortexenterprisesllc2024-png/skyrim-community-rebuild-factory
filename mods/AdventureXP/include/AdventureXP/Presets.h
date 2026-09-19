#pragma once

// Taquitos PlayStylePack, extended to the full quest/place lists.
// Weights are 0–100 RP style. Deny zeros stay zeros. ApplyPreset copies
// categories + quests + places + flavor.

#include "AdventureXP/Types.h"

#include <array>
#include <string_view>

namespace AdventureXP {

struct CategoryWeights {
	float quest = 100.f;
	float discovery = 100.f;
	float clear = 100.f;
	float combat = 0.f;
	float reading = 0.f;
	float crafting = 0.f;
	float skillup = 0.f;

	constexpr std::array<float, 7> ToArray() const
	{
		return {quest, discovery, clear, combat, reading, crafting, skillup};
	}
};

struct QuestTypeWeights {
	int objectives = 100;
	int none = 100;
	int main = 100;
	int college = 100;
	int thieves = 100;
	int brotherhood = 100;
	int companions = 100;
	int misc = 100;
	int daedric = 100;
	int side = 100;
	int civilWar = 100;
	int dawnguard = 100;
	int dragonborn = 100;

	constexpr std::array<int, 13> ToArray() const
	{
		return {
			objectives,
			none,
			main,
			college,
			thieves,
			brotherhood,
			companions,
			misc,
			daedric,
			side,
			civilWar,
			dawnguard,
			dragonborn,
		};
	}
};

constexpr std::array<int, 39> FillPlaces(int value)
{
	std::array<int, 39> out{};
	for (auto& item : out) {
		item = value;
	}
	return out;
}

constexpr std::array<int, 39> Bias(int fill, PlaceKind kind, int scale)
{
	auto out = FillPlaces(fill);
	out[static_cast<std::size_t>(kind)] = scale;
	return out;
}

template <class... Rest>
constexpr std::array<int, 39> Bias(int fill, PlaceKind kind, int scale, Rest... rest)
{
	auto out = Bias(fill, rest...);
	out[static_cast<std::size_t>(kind)] = scale;
	return out;
}

struct PlaceTypeWeights {
	std::array<int, 39> discovery = FillPlaces(100);
	std::array<int, 39> clear = FillPlaces(100);
};

struct PackFlavor {
	float undead = 0.f;
	float stealth = 0.f;
	float beast = 0.f;
	bool skipMisc = false;
	float mainQuestMultiplier = 1.0f;
};

struct SkillWeights {
	std::array<float, 18> value = kDefaultSkillWeight;

	constexpr float operator[](SkillKind kind) const
	{
		return value[static_cast<std::size_t>(kind)];
	}
};

constexpr SkillWeights FillSkills(float value)
{
	SkillWeights out{};
	for (auto& item : out.value) {
		item = value;
	}
	return out;
}

constexpr SkillWeights BiasSkills(float fill, SkillKind kind, float scale)
{
	auto out = FillSkills(fill);
	out.value[static_cast<std::size_t>(kind)] = scale;
	return out;
}

template <class... Rest>
constexpr SkillWeights BiasSkills(float fill, SkillKind kind, float scale, Rest... rest)
{
	auto out = BiasSkills(fill, rest...);
	out.value[static_cast<std::size_t>(kind)] = scale;
	return out;
}

struct PlayStylePack {
	const char* id;
	const char* name;
	CategoryWeights categories;
	QuestTypeWeights quests;
	PlaceTypeWeights places;
	PackFlavor flavor;
	SkillWeights skills{};
};

using Preset = PlayStylePack;

constexpr bool CategoriesCapped(const CategoryWeights& weights)
{
	for (const auto value : weights.ToArray()) {
		if (value < 0.f || value > 100.f) {
			return false;
		}
	}
	return true;
}

// Adventurer — pays: quests, discovery, clears at stock 100. Denies: combat,
// reading, crafting, skill-ups (0). EverQuest-style default.
inline constexpr PlayStylePack kAdventurer{
	"Adventurer",
	"Adventurer",
	{.quest = 100, .discovery = 100, .clear = 100, .combat = 0, .reading = 0, .crafting = 0, .skillup = 0},
	{},
	{},
	{},
};

// Vigilant — pays: barrow clears, undead, scripture, Main, Dawnguard.
// Denies: Daedric=0, Thieves/Brotherhood low, Misc low.
inline constexpr PlayStylePack kVigilant{
	"Vigilant",
	"Vigilant",
	{.quest = 90, .discovery = 70, .clear = 100, .combat = 80, .reading = 80, .crafting = 0, .skillup = 20},
	{
		.objectives = 80,
		.none = 50,
		.main = 100,
		.college = 60,
		.thieves = 10,
		.brotherhood = 5,
		.companions = 80,
		.misc = 25,
		.daedric = 0,
		.side = 90,
		.civilWar = 50,
		.dawnguard = 100,
		.dragonborn = 60,
	},
	{
		Bias(70,
			PlaceKind::Altar, 100,
			PlaceKind::StandingStone, 100,
			PlaceKind::Doomstone, 60,
			PlaceKind::DaedricShrine, 0,
			PlaceKind::NordicRuin, 90,
			PlaceKind::Cave, 90,
			PlaceKind::Default, 80,
			PlaceKind::City, 50,
			PlaceKind::Town, 50),
		Bias(100,
			PlaceKind::NordicRuin, 100,
			PlaceKind::Cave, 100,
			PlaceKind::Default, 100,
			PlaceKind::NordicDwelling, 100,
			PlaceKind::NordicTower, 100,
			PlaceKind::Fort, 90,
			PlaceKind::DaedricShrine, 0,
			PlaceKind::Camp, 50),
	},
	{.undead = 60, .stealth = 0, .beast = 0, .skipMisc = false, .mainQuestMultiplier = 1.5f},
	BiasSkills(15,
		SkillKind::Restoration, 90,
		SkillKind::OneHanded, 45,
		SkillKind::Block, 45,
		SkillKind::HeavyArmor, 40,
		SkillKind::Speech, 30),
};

// Summoner — display name Conjurer (old summoner id). Pays: College, reading,
// school ranks. Denies: combat (you don't swing), fortress clears.
inline constexpr PlayStylePack kSummoner{
	"Summoner",
	"Conjurer",
	{.quest = 80, .discovery = 80, .clear = 60, .combat = 20, .reading = 100, .crafting = 50, .skillup = 100},
	{
		.objectives = 100,
		.none = 90,
		.main = 70,
		.college = 100,
		.thieves = 25,
		.brotherhood = 20,
		.companions = 25,
		.misc = 50,
		.daedric = 80,
		.side = 70,
		.civilWar = 25,
		.dawnguard = 50,
		.dragonborn = 70,
	},
	{
		Bias(100,
			PlaceKind::DwemerRuin, 100,
			PlaceKind::TelvanniTower, 100,
			PlaceKind::MiraakTemple, 90,
			PlaceKind::Fort, 40,
			PlaceKind::MilitaryCamp, 30),
		Bias(60,
			PlaceKind::Fort, 30,
			PlaceKind::MilitaryCamp, 20,
			PlaceKind::Camp, 40,
			PlaceKind::GiantCamp, 30),
	},
	{},
	BiasSkills(10,
		SkillKind::Conjuration, 100,
		SkillKind::Enchanting, 40,
		SkillKind::Speech, 30,
		SkillKind::Illusion, 20),
};

// Thief — pays: discovery, Thieves Guild, stealth, skill-ups.
// Denies: dungeon clears, Civil War, Main.
inline constexpr PlayStylePack kThief{
	"Thief",
	"Thief",
	{.quest = 70, .discovery = 100, .clear = 35, .combat = 70, .reading = 20, .crafting = 40, .skillup = 100},
	{
		.objectives = 100,
		.none = 75,
		.main = 35,
		.college = 40,
		.thieves = 100,
		.brotherhood = 60,
		.companions = 25,
		.misc = 90,
		.daedric = 60,
		.side = 70,
		.civilWar = 15,
		.dawnguard = 35,
		.dragonborn = 50,
	},
	{
		Bias(100,
			PlaceKind::City, 100,
			PlaceKind::Town, 100,
			PlaceKind::Settlement, 100,
			PlaceKind::Docks, 100,
			PlaceKind::Shack, 100,
			PlaceKind::Castle, 90,
			PlaceKind::NordicRuin, 40,
			PlaceKind::Fort, 40),
		FillPlaces(35),
	},
	{.undead = 0, .stealth = 50, .beast = 0, .skipMisc = false, .mainQuestMultiplier = 0.7f},
	BiasSkills(10,
		SkillKind::Sneak, 100,
		SkillKind::Lockpicking, 100,
		SkillKind::Pickpocket, 100,
		SkillKind::Speech, 100,
		SkillKind::LightArmor, 80,
		SkillKind::Alchemy, 50,
		SkillKind::Archery, 40),
};

// Assassin — pays: Brotherhood, stealth combat, Misc contracts.
// Denies: reading=0, crafting=0, sightseeing discovery.
inline constexpr PlayStylePack kAssassin{
	"Assassin",
	"Assassin",
	{.quest = 70, .discovery = 45, .clear = 80, .combat = 100, .reading = 0, .crafting = 0, .skillup = 50},
	{
		.objectives = 80,
		.none = 90,
		.main = 35,
		.college = 15,
		.thieves = 70,
		.brotherhood = 100,
		.companions = 25,
		.misc = 100,
		.daedric = 50,
		.side = 50,
		.civilWar = 30,
		.dawnguard = 40,
		.dragonborn = 40,
	},
	{
		FillPlaces(45),
		Bias(80, PlaceKind::City, 0, PlaceKind::Town, 0, PlaceKind::Settlement, 0),
	},
	{.undead = 0, .stealth = 70, .beast = 0, .skipMisc = false, .mainQuestMultiplier = 0.8f},
	BiasSkills(0,
		SkillKind::Sneak, 100,
		SkillKind::OneHanded, 80,
		SkillKind::Archery, 70,
		SkillKind::LightArmor, 40,
		SkillKind::Lockpicking, 30),
};

// Paladin — pays: Main, holy clears, undead, Dawnguard, Companions medium.
// Denies: Daedric=0, Thieves=0, Brotherhood=0.
inline constexpr PlayStylePack kPaladin{
	"Paladin",
	"Paladin",
	{.quest = 100, .discovery = 80, .clear = 100, .combat = 80, .reading = 50, .crafting = 20, .skillup = 40},
	{
		.objectives = 100,
		.none = 90,
		.main = 100,
		.college = 50,
		.thieves = 0,
		.brotherhood = 0,
		.companions = 90,
		.misc = 50,
		.daedric = 0,
		.side = 90,
		.civilWar = 80,
		.dawnguard = 100,
		.dragonborn = 80,
	},
	{
		Bias(80,
			PlaceKind::Altar, 100,
			PlaceKind::StandingStone, 100,
			PlaceKind::DaedricShrine, 0,
			PlaceKind::NordicRuin, 90,
			PlaceKind::Cave, 90),
		Bias(100,
			PlaceKind::NordicRuin, 100,
			PlaceKind::Cave, 100,
			PlaceKind::Default, 100,
			PlaceKind::Fort, 90,
			PlaceKind::DragonLair, 100,
			PlaceKind::DaedricShrine, 0),
	},
	{.undead = 60, .stealth = 0, .beast = 0, .skipMisc = false, .mainQuestMultiplier = 1.6f},
	BiasSkills(10,
		SkillKind::Restoration, 90,
		SkillKind::OneHanded, 80,
		SkillKind::Block, 80,
		SkillKind::HeavyArmor, 80,
		SkillKind::Smithing, 40,
		SkillKind::Speech, 40),
};

// Warrior — pays: clears, kills, forge, Companions, Civil War.
// Denies: reading=0, College low.
inline constexpr PlayStylePack kWarrior{
	"Warrior",
	"Warrior",
	{.quest = 70, .discovery = 70, .clear = 100, .combat = 100, .reading = 0, .crafting = 90, .skillup = 80},
	{
		.objectives = 70,
		.none = 75,
		.main = 75,
		.college = 25,
		.thieves = 25,
		.brotherhood = 35,
		.companions = 100,
		.misc = 50,
		.daedric = 50,
		.side = 70,
		.civilWar = 100,
		.dawnguard = 80,
		.dragonborn = 70,
	},
	{
		Bias(70,
			PlaceKind::Fort, 100,
			PlaceKind::MilitaryCamp, 100,
			PlaceKind::GiantCamp, 100,
			PlaceKind::DragonLair, 100,
			PlaceKind::Camp, 90),
		Bias(100,
			PlaceKind::Fort, 100,
			PlaceKind::MilitaryCamp, 100,
			PlaceKind::GiantCamp, 100,
			PlaceKind::DragonLair, 100,
			PlaceKind::Camp, 100,
			PlaceKind::Cave, 90),
	},
	{},
	BiasSkills(0,
		SkillKind::OneHanded, 100,
		SkillKind::TwoHanded, 100,
		SkillKind::Block, 100,
		SkillKind::HeavyArmor, 100,
		SkillKind::Smithing, 100,
		SkillKind::Archery, 40),
};

// Mage — pays: College, reading, skill-ups, enchanting.
// Denies: swinging steel, fort clears.
inline constexpr PlayStylePack kMage{
	"Mage",
	"Mage",
	{.quest = 90, .discovery = 80, .clear = 40, .combat = 25, .reading = 100, .crafting = 100, .skillup = 100},
	{
		.objectives = 100,
		.none = 90,
		.main = 70,
		.college = 100,
		.thieves = 25,
		.brotherhood = 20,
		.companions = 25,
		.misc = 50,
		.daedric = 100,
		.side = 70,
		.civilWar = 25,
		.dawnguard = 50,
		.dragonborn = 80,
	},
	{
		Bias(80,
			PlaceKind::DwemerRuin, 100,
			PlaceKind::TelvanniTower, 100,
			PlaceKind::MiraakTemple, 90,
			PlaceKind::Fort, 30,
			PlaceKind::MilitaryCamp, 20),
		Bias(40,
			PlaceKind::DwemerRuin, 70,
			PlaceKind::Fort, 20,
			PlaceKind::MilitaryCamp, 10,
			PlaceKind::Camp, 20),
	},
	{},
	BiasSkills(0,
		SkillKind::Alteration, 100,
		SkillKind::Conjuration, 100,
		SkillKind::Destruction, 100,
		SkillKind::Illusion, 100,
		SkillKind::Restoration, 100,
		SkillKind::Enchanting, 100,
		SkillKind::Alchemy, 80),
};

// Ranger — pays: woods/camps/passes, discovery, beast hunts.
// Denies: city Misc, hold capitals.
inline constexpr PlayStylePack kRanger{
	"Ranger",
	"Ranger",
	{.quest = 70, .discovery = 100, .clear = 90, .combat = 90, .reading = 20, .crafting = 20, .skillup = 60},
	{
		.objectives = 80,
		.none = 60,
		.main = 50,
		.college = 25,
		.thieves = 35,
		.brotherhood = 25,
		.companions = 70,
		.misc = 25,
		.daedric = 40,
		.side = 80,
		.civilWar = 40,
		.dawnguard = 50,
		.dragonborn = 60,
	},
	{
		Bias(40,
			PlaceKind::Camp, 100,
			PlaceKind::Grove, 100,
			PlaceKind::Clearing, 100,
			PlaceKind::Pass, 100,
			PlaceKind::WoodMill, 90,
			PlaceKind::Farm, 80,
			PlaceKind::Landmark, 90,
			PlaceKind::GiantCamp, 100,
			PlaceKind::Rock, 80,
			PlaceKind::City, 20,
			PlaceKind::Town, 25,
			PlaceKind::Docks, 25,
			PlaceKind::Castle, 25),
		Bias(80,
			PlaceKind::Camp, 100,
			PlaceKind::Grove, 100,
			PlaceKind::GiantCamp, 100,
			PlaceKind::Clearing, 90,
			PlaceKind::City, 0,
			PlaceKind::Town, 0),
	},
	{.undead = 0, .stealth = 0, .beast = 40, .skipMisc = false, .mainQuestMultiplier = 0.9f},
	BiasSkills(10,
		SkillKind::Archery, 100,
		SkillKind::Sneak, 70,
		SkillKind::LightArmor, 70,
		SkillKind::Alchemy, 40,
		SkillKind::Block, 30),
};

// Bard — pays: Side, Misc stories, the road, speech. Denies: grinding
// clears, shield-wall combat.
inline constexpr PlayStylePack kBard{
	"Bard",
	"Bard",
	{.quest = 100, .discovery = 100, .clear = 40, .combat = 30, .reading = 90, .crafting = 30, .skillup = 100},
	{
		.objectives = 100,
		.none = 100,
		.main = 75,
		.college = 70,
		.thieves = 70,
		.brotherhood = 35,
		.companions = 50,
		.misc = 100,
		.daedric = 70,
		.side = 100,
		.civilWar = 50,
		.dawnguard = 60,
		.dragonborn = 70,
	},
	{
		Bias(100,
			PlaceKind::City, 100,
			PlaceKind::Town, 100,
			PlaceKind::Settlement, 100,
			PlaceKind::Landmark, 100,
			PlaceKind::Docks, 100),
		FillPlaces(40),
	},
	{},
	BiasSkills(15,
		SkillKind::Speech, 100,
		SkillKind::Illusion, 50,
		SkillKind::Alchemy, 30,
		SkillKind::LightArmor, 25),
};

// Merchant — pays: crafting, trade skill-ups, Misc jobs.
// Denies: combat=0, clears=0.
inline constexpr PlayStylePack kMerchant{
	"Merchant",
	"Merchant",
	{.quest = 80, .discovery = 70, .clear = 0, .combat = 0, .reading = 30, .crafting = 100, .skillup = 100},
	{
		.objectives = 80,
		.none = 90,
		.main = 40,
		.college = 50,
		.thieves = 70,
		.brotherhood = 15,
		.companions = 35,
		.misc = 100,
		.daedric = 30,
		.side = 80,
		.civilWar = 10,
		.dawnguard = 20,
		.dragonborn = 30,
	},
	{
		Bias(70,
			PlaceKind::City, 100,
			PlaceKind::Town, 100,
			PlaceKind::Settlement, 100,
			PlaceKind::Farm, 90,
			PlaceKind::Smelter, 100,
			PlaceKind::Stable, 90,
			PlaceKind::WoodMill, 90,
			PlaceKind::WheatMill, 90,
			PlaceKind::Docks, 100),
		FillPlaces(0),
	},
	{.undead = 0, .stealth = 0, .beast = 0, .skipMisc = false, .mainQuestMultiplier = 0.8f},
	BiasSkills(10,
		SkillKind::Speech, 100,
		SkillKind::Smithing, 80,
		SkillKind::Alchemy, 80,
		SkillKind::Enchanting, 60,
		SkillKind::Pickpocket, 20),
};

// Necromancer — display name Summoner. Pays: tombs/clears, reading,
// enchanting, Daedric medium. Denies: fighting in person, military camps.
inline constexpr PlayStylePack kNecromancer{
	"Necromancer",
	"Summoner",
	{.quest = 80, .discovery = 70, .clear = 100, .combat = 25, .reading = 100, .crafting = 80, .skillup = 90},
	{
		.objectives = 80,
		.none = 90,
		.main = 60,
		.college = 90,
		.thieves = 25,
		.brotherhood = 50,
		.companions = 25,
		.misc = 40,
		.daedric = 90,
		.side = 70,
		.civilWar = 25,
		.dawnguard = 40,
		.dragonborn = 80,
	},
	{
		Bias(70,
			PlaceKind::NordicRuin, 100,
			PlaceKind::Cave, 100,
			PlaceKind::NordicDwelling, 100,
			PlaceKind::DwemerRuin, 90,
			PlaceKind::Default, 90,
			PlaceKind::DaedricShrine, 90,
			PlaceKind::City, 40,
			PlaceKind::MilitaryCamp, 25,
			PlaceKind::Fort, 40),
		Bias(50,
			PlaceKind::NordicRuin, 100,
			PlaceKind::Cave, 100,
			PlaceKind::NordicDwelling, 100,
			PlaceKind::NordicTower, 100,
			PlaceKind::DwemerRuin, 90,
			PlaceKind::Default, 90,
			PlaceKind::CastleKarstaag, 100,
			PlaceKind::Camp, 25,
			PlaceKind::MilitaryCamp, 20,
			PlaceKind::Fort, 30,
			PlaceKind::GiantCamp, 25),
	},
	{.undead = 50, .stealth = 0, .beast = 0, .skipMisc = false, .mainQuestMultiplier = 1.0f},
	BiasSkills(10,
		SkillKind::Conjuration, 100,
		SkillKind::Enchanting, 80,
		SkillKind::Destruction, 30,
		SkillKind::Illusion, 25),
};

// Beastblood — pays: the hunt, clears, beast combat. Denies: reading=0,
// crafting=0, Misc busywork (skipped).
inline constexpr PlayStylePack kBeastblood{
	"Beastblood",
	"Beastblood",
	{.quest = 40, .discovery = 80, .clear = 100, .combat = 100, .reading = 0, .crafting = 0, .skillup = 30},
	{
		.objectives = 50,
		.none = 50,
		.main = 35,
		.college = 20,
		.thieves = 25,
		.brotherhood = 30,
		.companions = 80,
		.misc = 20,
		.daedric = 40,
		.side = 40,
		.civilWar = 35,
		.dawnguard = 25,
		.dragonborn = 40,
	},
	{
		Bias(50,
			PlaceKind::Grove, 100,
			PlaceKind::Camp, 100,
			PlaceKind::GiantCamp, 100,
			PlaceKind::Cave, 90,
			PlaceKind::Clearing, 90,
			PlaceKind::City, 20,
			PlaceKind::Town, 25),
		Bias(100,
			PlaceKind::Grove, 100,
			PlaceKind::Camp, 100,
			PlaceKind::GiantCamp, 100,
			PlaceKind::Cave, 100,
			PlaceKind::City, 0,
			PlaceKind::Town, 0),
	},
	{.undead = 0, .stealth = 0, .beast = 70, .skipMisc = true, .mainQuestMultiplier = 0.7f},
	BiasSkills(0,
		SkillKind::TwoHanded, 80,
		SkillKind::LightArmor, 70,
		SkillKind::Sneak, 50,
		SkillKind::Archery, 40),
};

// Spellsword — pays: combat + College + clears. Denies: nothing hard —
// mixed war-mage.
inline constexpr PlayStylePack kSpellsword{
	"Spellsword",
	"Spellsword",
	{.quest = 90, .discovery = 80, .clear = 100, .combat = 100, .reading = 70, .crafting = 50, .skillup = 90},
	{
		.objectives = 100,
		.none = 100,
		.main = 85,
		.college = 100,
		.thieves = 35,
		.brotherhood = 30,
		.companions = 80,
		.misc = 60,
		.daedric = 70,
		.side = 80,
		.civilWar = 80,
		.dawnguard = 80,
		.dragonborn = 80,
	},
	{
		Bias(80,
			PlaceKind::Fort, 100,
			PlaceKind::DwemerRuin, 100,
			PlaceKind::NordicRuin, 100,
			PlaceKind::Cave, 90),
		Bias(100,
			PlaceKind::Fort, 100,
			PlaceKind::DwemerRuin, 100,
			PlaceKind::NordicRuin, 100,
			PlaceKind::Cave, 90),
	},
	{.undead = 0, .stealth = 0, .beast = 0, .skipMisc = false, .mainQuestMultiplier = 1.1f},
	BiasSkills(20,
		SkillKind::Destruction, 80,
		SkillKind::OneHanded, 80,
		SkillKind::HeavyArmor, 60,
		SkillKind::Conjuration, 50,
		SkillKind::Block, 50,
		SkillKind::Alteration, 40),
};

// Monk — pays: training, pilgrimage stones/altars. Denies: crafting=0,
// Brotherhood/Thieves low.
inline constexpr PlayStylePack kMonk{
	"Monk",
	"Monk",
	{.quest = 100, .discovery = 100, .clear = 70, .combat = 40, .reading = 80, .crafting = 0, .skillup = 100},
	{
		.objectives = 100,
		.none = 75,
		.main = 85,
		.college = 70,
		.thieves = 10,
		.brotherhood = 5,
		.companions = 50,
		.misc = 50,
		.daedric = 20,
		.side = 90,
		.civilWar = 40,
		.dawnguard = 60,
		.dragonborn = 70,
	},
	{
		Bias(80,
			PlaceKind::Altar, 100,
			PlaceKind::StandingStone, 100,
			PlaceKind::Doomstone, 100,
			PlaceKind::Pass, 100,
			PlaceKind::Landmark, 90,
			PlaceKind::Grove, 90,
			PlaceKind::Fort, 30,
			PlaceKind::MilitaryCamp, 25,
			PlaceKind::DaedricShrine, 30),
		Bias(50,
			PlaceKind::Fort, 30,
			PlaceKind::MilitaryCamp, 20,
			PlaceKind::Camp, 40,
			PlaceKind::Altar, 0,
			PlaceKind::StandingStone, 0),
	},
	{.undead = 0, .stealth = 0, .beast = 0, .skipMisc = false, .mainQuestMultiplier = 1.1f},
	BiasSkills(20,
		SkillKind::Restoration, 80,
		SkillKind::Alteration, 70,
		SkillKind::Block, 40,
		SkillKind::Speech, 50,
		SkillKind::Sneak, 30),
};

// Illusionist — pays: Illusion only, Speech medium. Denies: other magic
// schools 0, combat skills 0.
inline constexpr PlayStylePack kIllusionist{
	"Illusionist",
	"Illusionist",
	{.quest = 80, .discovery = 70, .clear = 20, .combat = 10, .reading = 100, .crafting = 20, .skillup = 100},
	{
		.objectives = 90,
		.none = 70,
		.main = 50,
		.college = 100,
		.thieves = 30,
		.brotherhood = 10,
		.companions = 15,
		.misc = 60,
		.daedric = 40,
		.side = 70,
		.civilWar = 15,
		.dawnguard = 30,
		.dragonborn = 50,
	},
	{
		Bias(70, PlaceKind::City, 80, PlaceKind::Town, 80, PlaceKind::Fort, 20),
		FillPlaces(20),
	},
	{},
	BiasSkills(0, SkillKind::Illusion, 100, SkillKind::Speech, 50),
};

// Elementalist — pays: Destruction 100. Denies: other schools 0.
inline constexpr PlayStylePack kElementalist{
	"elementalist",
	"Elementalist",
	{.quest = 85, .discovery = 70, .clear = 40, .combat = 20, .reading = 100, .crafting = 40, .skillup = 100},
	{
		.objectives = 90,
		.none = 70,
		.main = 60,
		.college = 100,
		.thieves = 15,
		.brotherhood = 10,
		.companions = 20,
		.misc = 40,
		.daedric = 70,
		.side = 60,
		.civilWar = 20,
		.dawnguard = 40,
		.dragonborn = 60,
	},
	{
		Bias(70, PlaceKind::DwemerRuin, 80, PlaceKind::Fort, 30),
		Bias(40, PlaceKind::Fort, 20, PlaceKind::Camp, 20),
	},
	{},
	BiasSkills(0, SkillKind::Destruction, 100, SkillKind::Enchanting, 20),
};

// Battlemage — pays: Destruction, Conjuration, Heavy Armor, One-handed.
// Denies: stealth skills 0.
inline constexpr PlayStylePack kBattlemage{
	"Battlemage",
	"Battlemage",
	{.quest = 85, .discovery = 75, .clear = 90, .combat = 80, .reading = 70, .crafting = 40, .skillup = 100},
	{
		.objectives = 90,
		.none = 80,
		.main = 80,
		.college = 100,
		.thieves = 20,
		.brotherhood = 15,
		.companions = 60,
		.misc = 40,
		.daedric = 70,
		.side = 70,
		.civilWar = 70,
		.dawnguard = 70,
		.dragonborn = 70,
	},
	{
		Bias(80, PlaceKind::Fort, 100, PlaceKind::DwemerRuin, 90, PlaceKind::Cave, 80),
		Bias(90, PlaceKind::Fort, 100, PlaceKind::DwemerRuin, 90, PlaceKind::Cave, 80),
	},
	{},
	BiasSkills(0,
		SkillKind::Destruction, 100,
		SkillKind::Conjuration, 100,
		SkillKind::OneHanded, 80,
		SkillKind::HeavyArmor, 70,
		SkillKind::Block, 50,
		SkillKind::Enchanting, 40),
};


// Crafter — pays: crafting category, smithing/alchemy/enchanting ranks,
// town/smelter discovery, Misc/Side craft jobs. Denies: combat low, clears low,
// swinging steel / war magic skills 0.
inline constexpr PlayStylePack kCrafter{
	"Crafter",
	"Crafter",
	{.quest = 70, .discovery = 80, .clear = 25, .combat = 15, .reading = 40, .crafting = 100, .skillup = 100},
	{
		.objectives = 80,
		.none = 70,
		.main = 40,
		.college = 70,
		.thieves = 35,
		.brotherhood = 10,
		.companions = 50,
		.misc = 100,
		.daedric = 30,
		.side = 90,
		.civilWar = 15,
		.dawnguard = 25,
		.dragonborn = 40,
	},
	{
		Bias(50,
			PlaceKind::City, 100,
			PlaceKind::Town, 100,
			PlaceKind::Settlement, 100,
			PlaceKind::Smelter, 100,
			PlaceKind::Farm, 90,
			PlaceKind::WoodMill, 90,
			PlaceKind::WheatMill, 90,
			PlaceKind::Stable, 80,
			PlaceKind::Docks, 80,
			PlaceKind::Fort, 20,
			PlaceKind::MilitaryCamp, 15,
			PlaceKind::DragonLair, 20),
		Bias(25,
			PlaceKind::Fort, 15,
			PlaceKind::MilitaryCamp, 10,
			PlaceKind::Camp, 20,
			PlaceKind::City, 0,
			PlaceKind::Town, 0),
	},
	{.undead = 0, .stealth = 0, .beast = 0, .skipMisc = false, .mainQuestMultiplier = 0.85f},
	BiasSkills(0,
		SkillKind::Smithing, 100,
		SkillKind::Alchemy, 100,
		SkillKind::Enchanting, 100,
		SkillKind::Speech, 40),
};

// Wizard — pays: all six magic schools 100, reading, skill-ups, College/Daedric.
// Denies: Alchemy/Speech/smithing/combat/stealth 0. Mage keeps Alchemy 80 +
// crafting 100; Wizard is magic-skills-only.
inline constexpr PlayStylePack kWizard{
	"Wizard",
	"Wizard",
	{.quest = 90, .discovery = 70, .clear = 20, .combat = 10, .reading = 100, .crafting = 20, .skillup = 100},
	{
		.objectives = 90,
		.none = 80,
		.main = 60,
		.college = 100,
		.thieves = 20,
		.brotherhood = 10,
		.companions = 15,
		.misc = 40,
		.daedric = 100,
		.side = 70,
		.civilWar = 15,
		.dawnguard = 40,
		.dragonborn = 70,
	},
	{
		Bias(70,
			PlaceKind::City, 80,
			PlaceKind::Town, 70,
			PlaceKind::DwemerRuin, 90,
			PlaceKind::TelvanniTower, 100,
			PlaceKind::MiraakTemple, 80,
			PlaceKind::Fort, 20,
			PlaceKind::MilitaryCamp, 15),
		Bias(20,
			PlaceKind::Fort, 15,
			PlaceKind::MilitaryCamp, 10,
			PlaceKind::Camp, 15,
			PlaceKind::City, 0,
			PlaceKind::Town, 0),
	},
	{.undead = 0, .stealth = 0, .beast = 0, .skipMisc = false, .mainQuestMultiplier = 0.9f},
	BiasSkills(0,
		SkillKind::Alteration, 100,
		SkillKind::Conjuration, 100,
		SkillKind::Destruction, 100,
		SkillKind::Illusion, 100,
		SkillKind::Restoration, 100,
		SkillKind::Enchanting, 100),
};

inline constexpr std::array<PlayStylePack, 20> kPresets = {{
	kAdventurer,
	kVigilant,
	kSummoner,
	kThief,
	kAssassin,
	kPaladin,
	kWarrior,
	kMage,
	kRanger,
	kBard,
	kMerchant,
	kNecromancer,
	kBeastblood,
	kSpellsword,
	kMonk,
	kIllusionist,
	kElementalist,
	kBattlemage,
	kCrafter,
	kWizard,
}};

static_assert(CategoriesCapped(kAdventurer.categories));
static_assert(CategoriesCapped(kVigilant.categories));
static_assert(CategoriesCapped(kSummoner.categories));
static_assert(CategoriesCapped(kThief.categories));
static_assert(CategoriesCapped(kAssassin.categories));
static_assert(CategoriesCapped(kPaladin.categories));
static_assert(CategoriesCapped(kWarrior.categories));
static_assert(CategoriesCapped(kMage.categories));
static_assert(CategoriesCapped(kRanger.categories));
static_assert(CategoriesCapped(kBard.categories));
static_assert(CategoriesCapped(kMerchant.categories));
static_assert(CategoriesCapped(kNecromancer.categories));
static_assert(CategoriesCapped(kBeastblood.categories));
static_assert(CategoriesCapped(kSpellsword.categories));
static_assert(CategoriesCapped(kMonk.categories));
static_assert(CategoriesCapped(kIllusionist.categories));
static_assert(CategoriesCapped(kElementalist.categories));
static_assert(CategoriesCapped(kBattlemage.categories));
static_assert(CategoriesCapped(kCrafter.categories));
static_assert(kCrafter.categories.crafting == 100.f);
static_assert(kCrafter.categories.combat == 15.f);
static_assert(kCrafter.skills[SkillKind::Smithing] == 100.f);
static_assert(kCrafter.skills[SkillKind::Alchemy] == 100.f);
static_assert(kCrafter.skills[SkillKind::Enchanting] == 100.f);
static_assert(kCrafter.skills[SkillKind::OneHanded] == 0.f);
static_assert(CategoriesCapped(kWizard.categories));
static_assert(kWizard.categories.reading == 100.f);
static_assert(kWizard.categories.skillup == 100.f);
static_assert(kWizard.categories.quest == 90.f);
static_assert(kWizard.categories.combat == 10.f);
static_assert(kWizard.categories.crafting == 20.f);
static_assert(kWizard.categories.clear == 20.f);
static_assert(kWizard.skills[SkillKind::Alteration] == 100.f);
static_assert(kWizard.skills[SkillKind::Conjuration] == 100.f);
static_assert(kWizard.skills[SkillKind::Destruction] == 100.f);
static_assert(kWizard.skills[SkillKind::Illusion] == 100.f);
static_assert(kWizard.skills[SkillKind::Restoration] == 100.f);
static_assert(kWizard.skills[SkillKind::Enchanting] == 100.f);
static_assert(kWizard.skills[SkillKind::Alchemy] == 0.f);
static_assert(kWizard.skills[SkillKind::Speech] == 0.f);
static_assert(kWizard.skills[SkillKind::Smithing] == 0.f);
static_assert(kWizard.skills[SkillKind::OneHanded] == 0.f);
static_assert(kMage.skills[SkillKind::Alchemy] == 80.f);
static_assert(kMage.categories.crafting == 100.f);
static_assert(kIllusionist.skills[SkillKind::Illusion] == 100.f);
static_assert(kIllusionist.skills[SkillKind::Alteration] == 0.f);
static_assert(kIllusionist.skills[SkillKind::Conjuration] == 0.f);
static_assert(kIllusionist.skills[SkillKind::Destruction] == 0.f);
static_assert(kIllusionist.skills[SkillKind::Restoration] == 0.f);
static_assert(kIllusionist.skills[SkillKind::OneHanded] == 0.f);
static_assert(kElementalist.skills[SkillKind::Destruction] == 100.f);
static_assert(kElementalist.skills[SkillKind::Illusion] == 0.f);
static_assert(kSummoner.skills[SkillKind::Conjuration] == 100.f);
static_assert(kVigilant.quests.daedric == 0);
static_assert(kPaladin.quests.daedric == 0);
static_assert(kPaladin.quests.thieves == 0);
static_assert(kPaladin.quests.brotherhood == 0);
static_assert(kMerchant.categories.combat == 0);
static_assert(kMerchant.categories.clear == 0);
static_assert(kAssassin.categories.reading == 0);
static_assert(kAssassin.categories.crafting == 0);
static_assert(kWarrior.categories.reading == 0);
static_assert(kBeastblood.categories.reading == 0);
static_assert(kBeastblood.categories.crafting == 0);
static_assert(kMonk.categories.crafting == 0);

inline std::string_view CanonicalPresetId(std::string_view id)
{
	if (EqI(id, "Conjurer")) {
		return "Summoner";
	}
	// Jo rename: old INI / Papyrus saves used DestructionMage.
	if (EqI(id, "DestructionMage") || EqI(id, "Destruction Mage")) {
		return "elementalist";
	}
	return id;
}

inline const PlayStylePack* FindPreset(std::string_view id)
{
	const auto canon = CanonicalPresetId(id);
	for (const auto& pack : kPresets) {
		if (EqI(pack.id, canon) || EqI(pack.id, id)) {
			return &pack;
		}
	}
	return nullptr;
}

inline int ScaledWeightXP(int base, int scalePercent, int lo, int hi)
{
	if (base <= 0 || scalePercent <= 0) {
		return 0;
	}
	return ClampInt((base * scalePercent) / 100, lo, hi);
}

inline int ScaledPlaceXP(int base, int scalePercent)
{
	return ScaledWeightXP(base, scalePercent, kPlaceXPMin, kPlaceXPMax);
}

inline int ScaledQuestXP(int base, int scalePercent)
{
	return ScaledWeightXP(base, scalePercent, kQuestXPMin, kQuestXPMax);
}

}  // namespace AdventureXP
