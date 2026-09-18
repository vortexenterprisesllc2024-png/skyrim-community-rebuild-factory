#include "AdventureXP/PCH.h"
#include "AdventureXP/Papyrus.h"
#include "AdventureXP/Config.h"
#include "AdventureXP/Presets.h"
#include "AdventureXP/Version.h"

namespace AdventureXP::Papyrus
{
    namespace
    {
        RE::BSFixedString GetVersion(RE::StaticFunctionTag*)
        {
            return ADVENTUREXP_VERSION_STRING;
        }

        std::int32_t GetPresetCount(RE::StaticFunctionTag*)
        {
            return static_cast<std::int32_t>(kPlayStylePackCount);
        }

        RE::BSFixedString GetPresetId(RE::StaticFunctionTag*, std::int32_t index)
        {
            const auto* pack = PackAt(static_cast<std::size_t>(index));
            return pack ? pack->id : "";
        }

        RE::BSFixedString GetPresetName(RE::StaticFunctionTag*, std::int32_t index)
        {
            const auto* pack = PackAt(static_cast<std::size_t>(index));
            return pack ? pack->displayName : "";
        }

        RE::BSFixedString GetActivePreset(RE::StaticFunctionTag*)
        {
            return Config::Get().presetId;
        }

        bool ApplyPreset(RE::StaticFunctionTag*, RE::BSFixedString id)
        {
            const bool ok = Config::Get().ApplyPreset(id.c_str());
            if (ok) {
                logger::info("Papyrus applied play-style pack '{}'", Config::Get().presetId);
            }
            return ok;
        }

        float* WeightPtr(std::string_view category)
        {
            auto& w = Config::Get().weights;
            if (CompareIgnoreCase(category, "quests") == 0 || CompareIgnoreCase(category, "quest") == 0) {
                return &w.quests;
            }
            if (CompareIgnoreCase(category, "discovery") == 0 || CompareIgnoreCase(category, "explore") == 0) {
                return &w.discovery;
            }
            if (CompareIgnoreCase(category, "clears") == 0 || CompareIgnoreCase(category, "clear") == 0) {
                return &w.clears;
            }
            if (CompareIgnoreCase(category, "combat") == 0 || CompareIgnoreCase(category, "kills") == 0) {
                return &w.combat;
            }
            if (CompareIgnoreCase(category, "reading") == 0) {
                return &w.reading;
            }
            if (CompareIgnoreCase(category, "crafting") == 0) {
                return &w.crafting;
            }
            if (CompareIgnoreCase(category, "skillups") == 0 || CompareIgnoreCase(category, "skill") == 0) {
                return &w.skillups;
            }
            return nullptr;
        }

        float GetCategoryWeight(RE::StaticFunctionTag*, RE::BSFixedString category)
        {
            if (const auto* ptr = WeightPtr(category.c_str())) {
                return *ptr;
            }
            return 0.0f;
        }

        void SetCategoryWeight(RE::StaticFunctionTag*, RE::BSFixedString category, float weight)
        {
            if (auto* ptr = WeightPtr(category.c_str())) {
                *ptr = Math::ClampPercent(weight);
                Config::Get().presetId = "custom";
            }
        }

        float GetGlobalXPPercent(RE::StaticFunctionTag*)
        {
            return Config::Get().globalXPPercent;
        }

        void SetGlobalXPPercent(RE::StaticFunctionTag*, float percent)
        {
            Config::Get().globalXPPercent = Math::ClampPercent(percent);
        }

        bool GetEnabled(RE::StaticFunctionTag*)
        {
            return Config::Get().enabled;
        }

        void SetEnabled(RE::StaticFunctionTag*, bool enabled)
        {
            Config::Get().enabled = enabled;
        }

        float* QuestTypePtr(std::string_view name)
        {
            auto& q = Config::Get().questTypes;
            if (CompareIgnoreCase(name, "main") == 0) {
                return &q.main;
            }
            if (CompareIgnoreCase(name, "guild") == 0) {
                return &q.guild;
            }
            if (CompareIgnoreCase(name, "daedric") == 0) {
                return &q.daedric;
            }
            if (CompareIgnoreCase(name, "side") == 0) {
                return &q.side;
            }
            if (CompareIgnoreCase(name, "misc") == 0) {
                return &q.misc;
            }
            if (CompareIgnoreCase(name, "civilwar") == 0) {
                return &q.civilWar;
            }
            if (CompareIgnoreCase(name, "dlc") == 0) {
                return &q.dlc;
            }
            if (CompareIgnoreCase(name, "other") == 0) {
                return &q.other;
            }
            return nullptr;
        }

