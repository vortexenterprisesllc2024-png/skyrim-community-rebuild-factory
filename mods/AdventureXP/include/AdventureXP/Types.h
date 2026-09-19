#pragma once

// Clean-room AdventureXP types. No third-party plugin symbols.
// Header-only so Linux host tests can compile this without CommonLibSSE.

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace AdventureXP {

inline std::string Lower(std::string_view text)
{
	std::string out(text);
	std::transform(out.begin(), out.end(), out.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});
	return out;
}

inline bool EqI(std::string_view a, std::string_view b)
{
	return Lower(a) == Lower(b);
}

inline bool ContainsI(std::string_view hay, std::string_view needle)
{
	const auto h = Lower(hay);
	const auto n = Lower(needle);
	return h.find(n) != std::string::npos;
}

inline bool StartsI(std::string_view hay, std::string_view prefix)
{
	const auto h = Lower(hay);
	const auto p = Lower(prefix);
	return h.size() >= p.size() && h.compare(0, p.size(), p) == 0;
}

enum class Category : std::uint8_t {
	Quest = 0,
	Discovery,
	Clear,
	Combat,
	Reading,
	Crafting,
	SkillUp,
	Count
};

inline constexpr std::array<std::string_view, 7> kCategoryKeys = {
	"quest",
	"discovery",
	"clear",
	"combat",
	"reading",
	"crafting",
	"skillup",
};

inline constexpr std::array<float, 7> kDefaultCategoryWeight = {
	100.f, 100.f, 100.f, 0.f, 0.f, 0.f, 0.f
};

inline std::optional<Category> ParseCategory(std::string_view name)
{
	const auto key = Lower(name);
	if (key == "quest" || key == "quests") {
		return Category::Quest;
	}
	if (key == "discovery") {
		return Category::Discovery;
	}
	if (key == "clear" || key == "clears") {
		return Category::Clear;
	}
	if (key == "combat" || key == "killing") {
		return Category::Combat;
	}
	if (key == "reading") {
		return Category::Reading;
	}
	if (key == "crafting") {
		return Category::Crafting;
	}
	if (key == "skillup" || key == "skill-ups" || key == "skillups") {
		return Category::SkillUp;
	}
	return std::nullopt;
}

inline const char* Key(Category category)
{
	const auto i = static_cast<std::size_t>(category);
	if (i >= kCategoryKeys.size()) {
		return "";
	}
	return kCategoryKeys[i].data();
}

// Quest kinds. Objectives is the per-stage award, not a CK type.
// CK TESQuest type bytes: 0 None, 1 Main, 2 College, 3 Thieves, 4 Brotherhood,
// 5 Companions, 6 Misc, 7 Daedric, 8 Side, 9 Civil War, 10 Dawnguard, 11 Dragonborn.
enum class QuestKind : std::uint8_t {
	Objectives = 0,
	None,
	Main,
	College,
	Thieves,
	Brotherhood,
	Companions,
	Misc,
	Daedric,
	Side,
	CivilWar,
	Dawnguard,
	Dragonborn,
	Count
};

inline constexpr std::array<std::string_view, 13> kQuestKeys = {
	"Objectives",
	"None",
	"Main",
	"College",
	"Thieves",
	"Brotherhood",
	"Companions",
	"Misc",
	"Daedric",
	"Side",
	"CivilWar",
	"Dawnguard",
	"Dragonborn",
};

inline constexpr int kQuestXPMin = 0;
inline constexpr int kQuestXPMax = 200;

inline constexpr std::array<int, 13> kDefaultQuestXP = {
	12,   // Objectives — was fQuestStageXP
	80,   // None — untyped complete, was fQuestCompleteXP
	120,  // Main — 80 * former 1.5 main multiplier
	80,   // College
	80,   // Thieves
	80,   // Brotherhood
	80,   // Companions
	50,   // Misc
	100,  // Daedric
	80,   // Side
	80,   // CivilWar
	90,   // Dawnguard
	90,   // Dragonborn
};

