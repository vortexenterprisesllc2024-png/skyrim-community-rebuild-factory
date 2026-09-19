#include "AdventureXP/PCH.h"
#include "AdventureXP/Config.h"

namespace AdventureXP
{
    namespace
    {
        std::filesystem::path PluginIniPath()
        {
            HMODULE self = nullptr;
            if (GetModuleHandleExW(
                    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                    reinterpret_cast<LPCWSTR>(&PluginIniPath),
                    &self)) {
                wchar_t buf[MAX_PATH]{};
                if (GetModuleFileNameW(self, buf, MAX_PATH) > 0) {
                    return std::filesystem::path{ buf }.parent_path() / "AdventureXP.ini";
                }
            }
            return std::filesystem::path{ "AdventureXP.ini" };
        }

        bool ReadBool(const CSimpleIniA& ini, const char* section, const char* key, bool fallback)
        {
            return ini.GetBoolValue(section, key, fallback);
        }

        float ReadFloat(const CSimpleIniA& ini, const char* section, const char* key, float fallback)
        {
            return static_cast<float>(ini.GetDoubleValue(section, key, fallback));
        }

        std::int32_t ReadInt(const CSimpleIniA& ini, const char* section, const char* key, std::int32_t fallback)
        {
            return static_cast<std::int32_t>(ini.GetLongValue(section, key, fallback));
        }
    }

    Config& Config::Get()
    {
        static Config instance;
        return instance;
    }

    bool Config::ApplyPreset(std::string_view id)
    {
        const auto* pack = FindPack(id);
        if (!pack) {
            presetId = "custom";
            return false;
        }
        presetId = pack->id;
        weights = pack->weights;
        flavor = pack->flavor;
        questTypes = pack->questTypes;
        placeTypes = pack->placeTypes;
        mainQuestMultiplier = pack->flavor.mainQuestMultiplier;
        skipMiscQuests = pack->flavor.skipMiscQuests;
        return true;
    }