        float* PlaceTypePtr(std::string_view name)
        {
            auto& p = Config::Get().placeTypes;
            if (CompareIgnoreCase(name, "default") == 0) {
                return &p.def;
            }
            if (CompareIgnoreCase(name, "city") == 0) {
                return &p.city;
            }
            if (CompareIgnoreCase(name, "dungeon") == 0) {
                return &p.dungeon;
            }
            if (CompareIgnoreCase(name, "cave") == 0) {
                return &p.cave;
            }
            if (CompareIgnoreCase(name, "nordic") == 0) {
                return &p.nordic;
            }
            if (CompareIgnoreCase(name, "dwemer") == 0) {
                return &p.dwemer;
            }
            if (CompareIgnoreCase(name, "fort") == 0) {
                return &p.fort;
            }
            if (CompareIgnoreCase(name, "camp") == 0) {
                return &p.camp;
            }
            if (CompareIgnoreCase(name, "dragon") == 0 || CompareIgnoreCase(name, "dragonlair") == 0) {
                return &p.dragon;
            }
            if (CompareIgnoreCase(name, "mine") == 0) {
                return &p.mine;
            }
            return nullptr;
        }

        float GetQuestTypeWeight(RE::StaticFunctionTag*, RE::BSFixedString name)
        {
            if (const auto* ptr = QuestTypePtr(name.c_str())) {
                return *ptr;
            }
            return 0.0f;
        }

        void SetQuestTypeWeight(RE::StaticFunctionTag*, RE::BSFixedString name, float weight)
        {
            if (auto* ptr = QuestTypePtr(name.c_str())) {
                *ptr = Math::ClampPercent(weight);
            }
        }

        float GetPlaceTypeWeight(RE::StaticFunctionTag*, RE::BSFixedString name)
        {
            if (const auto* ptr = PlaceTypePtr(name.c_str())) {
                return *ptr;
            }
            return 0.0f;
        }

        void SetPlaceTypeWeight(RE::StaticFunctionTag*, RE::BSFixedString name, float weight)
        {
            if (auto* ptr = PlaceTypePtr(name.c_str())) {
                *ptr = Math::ClampPercent(weight);
            }
        }
    }

    bool Register(RE::BSScript::IVirtualMachine* vm)
    {
        if (!vm) {
            return false;
        }
        vm->RegisterFunction("GetVersion", "AdventureXP", GetVersion);
        vm->RegisterFunction("GetPresetCount", "AdventureXP", GetPresetCount);
        vm->RegisterFunction("GetPresetId", "AdventureXP", GetPresetId);
        vm->RegisterFunction("GetPresetName", "AdventureXP", GetPresetName);
        vm->RegisterFunction("GetActivePreset", "AdventureXP", GetActivePreset);
        vm->RegisterFunction("ApplyPreset", "AdventureXP", ApplyPreset);
        vm->RegisterFunction("GetCategoryWeight", "AdventureXP", GetCategoryWeight);
        vm->RegisterFunction("SetCategoryWeight", "AdventureXP", SetCategoryWeight);
        vm->RegisterFunction("GetGlobalXPPercent", "AdventureXP", GetGlobalXPPercent);
        vm->RegisterFunction("SetGlobalXPPercent", "AdventureXP", SetGlobalXPPercent);
        vm->RegisterFunction("GetEnabled", "AdventureXP", GetEnabled);
        vm->RegisterFunction("SetEnabled", "AdventureXP", SetEnabled);
        vm->RegisterFunction("GetQuestTypeWeight", "AdventureXP", GetQuestTypeWeight);
        vm->RegisterFunction("SetQuestTypeWeight", "AdventureXP", SetQuestTypeWeight);
        vm->RegisterFunction("GetPlaceTypeWeight", "AdventureXP", GetPlaceTypeWeight);
        vm->RegisterFunction("SetPlaceTypeWeight", "AdventureXP", SetPlaceTypeWeight);
        logger::info("Registered AdventureXP Papyrus API (own natives — not zax Experience)");
        return true;
    }
}