inline std::optional<QuestKind> ParseQuestKind(std::string_view name)
{
	const auto key = Lower(name);
	if (key == "objectives" || key == "objective" || key == "stages") {
		return QuestKind::Objectives;
	}
	if (key == "none" || key == "untyped" || key == "other") {
		return QuestKind::None;
	}
	if (key == "main") {
		return QuestKind::Main;
	}
	if (key == "college" || key == "mage" || key == "winterhold") {
		return QuestKind::College;
	}
	if (key == "thieves" || key == "thievesguild") {
		return QuestKind::Thieves;
	}
	if (key == "brotherhood" || key == "darkbrotherhood") {
		return QuestKind::Brotherhood;
	}
	if (key == "companions") {
		return QuestKind::Companions;
	}
	if (key == "misc" || key == "miscellaneous") {
		return QuestKind::Misc;
	}
	if (key == "daedric") {
		return QuestKind::Daedric;
	}
	if (key == "side") {
		return QuestKind::Side;
	}
	if (key == "civilwar" || key == "civil_war") {
		return QuestKind::CivilWar;
	}
	if (key == "dawnguard") {
		return QuestKind::Dawnguard;
	}
	if (key == "dragonborn") {
		return QuestKind::Dragonborn;
	}
	return std::nullopt;
}

inline const char* Key(QuestKind kind)
{
	const auto i = static_cast<std::size_t>(kind);
	if (i >= kQuestKeys.size()) {
		return "";
	}
	return kQuestKeys[i].data();
}

inline int DefaultQuestXP(QuestKind kind)
{
	const auto i = static_cast<std::size_t>(kind);
	if (i >= kDefaultQuestXP.size()) {
		return 80;
	}
	return kDefaultQuestXP[i];
}

inline QuestKind QuestKindFromCKType(std::uint8_t ckType)
{
	switch (ckType) {
	case 1:
		return QuestKind::Main;
	case 2:
		return QuestKind::College;
	case 3:
		return QuestKind::Thieves;
	case 4:
		return QuestKind::Brotherhood;
	case 5:
		return QuestKind::Companions;
	case 6:
		return QuestKind::Misc;
	case 7:
		return QuestKind::Daedric;
	case 8:
		return QuestKind::Side;
	case 9:
		return QuestKind::CivilWar;
	case 10:
		return QuestKind::Dawnguard;
	case 11:
		return QuestKind::Dragonborn;
	default:
		return QuestKind::None;
	}
}

inline std::optional<QuestKind> ClassifyQuestEditorId(std::string_view editorId)
{
	if (editorId.empty()) {
		return std::nullopt;
	}
	// Vanilla editor-ID prefixes. Checked only when CK type is 0.
	if (StartsI(editorId, "DLC1")) {
		return QuestKind::Dawnguard;
	}
	if (StartsI(editorId, "DLC2")) {
		return QuestKind::Dragonborn;
	}
	if (StartsI(editorId, "MQ")) {
		return QuestKind::Main;
	}
	if (StartsI(editorId, "MG")) {
		return QuestKind::College;
	}
	if (StartsI(editorId, "TG")) {
		return QuestKind::Thieves;
	}
	if (StartsI(editorId, "DB")) {
		return QuestKind::Brotherhood;
	}
	if (StartsI(editorId, "CR") || StartsI(editorId, "C00") || StartsI(editorId, "C01") ||
		StartsI(editorId, "C02") || StartsI(editorId, "C03") || StartsI(editorId, "C04") ||
		StartsI(editorId, "C05") || StartsI(editorId, "C06")) {
		return QuestKind::Companions;
	}
	if (StartsI(editorId, "CW")) {
		return QuestKind::CivilWar;
	}
	if (StartsI(editorId, "DA")) {
		return QuestKind::Daedric;
	}
	if (StartsI(editorId, "MS")) {
		return QuestKind::Side;
	}
	if (StartsI(editorId, "Favor") || StartsI(editorId, "Hireling") || StartsI(editorId, "Freeform")) {
		return QuestKind::Misc;
	}
	return std::nullopt;
}