    bool Config::Load()
    {
        const auto path = PluginIniPath();
        path_ = path.string();

        CSimpleIniA ini;
        ini.SetUnicode();

        const SI_Error rc = ini.LoadFile(path_.c_str());
        if (rc < 0) {
            logger::warn("AdventureXP.ini not found at {} — using {} pack defaults", path_, kDefaultPresetId);
            ApplyPreset(kDefaultPresetId);
            return false;
        }

        enabled = ReadBool(ini, "General", "bEnabled", enabled);
        globalXPPercent = ReadFloat(ini, "General", "fGlobalXPPercent", globalXPPercent);
        showXPMessages = ReadBool(ini, "General", "bShowXPMessages", showXPMessages);
        ignoreSkillLeveling = ReadBool(ini, "General", "bIgnoreSkillLeveling", ignoreSkillLeveling);

        const char* preset = ini.GetValue("General", "sPreset", kDefaultPresetId);
        const std::string presetName = preset ? preset : kDefaultPresetId;
        if (!ApplyPreset(presetName)) {
            logger::info("sPreset={} is Custom or unknown — keeping / reading slider overrides", presetName);
            presetId = "custom";
        } else {
            logger::info("Applied play-style pack '{}'", presetId);
        }

        awardQuestStages = ReadBool(ini, "Quests", "bAwardQuestStages", awardQuestStages);
        awardQuestComplete = ReadBool(ini, "Quests", "bAwardQuestComplete", awardQuestComplete);
        skipHiddenQuests = ReadBool(ini, "Quests", "bSkipHiddenQuests", skipHiddenQuests);
        skipMiscQuests = ReadBool(ini, "Quests", "bSkipMiscQuests", skipMiscQuests);
        questStageXP = ReadFloat(ini, "Quests", "fQuestStageXP", questStageXP);
        questCompleteXP = ReadFloat(ini, "Quests", "fQuestCompleteXP", questCompleteXP);
        mainQuestMultiplier = ReadFloat(ini, "Quests", "fMainQuestMultiplier", mainQuestMultiplier);

        discoveryXP = ReadFloat(ini, "Exploration", "fDiscoveryXP", discoveryXP);
        dungeonClearXP = ReadFloat(ini, "Exploration", "fDungeonClearXP", dungeonClearXP);

        killXP = ReadFloat(ini, "Optional", "fKillXP", killXP);
        bossKillXP = ReadFloat(ini, "Optional", "fBossKillXP", bossKillXP);
        readingXP = ReadFloat(ini, "Optional", "fReadingXP", readingXP);
        craftingXP = ReadFloat(ini, "Optional", "fCraftingXP", craftingXP);
        skillUpXP = ReadFloat(ini, "Optional", "fSkillUpXP", skillUpXP);

        weights.quests = ReadFloat(ini, "Weights", "fQuestWeight", weights.quests);
        weights.discovery = ReadFloat(ini, "Weights", "fDiscoveryWeight", weights.discovery);
        weights.clears = ReadFloat(ini, "Weights", "fClearWeight", weights.clears);
        weights.combat = ReadFloat(ini, "Weights", "fCombatWeight", weights.combat);
        weights.reading = ReadFloat(ini, "Weights", "fReadingWeight", weights.reading);
        weights.crafting = ReadFloat(ini, "Weights", "fCraftingWeight", weights.crafting);
        weights.skillups = ReadFloat(ini, "Weights", "fSkillUpWeight", weights.skillups);

        flavor.undeadCombatBonus = ReadFloat(ini, "Flavor", "fUndeadCombatBonus", flavor.undeadCombatBonus);
        flavor.stealthCombatBonus = ReadFloat(ini, "Flavor", "fStealthCombatBonus", flavor.stealthCombatBonus);
        flavor.beastCombatBonus = ReadFloat(ini, "Flavor", "fBeastCombatBonus", flavor.beastCombatBonus);

        questTypes.main = ReadFloat(ini, "QuestTypes", "fMain", questTypes.main);
        questTypes.guild = ReadFloat(ini, "QuestTypes", "fGuild", questTypes.guild);
        questTypes.daedric = ReadFloat(ini, "QuestTypes", "fDaedric", questTypes.daedric);
        questTypes.side = ReadFloat(ini, "QuestTypes", "fSide", questTypes.side);
        questTypes.misc = ReadFloat(ini, "QuestTypes", "fMisc", questTypes.misc);
        questTypes.civilWar = ReadFloat(ini, "QuestTypes", "fCivilWar", questTypes.civilWar);
        questTypes.dlc = ReadFloat(ini, "QuestTypes", "fDLC", questTypes.dlc);
        questTypes.other = ReadFloat(ini, "QuestTypes", "fOther", questTypes.other);

        placeTypes.def = ReadFloat(ini, "PlaceTypes", "fDefault", placeTypes.def);
        placeTypes.city = ReadFloat(ini, "PlaceTypes", "fCity", placeTypes.city);
        placeTypes.dungeon = ReadFloat(ini, "PlaceTypes", "fDungeon", placeTypes.dungeon);
        placeTypes.cave = ReadFloat(ini, "PlaceTypes", "fCave", placeTypes.cave);
        placeTypes.nordic = ReadFloat(ini, "PlaceTypes", "fNordic", placeTypes.nordic);
        placeTypes.dwemer = ReadFloat(ini, "PlaceTypes", "fDwemer", placeTypes.dwemer);
        placeTypes.fort = ReadFloat(ini, "PlaceTypes", "fFort", placeTypes.fort);
        placeTypes.camp = ReadFloat(ini, "PlaceTypes", "fCamp", placeTypes.camp);
        placeTypes.dragon = ReadFloat(ini, "PlaceTypes", "fDragonLair", placeTypes.dragon);
        placeTypes.mine = ReadFloat(ini, "PlaceTypes", "fMine", placeTypes.mine);

        curve.base = ReadFloat(ini, "Leveling", "fXPBase", curve.base);
        curve.perLevel = ReadFloat(ini, "Leveling", "fXPPerLevel", curve.perLevel);
        curve.exponent = ReadFloat(ini, "Leveling", "fXPExponent", curve.exponent);
        curve.maxLevel = ReadInt(ini, "Leveling", "iMaxLevel", curve.maxLevel);

        logger::info(
            "Loaded config from {} (preset {}, quest {} / discover {} / clear {} / combat {} / read {} / craft {} / skill {})",
            path_,
            presetId,
            weights.quests,
            weights.discovery,
            weights.clears,
            weights.combat,
            weights.reading,
            weights.crafting,
            weights.skillups);
        return true;
    }
}
