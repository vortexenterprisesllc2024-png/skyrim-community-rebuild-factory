#include "AdventureXP/Config.h"

#include <fstream>
#include <sstream>
#include <utility>

namespace AdventureXP {
namespace {

std::string Trim(std::string_view text)
{
	std::size_t a = 0;
	while (a < text.size() && (text[a] == ' ' || text[a] == '\t' || text[a] == '\r')) {
		++a;
	}
	std::size_t b = text.size();
	while (b > a && (text[b - 1] == ' ' || text[b - 1] == '\t' || text[b - 1] == '\r')) {
		--b;
	}
	return std::string(text.substr(a, b - a));
}

bool IsComment(std::string_view line)
{
	return line.empty() || line[0] == ';' || line[0] == '#';
}

bool ReplaceIniValue(std::string& text, std::string_view section, std::string_view key, const std::string& value)
{
	const std::string header = "[" + std::string(section) + "]";
	const auto headerPos = Lower(text).find(Lower(header));
	if (headerPos == std::string::npos) {
		if (!text.empty() && text.back() != '\n') {
			text += '\n';
		}
		text += header;
		text += '\n';
		text += std::string(key);
		text += "=";
		text += value;
		text += '\n';
		return true;
	}

	const auto next = text.find('[', headerPos + header.size());
	const auto end = next == std::string::npos ? text.size() : next;
	const auto keyEq = std::string(key) + "=";
	auto searchFrom = headerPos + header.size();
	while (searchFrom < end) {
		const auto lineEnd = text.find('\n', searchFrom);
		const auto sliceEnd = lineEnd == std::string::npos ? end : std::min(lineEnd, end);
		const auto line = Trim(text.substr(searchFrom, sliceEnd - searchFrom));
		if (!IsComment(line) && StartsI(line, keyEq)) {
			const auto eq = text.find('=', searchFrom);
			if (eq != std::string::npos && eq < sliceEnd) {
				text.replace(eq + 1, sliceEnd - (eq + 1), value);
				return true;
			}
		}
		if (lineEnd == std::string::npos || lineEnd >= end) {
			break;
		}
		searchFrom = lineEnd + 1;
	}

	std::string insert = std::string(key) + "=" + value + "\n";
	text.insert(end, insert);
	return true;
}

}  // namespace

Config& Config::Get()
{
	static Config instance;
	return instance;
}

float Config::CategoryWeight(Category category) const
{
	const auto i = static_cast<std::size_t>(category);
	if (i >= categoryWeight.size()) {
		return 0.f;
	}
	return categoryWeight[i];
}

void Config::SetCategoryWeight(Category category, float weight)
{
	const auto i = static_cast<std::size_t>(category);
	if (i >= categoryWeight.size()) {
		return;
	}
	categoryWeight[i] = ClampPercent(weight);
	preset = "Custom";
	Save();
}

int Config::QuestXP(QuestKind kind) const
{
	const auto i = static_cast<std::size_t>(kind);
	if (i >= questXP.size()) {
		return 0;
	}
	return questXP[i];
}

void Config::SetQuestXP(QuestKind kind, int amount)
{
	const auto i = static_cast<std::size_t>(kind);
	if (i >= questXP.size()) {
		return;
	}
	questXP[i] = ClampInt(amount, kQuestXPMin, kQuestXPMax);
	preset = "Custom";
	Save();
}

int Config::DiscoveryXP(PlaceKind kind) const
{
	const auto i = static_cast<std::size_t>(kind);
	if (i >= discoveryXP.size()) {
		return 0;
	}
	return discoveryXP[i];
}

void Config::SetDiscoveryXP(PlaceKind kind, int amount)
{
	const auto i = static_cast<std::size_t>(kind);
	if (i >= discoveryXP.size()) {
		return;
	}
	discoveryXP[i] = ClampInt(amount, kPlaceXPMin, kPlaceXPMax);
	preset = "Custom";
	Save();
}

int Config::ClearXP(PlaceKind kind) const
{
	const auto i = static_cast<std::size_t>(kind);
	if (i >= clearXP.size()) {
		return 0;
	}
	return clearXP[i];
}

void Config::SetClearXP(PlaceKind kind, int amount)
{
	const auto i = static_cast<std::size_t>(kind);
	if (i >= clearXP.size()) {
		return;
	}
	clearXP[i] = ClampInt(amount, kPlaceXPMin, kPlaceXPMax);
	preset = "Custom";
	Save();
}

void Config::ApplyPack(const PlayStylePack& pack)
{
	categoryWeight = pack.categories.ToArray();
	for (auto& weight : categoryWeight) {
		weight = ClampPercent(weight);
	}
	const auto questScales = pack.quests.ToArray();
	for (std::size_t i = 0; i < kQuestKeys.size(); ++i) {
		questXP[i] = ScaledQuestXP(DefaultQuestXP(static_cast<QuestKind>(i)), questScales[i]);
	}
	for (std::size_t i = 0; i < kPlaceKeys.size(); ++i) {
		const auto kind = static_cast<PlaceKind>(i);
		discoveryXP[i] = ScaledPlaceXP(DefaultDiscoveryXP(kind), pack.places.discovery[i]);
		clearXP[i] = ScaledPlaceXP(DefaultClearXP(kind), pack.places.clear[i]);
	}
	undeadCombatBonus = pack.flavor.undead;
	stealthCombatBonus = pack.flavor.stealth;
	beastCombatBonus = pack.flavor.beast;
	skipMiscQuests = pack.flavor.skipMisc;
	mainQuestMultiplier = pack.flavor.mainQuestMultiplier;
	preset = pack.id;
	awardKilling = pack.categories.combat > 0.f;
	awardReading = pack.categories.reading > 0.f;
	skillWeight = pack.skills.value;
	for (auto& weight : skillWeight) {
		weight = ClampPercent(weight);
	}
}

float Config::SkillWeight(SkillKind kind) const
{
	const auto i = static_cast<std::size_t>(kind);
	if (i >= skillWeight.size()) {
		return 0.f;
	}
	return skillWeight[i];
}

void Config::SetSkillWeight(SkillKind kind, float weight)
{
	const auto i = static_cast<std::size_t>(kind);
	if (i >= skillWeight.size()) {
		return;
	}
	skillWeight[i] = ClampPercent(weight);
	preset = "Custom";
	Save();
}

bool Config::ApplyPreset(std::string_view id)
{
	if (const auto* pack = FindPreset(id)) {
		ApplyPack(*pack);
		Save();
		return true;
	}
	return false;
}

void Config::Load(const std::filesystem::path& path)
{
	path_ = path;
	std::ifstream in(path);
	if (!in) {
		Save();
		return;
	}

	std::string section;
	std::string line;
	while (std::getline(in, line)) {
		if (!line.empty() && line.back() == '\r') {
			line.pop_back();
		}
		const auto trimmed = Trim(line);
		if (IsComment(trimmed)) {
			continue;
		}
		if (!trimmed.empty() && trimmed.front() == '[' && trimmed.back() == ']') {
			section = trimmed.substr(1, trimmed.size() - 2);
			continue;
		}
		const auto eq = trimmed.find('=');
		if (eq == std::string::npos) {
			continue;
		}
		const auto key = Trim(trimmed.substr(0, eq));
		const auto value = Trim(trimmed.substr(eq + 1));

		auto asBool = [&]() {
			return value == "1" || EqI(value, "true") || EqI(value, "yes");
		};
		auto asFloat = [&](float fallback) {
			try {
				return std::stof(value);
			} catch (...) {
				return fallback;
			}
		};
		auto asInt = [&](int fallback) {
			try {
				return std::stoi(value);
			} catch (...) {
				return fallback;
			}
		};

		if (EqI(section, "General")) {
			if (EqI(key, "bEnabled")) {
				enabled = asBool();
			} else if (EqI(key, "fGlobalXPPercent")) {
				globalXPPercent = ClampPercent(asFloat(100.f));
			} else if (EqI(key, "bShowXPMessages")) {
				showMessages = asBool();
			} else if (EqI(key, "bIgnoreSkillLeveling")) {
				ignoreSkillLeveling = asBool();
			} else if (EqI(key, "bAwardKilling")) {
				awardKilling = asBool();
			} else if (EqI(key, "bAwardReading")) {
				awardReading = asBool();
			} else if (EqI(key, "sPreset")) {
				preset = value;
			}
		} else if (EqI(section, "Weights")) {
			if (EqI(key, "fQuestWeight")) {
				categoryWeight[0] = ClampPercent(asFloat(100.f));
			} else if (EqI(key, "fDiscoveryWeight")) {
				categoryWeight[1] = ClampPercent(asFloat(100.f));
			} else if (EqI(key, "fClearWeight")) {
				categoryWeight[2] = ClampPercent(asFloat(100.f));
			} else if (EqI(key, "fCombatWeight")) {
				categoryWeight[3] = ClampPercent(asFloat(0.f));
			} else if (EqI(key, "fReadingWeight")) {
				categoryWeight[4] = ClampPercent(asFloat(0.f));
			} else if (EqI(key, "fCraftingWeight")) {
				categoryWeight[5] = ClampPercent(asFloat(0.f));
			} else if (EqI(key, "fSkillUpWeight")) {
				categoryWeight[6] = ClampPercent(asFloat(0.f));
			}
		} else if (EqI(section, "Quests")) {
			if (EqI(key, "bAwardQuestStages")) {
				awardQuestStages = asBool();
			} else if (EqI(key, "bAwardQuestComplete")) {
				awardQuestComplete = asBool();
			} else if (EqI(key, "bSkipHiddenQuests")) {
				skipHiddenQuests = asBool();
			} else if (EqI(key, "bSkipMiscQuests")) {
				skipMiscQuests = asBool();
			} else if (EqI(key, "fMainQuestMultiplier")) {
				mainQuestMultiplier = asFloat(1.f);
			}
		} else if (EqI(section, "QuestXP")) {
			if (key.size() > 1 && (key[0] == 'i' || key[0] == 'I')) {
				if (const auto kind = ParseQuestKind(key.substr(1))) {
					questXP[static_cast<std::size_t>(*kind)] =
						ClampInt(asInt(DefaultQuestXP(*kind)), kQuestXPMin, kQuestXPMax);
				}
			}
		} else if (EqI(section, "Discovery")) {
			if (key.size() > 1 && (key[0] == 'i' || key[0] == 'I')) {
				if (const auto kind = ParsePlaceKind(key.substr(1))) {
					discoveryXP[static_cast<std::size_t>(*kind)] =
						ClampInt(asInt(DefaultDiscoveryXP(*kind)), kPlaceXPMin, kPlaceXPMax);
				}
			}
		} else if (EqI(section, "Clears")) {
			if (key.size() > 1 && (key[0] == 'i' || key[0] == 'I')) {
				if (const auto kind = ParsePlaceKind(key.substr(1))) {
					clearXP[static_cast<std::size_t>(*kind)] =
						ClampInt(asInt(DefaultClearXP(*kind)), kPlaceXPMin, kPlaceXPMax);
				}
			}
		} else if (EqI(section, "Skills")) {
			if (key.size() > 1 && (key[0] == 'f' || key[0] == 'F')) {
				if (const auto kind = ParseSkill(key.substr(1))) {
					skillWeight[static_cast<std::size_t>(*kind)] = ClampPercent(asFloat(100.f));
				}
			}
		} else if (EqI(section, "Optional")) {
			if (EqI(key, "fKillXP")) {
				killXP = asFloat(2.f);
			} else if (EqI(key, "fBossKillXP")) {
				bossKillXP = asFloat(25.f);
			} else if (EqI(key, "fReadingXP")) {
				readingXP = asFloat(5.f);
			} else if (EqI(key, "fCraftingXP")) {
				craftingXP = asFloat(8.f);
			} else if (EqI(key, "fSkillUpXP")) {
				skillUpXP = asFloat(4.f);
			}
		} else if (EqI(section, "Flavor")) {
			if (EqI(key, "fUndeadCombatBonus")) {
				undeadCombatBonus = asFloat(0.f);
			} else if (EqI(key, "fStealthCombatBonus")) {
				stealthCombatBonus = asFloat(0.f);
			} else if (EqI(key, "fBeastCombatBonus")) {
				beastCombatBonus = asFloat(0.f);
			}
		} else if (EqI(section, "Leveling")) {
			if (EqI(key, "fXPBase")) {
				xpBase = asFloat(200.f);
			} else if (EqI(key, "fXPPerLevel")) {
				xpPerLevel = asFloat(50.f);
			} else if (EqI(key, "fXPExponent")) {
				xpExponent = asFloat(1.f);
			} else if (EqI(key, "iMaxLevel")) {
				maxLevel = asInt(81);
			}
		}
	}

	if (!EqI(preset, "Custom")) {
		if (const auto* pack = FindPreset(preset)) {
			ApplyPack(*pack);
		}
	}
}

void Config::Save() const
{
	if (path_.empty()) {
		return;
	}

	std::string text;
	if (std::ifstream in(path_); in) {
		std::ostringstream ss;
		ss << in.rdbuf();
		text = ss.str();
	}

	if (text.empty()) {
		text =
			"; AdventureXP config. MCM writes these keys immediately.\n"
			"[General]\n"
			"[Weights]\n"
			"[Quests]\n"
			"[QuestXP]\n"
			"[Discovery]\n"
			"[Clears]\n"
			"[Skills]\n"
			"[Optional]\n"
			"[Flavor]\n"
			"[Leveling]\n";
	}

	auto set = [&](std::string_view section, std::string_view key, const std::string& value) {
		ReplaceIniValue(text, section, key, value);
	};
	auto num = [](float v) {
		std::ostringstream ss;
		ss << v;
		return ss.str();
	};

	set("General", "bEnabled", enabled ? "1" : "0");
	set("General", "fGlobalXPPercent", num(globalXPPercent));
	set("General", "bShowXPMessages", showMessages ? "1" : "0");
	set("General", "bIgnoreSkillLeveling", ignoreSkillLeveling ? "1" : "0");
	set("General", "bAwardKilling", awardKilling ? "1" : "0");
	set("General", "bAwardReading", awardReading ? "1" : "0");
	set("General", "sPreset", preset);

	set("Weights", "fQuestWeight", num(categoryWeight[0]));
	set("Weights", "fDiscoveryWeight", num(categoryWeight[1]));
	set("Weights", "fClearWeight", num(categoryWeight[2]));
	set("Weights", "fCombatWeight", num(categoryWeight[3]));
	set("Weights", "fReadingWeight", num(categoryWeight[4]));
	set("Weights", "fCraftingWeight", num(categoryWeight[5]));
	set("Weights", "fSkillUpWeight", num(categoryWeight[6]));

	set("Quests", "bAwardQuestStages", awardQuestStages ? "1" : "0");
	set("Quests", "bAwardQuestComplete", awardQuestComplete ? "1" : "0");
	set("Quests", "bSkipHiddenQuests", skipHiddenQuests ? "1" : "0");
	set("Quests", "bSkipMiscQuests", skipMiscQuests ? "1" : "0");
	set("Quests", "fMainQuestMultiplier", num(mainQuestMultiplier));

	for (std::size_t i = 0; i < kQuestKeys.size(); ++i) {
		set("QuestXP", std::string("i") + std::string(kQuestKeys[i]), std::to_string(questXP[i]));
	}
	for (std::size_t i = 0; i < kPlaceKeys.size(); ++i) {
		set("Discovery", std::string("i") + std::string(kPlaceKeys[i]), std::to_string(discoveryXP[i]));
		set("Clears", std::string("i") + std::string(kPlaceKeys[i]), std::to_string(clearXP[i]));
	}
	for (std::size_t i = 0; i < kSkillKeys.size(); ++i) {
		set("Skills", std::string("f") + std::string(kSkillKeys[i]), num(skillWeight[i]));
	}

	set("Optional", "fKillXP", num(killXP));
	set("Optional", "fBossKillXP", num(bossKillXP));
	set("Optional", "fReadingXP", num(readingXP));
	set("Optional", "fCraftingXP", num(craftingXP));
	set("Optional", "fSkillUpXP", num(skillUpXP));

	set("Flavor", "fUndeadCombatBonus", num(undeadCombatBonus));
	set("Flavor", "fStealthCombatBonus", num(stealthCombatBonus));
	set("Flavor", "fBeastCombatBonus", num(beastCombatBonus));

	set("Leveling", "fXPBase", num(xpBase));
	set("Leveling", "fXPPerLevel", num(xpPerLevel));
	set("Leveling", "iMaxLevel", std::to_string(maxLevel));

	std::filesystem::create_directories(path_.parent_path());
	std::ofstream out(path_, std::ios::trunc);
	out << text;
}

}  // namespace AdventureXP