inline std::optional<QuestKind> ClassifyQuestFaction(std::string_view factionEditorId)
{
	if (EqI(factionEditorId, "CollegeofWinterholdFaction")) {
		return QuestKind::College;
	}
	if (EqI(factionEditorId, "ThievesGuildFaction")) {
		return QuestKind::Thieves;
	}
	if (EqI(factionEditorId, "DarkBrotherhoodFaction")) {
		return QuestKind::Brotherhood;
	}
	if (EqI(factionEditorId, "CompanionsFaction") || EqI(factionEditorId, "CompanionsCrimeFaction")) {
		return QuestKind::Companions;
	}
	if (EqI(factionEditorId, "DLC1DawnguardFaction") || EqI(factionEditorId, "DLC1HunterFaction")) {
		return QuestKind::Dawnguard;
	}
	if (EqI(factionEditorId, "CWImperialFaction") || EqI(factionEditorId, "CWSonsFaction")) {
		return QuestKind::CivilWar;
	}
	return std::nullopt;
}

template <class Range>
QuestKind ClassifyQuest(std::uint8_t ckType, std::string_view editorId, const Range& factionEditorIds)
{
	if (ckType >= 1 && ckType <= 11) {
		return QuestKindFromCKType(ckType);
	}
	if (const auto fromId = ClassifyQuestEditorId(editorId)) {
		return *fromId;
	}
	for (const auto& faction : factionEditorIds) {
		if (const auto fromFaction = ClassifyQuestFaction(faction)) {
			return *fromFaction;
		}
	}
	return QuestKind::None;
}

// Compass / map-marker type bytes from Skyrim.esm (vanilla MARKER_TYPE).
// Used when a place has no distinct LocType* keyword.
enum class MapMarker : std::uint16_t {
	None = 0,
	City = 1,
	Town = 2,
	Settlement = 3,
	Cave = 4,
	Camp = 5,
	Fort = 6,
	NordicRuin = 7,
	DwemerRuin = 8,
	Shipwreck = 9,
	Grove = 10,
	Landmark = 11,
	DragonLair = 12,
	Farm = 13,
	WoodMill = 14,
	Mine = 15,
	ImperialTower = 16,
	Clearing = 17,
	Pass = 18,
	Altar = 19,
	Rock = 20,
	Lighthouse = 21,
	OrcStronghold = 22,
	GiantCamp = 23,
	Shack = 24,
	NordicDwelling = 25,
	Docks = 26,
	Shrine = 27,
	RiftenCastle = 28,
	RiftenCapitol = 29,
	WindhelmCastle = 30,
	WindhelmCapitol = 31,
	WhiterunCastle = 32,
	WhiterunCapitol = 33,
	SolitudeCastle = 34,
	SolitudeCapitol = 35,
	MarkarthCastle = 36,
	MarkarthCapitol = 37,
	WinterholdCastle = 38,
	WinterholdCapitol = 39,
	MorthalCastle = 40,
	MorthalCapitol = 41,
	FalkreathCastle = 42,
	FalkreathCapitol = 43,
	DawnstarCastle = 44,
	DawnstarCapitol = 45,
	MiraakTemple = 46,
	RavenRock = 47,
	AllMakerStone = 48,
	TelvanniTower = 49,
	ToSkyrim = 50,
	ToSolstheim = 51,
	CastleKarstaag = 52,
};

enum class PlaceKind : std::uint8_t {
	Altar = 0,
	Camp,
	Castle,
	CastleKarstaag,
	Cave,
	City,
	Clearing,
	DaedricShrine,
	Default,
	Docks,
	Doomstone,
	DragonLair,
	DwemerRuin,
	Farm,
	Fort,
	GiantCamp,
	Grove,
	ImperialTower,
	Landmark,
	Lighthouse,
	MilitaryCamp,
	Mine,
	MiraakTemple,
	NordicDwelling,
	NordicRuin,
	NordicTower,
	OrcStronghold,
	Pass,
	Rock,
	Settlement,
	Shack,
	Shipwreck,
	Smelter,
	Stable,
	StandingStone,
	TelvanniTower,
	Town,
	WheatMill,
	WoodMill,
	Count
};

inline constexpr std::array<std::string_view, 39> kPlaceKeys = {
	"Altar",
	"Camp",
	"Castle",
	"CastleKarstaag",
	"Cave",
	"City",
	"Clearing",
	"DaedricShrine",
	"Default",
	"Docks",
	"Doomstone",
	"DragonLair",
	"DwemerRuin",
	"Farm",
	"Fort",
	"GiantCamp",
	"Grove",
	"ImperialTower",
	"Landmark",
	"Lighthouse",
	"MilitaryCamp",
	"Mine",
	"MiraakTemple",
	"NordicDwelling",
	"NordicRuin",
	"NordicTower",
	"OrcStronghold",
	"Pass",
	"Rock",
	"Settlement",
	"Shack",
	"Shipwreck",
	"Smelter",
	"Stable",
	"StandingStone",
	"TelvanniTower",
	"Town",
	"WheatMill",
	"WoodMill",
};

inline constexpr int kPlaceXPMin = 0;
inline constexpr int kPlaceXPMax = 200;

// Discovery defaults: cities pay more to find, camps/farms less.
// Clear defaults: habitations 0 (not clearable); lairs and ruins pay the old 100-ish clear.
inline constexpr std::array<int, 39> kDefaultDiscoveryXP = {
	25,  // Altar
	20,  // Camp
	35,  // Castle
	40,  // CastleKarstaag
	30,  // Cave
	40,  // City
	20,  // Clearing
	30,  // DaedricShrine
	30,  // Default
	25,  // Docks
	25,  // Doomstone
	40,  // DragonLair
	35,  // DwemerRuin
	15,  // Farm
	30,  // Fort
	25,  // GiantCamp
	20,  // Grove
	25,  // ImperialTower
	20,  // Landmark
	25,  // Lighthouse
	20,  // MilitaryCamp
	25,  // Mine
	40,  // MiraakTemple
	25,  // NordicDwelling
	30,  // NordicRuin
	30,  // NordicTower
	30,  // OrcStronghold
	20,  // Pass
	15,  // Rock
	25,  // Settlement
	15,  // Shack
	25,  // Shipwreck
	15,  // Smelter
	15,  // Stable
	25,  // StandingStone
	35,  // TelvanniTower
	30,  // Town
	15,  // WheatMill
	15,  // WoodMill
};

inline constexpr std::array<int, 39> kDefaultClearXP = {
	0,    // Altar
	60,   // Camp
	0,    // Castle
	120,  // CastleKarstaag
	100,  // Cave
	0,    // City
	40,   // Clearing
	80,   // DaedricShrine
	100,  // Default
	0,    // Docks
	0,    // Doomstone
	150,  // DragonLair
	120,  // DwemerRuin
	0,    // Farm
	110,  // Fort
	90,   // GiantCamp
	70,   // Grove
	80,   // ImperialTower
	0,    // Landmark
	0,    // Lighthouse
	80,   // MilitaryCamp
	90,   // Mine
	140,  // MiraakTemple
	80,   // NordicDwelling
	110,  // NordicRuin
	100,  // NordicTower
	100,  // OrcStronghold
	50,   // Pass
	0,    // Rock
	0,    // Settlement
	0,    // Shack
	80,   // Shipwreck
	0,    // Smelter
	0,    // Stable
	0,    // StandingStone
	110,  // TelvanniTower
	0,    // Town
	0,    // WheatMill
	0,    // WoodMill
};

inline std::optional<PlaceKind> ParsePlaceKind(std::string_view name)
{
	const auto key = Lower(name);
	if (key == "altar") {
		return PlaceKind::Altar;
	}
	if (key == "camp") {
		return PlaceKind::Camp;
	}
	if (key == "castle") {
		return PlaceKind::Castle;
	}
	if (key == "castlekarstaag" || key == "karstaag") {
		return PlaceKind::CastleKarstaag;
	}
	if (key == "cave") {
		return PlaceKind::Cave;
	}
	if (key == "city") {
		return PlaceKind::City;
	}
	if (key == "clearing") {
		return PlaceKind::Clearing;
	}
	if (key == "daedricshrine" || key == "shrine") {
		return PlaceKind::DaedricShrine;
	}
	if (key == "default" || key == "dungeon") {
		return PlaceKind::Default;
	}
	if (key == "docks" || key == "dock" || key == "harbor") {
		return PlaceKind::Docks;
	}
	if (key == "doomstone" || key == "allmaker") {
		return PlaceKind::Doomstone;
	}
	if (key == "dragonlair" || key == "dragon") {
		return PlaceKind::DragonLair;
	}
	if (key == "dwemerruin" || key == "dwemer" || key == "dwarvenruin") {
		return PlaceKind::DwemerRuin;
	}
	if (key == "farm") {
		return PlaceKind::Farm;
	}
	if (key == "fort") {
		return PlaceKind::Fort;
	}
	if (key == "giantcamp") {
		return PlaceKind::GiantCamp;
	}
	if (key == "grove") {
		return PlaceKind::Grove;
	}
	if (key == "imperialtower") {
		return PlaceKind::ImperialTower;
	}
	if (key == "landmark") {
		return PlaceKind::Landmark;
	}
	if (key == "lighthouse") {
		return PlaceKind::Lighthouse;
	}
	if (key == "militarycamp") {
		return PlaceKind::MilitaryCamp;
	}
	if (key == "mine") {
		return PlaceKind::Mine;
	}
	if (key == "miraaktemple" || key == "miraak") {
		return PlaceKind::MiraakTemple;
	}
	if (key == "nordicdwelling") {
		return PlaceKind::NordicDwelling;
	}
	if (key == "nordicruin" || key == "nordic") {
		return PlaceKind::NordicRuin;
	}
	if (key == "nordictower") {
		return PlaceKind::NordicTower;
	}
	if (key == "orcstronghold") {
		return PlaceKind::OrcStronghold;
	}
	if (key == "pass") {
		return PlaceKind::Pass;
	}
	if (key == "rock") {
		return PlaceKind::Rock;
	}
	if (key == "settlement") {
		return PlaceKind::Settlement;
	}
	if (key == "shack") {
		return PlaceKind::Shack;
	}
	if (key == "shipwreck" || key == "ship") {
		return PlaceKind::Shipwreck;
	}
	if (key == "smelter") {
		return PlaceKind::Smelter;
	}
	if (key == "stable") {
		return PlaceKind::Stable;
	}
	if (key == "standingstone") {
		return PlaceKind::StandingStone;
	}
	if (key == "telvannitower" || key == "telvanni") {
		return PlaceKind::TelvanniTower;
	}
	if (key == "town") {
		return PlaceKind::Town;
	}
	if (key == "wheatmill" || key == "grainmill") {
		return PlaceKind::WheatMill;
	}
	if (key == "woodmill" || key == "lumbermill") {
		return PlaceKind::WoodMill;
	}
	return std::nullopt;
}

inline const char* Key(PlaceKind kind)
{
	const auto i = static_cast<std::size_t>(kind);
	if (i >= kPlaceKeys.size()) {
		return "";
	}
	return kPlaceKeys[i].data();
}

inline int DefaultDiscoveryXP(PlaceKind kind)
{
	const auto i = static_cast<std::size_t>(kind);
	if (i >= kDefaultDiscoveryXP.size()) {
		return 30;
	}
	return kDefaultDiscoveryXP[i];
}

inline int DefaultClearXP(PlaceKind kind)
{
	const auto i = static_cast<std::size_t>(kind);
	if (i >= kDefaultClearXP.size()) {
		return 100;
	}
	return kDefaultClearXP[i];
}

struct KeywordMap {
	std::string_view keyword;
	PlaceKind kind;
};

// Most-specific LocType* first. Only vanilla / DLC keywords that exist in CK.
inline constexpr std::array<KeywordMap, 24> kLocTypeMap = {{
	{"LocTypeDragonPriestLair", PlaceKind::DragonLair},
	{"LocTypeDragonLair", PlaceKind::DragonLair},
	{"LocTypeGiantCamp", PlaceKind::GiantCamp},
	{"LocTypeMilitaryCamp", PlaceKind::MilitaryCamp},
	{"LocTypeOrcStronghold", PlaceKind::OrcStronghold},
	{"LocTypeMine", PlaceKind::Mine},
	{"LocTypeLumberMill", PlaceKind::WoodMill},
	{"LocTypeSprigganGrove", PlaceKind::Grove},
	{"LocTypeShipwreck", PlaceKind::Shipwreck},
	{"LocTypeShip", PlaceKind::Shipwreck},
	{"LocTypeNordicRuin", PlaceKind::NordicRuin},
	{"LocTypeDwarvenAutomatons", PlaceKind::DwemerRuin},
	{"LocTypeCave", PlaceKind::Cave},
	{"LocTypeMilitaryFort", PlaceKind::Fort},
	{"LocTypeFort", PlaceKind::Fort},
	{"LocTypeBanditCamp", PlaceKind::Camp},
	{"LocTypeForswornCamp", PlaceKind::Camp},
	{"LocTypeCamp", PlaceKind::Camp},
	{"LocTypeCity", PlaceKind::City},
	{"LocTypeTown", PlaceKind::Town},
	{"LocTypeSettlement", PlaceKind::Settlement},
	{"LocTypeFarm", PlaceKind::Farm},
	{"LocTypeClearing", PlaceKind::Clearing},
	{"LocTypeCastle", PlaceKind::Castle},
}};

inline PlaceKind ClassifyKeywords(auto keywords)
{
	for (const auto& entry : kLocTypeMap) {
		for (const auto& keyword : keywords) {
			if (EqI(keyword, entry.keyword)) {
				return entry.kind;
			}
		}
	}
	return PlaceKind::Default;
}

inline PlaceKind ClassifyMarker(std::uint16_t markerType)
{
	switch (static_cast<MapMarker>(markerType)) {
	case MapMarker::City:
	case MapMarker::RiftenCapitol:
	case MapMarker::WindhelmCapitol:
	case MapMarker::WhiterunCapitol:
	case MapMarker::SolitudeCapitol:
	case MapMarker::MarkarthCapitol:
	case MapMarker::WinterholdCapitol:
	case MapMarker::MorthalCapitol:
	case MapMarker::FalkreathCapitol:
	case MapMarker::DawnstarCapitol:
		return PlaceKind::City;
	case MapMarker::Town:
	case MapMarker::RavenRock:
		return PlaceKind::Town;
	case MapMarker::Settlement:
		return PlaceKind::Settlement;
	case MapMarker::Cave:
		return PlaceKind::Cave;
	case MapMarker::Camp:
		return PlaceKind::Camp;
	case MapMarker::Fort:
		return PlaceKind::Fort;
	case MapMarker::NordicRuin:
		return PlaceKind::NordicRuin;
	case MapMarker::DwemerRuin:
		return PlaceKind::DwemerRuin;
	case MapMarker::Shipwreck:
		return PlaceKind::Shipwreck;
	case MapMarker::Grove:
		return PlaceKind::Grove;
	case MapMarker::Landmark:
		return PlaceKind::Landmark;
	case MapMarker::DragonLair:
		return PlaceKind::DragonLair;
	case MapMarker::Farm:
		return PlaceKind::Farm;
	case MapMarker::WoodMill:
		return PlaceKind::WoodMill;
	case MapMarker::Mine:
		return PlaceKind::Mine;
	case MapMarker::ImperialTower:
		return PlaceKind::ImperialTower;
	case MapMarker::Clearing:
		return PlaceKind::Clearing;
	case MapMarker::Pass:
		return PlaceKind::Pass;
	case MapMarker::Altar:
		return PlaceKind::Altar;
	case MapMarker::Rock:
		return PlaceKind::Rock;
	case MapMarker::Lighthouse:
		return PlaceKind::Lighthouse;
	case MapMarker::OrcStronghold:
		return PlaceKind::OrcStronghold;
	case MapMarker::GiantCamp:
		return PlaceKind::GiantCamp;
	case MapMarker::Shack:
		return PlaceKind::Shack;
	case MapMarker::NordicDwelling:
		return PlaceKind::NordicDwelling;
	case MapMarker::Docks:
		return PlaceKind::Docks;
	case MapMarker::Shrine:
		return PlaceKind::DaedricShrine;
	case MapMarker::RiftenCastle:
	case MapMarker::WindhelmCastle:
	case MapMarker::WhiterunCastle:
	case MapMarker::SolitudeCastle:
	case MapMarker::MarkarthCastle:
	case MapMarker::WinterholdCastle:
	case MapMarker::MorthalCastle:
	case MapMarker::FalkreathCastle:
	case MapMarker::DawnstarCastle:
		return PlaceKind::Castle;
	case MapMarker::MiraakTemple:
		return PlaceKind::MiraakTemple;
	case MapMarker::AllMakerStone:
		return PlaceKind::Doomstone;
	case MapMarker::TelvanniTower:
		return PlaceKind::TelvanniTower;
	case MapMarker::CastleKarstaag:
		return PlaceKind::CastleKarstaag;
	default:
		return PlaceKind::Default;
	}
}

// Editor-ID hints for kinds with no LocType* and no unique marker, plus unique DLC sites.
inline std::optional<PlaceKind> ClassifyPlaceEditorId(std::string_view editorId)
{
	if (editorId.empty()) {
		return std::nullopt;
	}
	if (ContainsI(editorId, "Karstaag")) {
		return PlaceKind::CastleKarstaag;
	}
	if (ContainsI(editorId, "Miraak")) {
		return PlaceKind::MiraakTemple;
	}
	if (ContainsI(editorId, "Telvanni")) {
		return PlaceKind::TelvanniTower;
	}
	if (ContainsI(editorId, "WheatMill") || ContainsI(editorId, "GrainMill") || ContainsI(editorId, "FlourMill")) {
		return PlaceKind::WheatMill;
	}
	if (ContainsI(editorId, "Smelter")) {
		return PlaceKind::Smelter;
	}
	if (ContainsI(editorId, "Stable") || ContainsI(editorId, "HorseMarker")) {
		return PlaceKind::Stable;
	}
	if (ContainsI(editorId, "NordicTower") || ContainsI(editorId, "WatchTower") || ContainsI(editorId, "Watchtower")) {
		return PlaceKind::NordicTower;
	}
	if (ContainsI(editorId, "Doomstone") || ContainsI(editorId, "AllMaker")) {
		return PlaceKind::Doomstone;
	}
	if (ContainsI(editorId, "StandingStone") || ContainsI(editorId, "GuardianStone")) {
		return PlaceKind::StandingStone;
	}
	if (ContainsI(editorId, "DaedricShrine") || ContainsI(editorId, "ShrineOf")) {
		return PlaceKind::DaedricShrine;
	}
	return std::nullopt;
}

inline PlaceKind ClassifyPlace(
	auto keywords,
	std::optional<std::uint16_t> markerType,
	std::string_view editorId)
{
	if (const auto fromId = ClassifyPlaceEditorId(editorId)) {
		return *fromId;
	}
	const auto fromKw = ClassifyKeywords(keywords);
	if (fromKw != PlaceKind::Default) {
		return fromKw;
	}
	if (markerType) {
		const auto fromMk = ClassifyMarker(*markerType);
		if (fromMk != PlaceKind::Default) {
			return fromMk;
		}
	}
	return PlaceKind::Default;
}

inline int ClampInt(int value, int lo, int hi)
{
	return std::max(lo, std::min(hi, value));
}

inline float ClampPercent(float value)
{
	if (value < 0.f) {
		return 0.f;
	}
	if (value > 100.f) {
		return 100.f;
	}
	return value;
}

enum class SkillKind : std::uint8_t {
	Alteration = 0,
	Conjuration,
	Destruction,
	Illusion,
	Restoration,
	Enchanting,
	OneHanded,
	TwoHanded,
	Archery,
	Block,
	HeavyArmor,
	Smithing,
	LightArmor,
	Sneak,
	Lockpicking,
	Pickpocket,
	Speech,
	Alchemy,
	Count
};

inline constexpr std::array<std::string_view, 18> kSkillKeys = {
	"Alteration",
	"Conjuration",
	"Destruction",
	"Illusion",
	"Restoration",
	"Enchanting",
	"OneHanded",
	"TwoHanded",
	"Archery",
	"Block",
	"HeavyArmor",
	"Smithing",
	"LightArmor",
	"Sneak",
	"Lockpicking",
	"Pickpocket",
	"Speech",
	"Alchemy",
};

inline constexpr std::array<float, 18> kDefaultSkillWeight = {
	100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
};

inline std::optional<SkillKind> ParseSkill(std::string_view name)
{
	const auto key = Lower(name);
	if (key == "alteration") {
		return SkillKind::Alteration;
	}
	if (key == "conjuration") {
		return SkillKind::Conjuration;
	}
	if (key == "destruction") {
		return SkillKind::Destruction;
	}
	if (key == "illusion") {
		return SkillKind::Illusion;
	}
	if (key == "restoration") {
		return SkillKind::Restoration;
	}
	if (key == "enchanting") {
		return SkillKind::Enchanting;
	}
	if (key == "onehanded" || key == "one-handed" || key == "one_handed") {
		return SkillKind::OneHanded;
	}
	if (key == "twohanded" || key == "two-handed" || key == "two_handed") {
		return SkillKind::TwoHanded;
	}
	if (key == "archery" || key == "marksman") {
		return SkillKind::Archery;
	}
	if (key == "block") {
		return SkillKind::Block;
	}
	if (key == "heavyarmor" || key == "heavy_armor" || key == "heavy armor") {
		return SkillKind::HeavyArmor;
	}
	if (key == "smithing") {
		return SkillKind::Smithing;
	}
	if (key == "lightarmor" || key == "light_armor" || key == "light armor") {
		return SkillKind::LightArmor;
	}
	if (key == "sneak") {
		return SkillKind::Sneak;
	}
	if (key == "lockpicking") {
		return SkillKind::Lockpicking;
	}
	if (key == "pickpocket") {
		return SkillKind::Pickpocket;
	}
	if (key == "speech" || key == "speechcraft") {
		return SkillKind::Speech;
	}
	if (key == "alchemy") {
		return SkillKind::Alchemy;
	}
	return std::nullopt;
}

inline const char* Key(SkillKind kind)
{
	const auto i = static_cast<std::size_t>(kind);
	if (i >= kSkillKeys.size()) {
		return "";
	}
	return kSkillKeys[i].data();
}

// Vanilla ActorValue bytes for the 18 skills (CommonLib / Skyrim.esm).
inline std::optional<SkillKind> SkillFromActorValue(std::int32_t actorValue)
{
	switch (actorValue) {
	case 6:
		return SkillKind::OneHanded;
	case 7:
		return SkillKind::TwoHanded;
	case 8:
		return SkillKind::Archery;
	case 9:
		return SkillKind::Block;
	case 10:
		return SkillKind::Smithing;
	case 11:
		return SkillKind::HeavyArmor;
	case 12:
		return SkillKind::LightArmor;
	case 13:
		return SkillKind::Pickpocket;
	case 14:
		return SkillKind::Lockpicking;
	case 15:
		return SkillKind::Sneak;
	case 16:
		return SkillKind::Alchemy;
	case 17:
		return SkillKind::Speech;
	case 18:
		return SkillKind::Alteration;
	case 19:
		return SkillKind::Conjuration;
	case 20:
		return SkillKind::Destruction;
	case 21:
		return SkillKind::Illusion;
	case 22:
		return SkillKind::Restoration;
	case 23:
		return SkillKind::Enchanting;
	default:
		return std::nullopt;
	}
}

}  // namespace AdventureXP
