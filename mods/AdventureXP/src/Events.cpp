#include "AdventureXP/PCH.h"
#include "AdventureXP/Events.h"
#include "AdventureXP/Config.h"
#include "AdventureXP/PlayerXp.h"
#include "AdventureXP/Serialization.h"
#include "AdventureXP/Types.h"
#include "AdventureXP/XpMath.h"

namespace AdventureXP
{
    namespace
    {
        std::uint64_t PackQuestStage(RE::FormID formID, std::uint16_t stage)
        {
            return (static_cast<std::uint64_t>(formID) << 16) | stage;
        }

        bool IsPlayerRef(const RE::TESObjectREFR* refr)
        {
            const auto* player = RE::PlayerCharacter::GetSingleton();
            return player && refr && refr->GetFormID() == player->GetFormID();
        }

        bool IsHiddenQuest(const RE::TESQuest* quest)
        {
            if (!quest) {
                return true;
            }
            if (ClassifyQuestType(static_cast<std::uint32_t>(quest->GetType())) == QuestKind::Hidden) {
                return true;
            }
            const auto* name = quest->GetName();
            return !name || name[0] == '\0';
        }

        float QuestMultiplier(const RE::TESQuest* quest)
        {
            if (!quest) {
                return 1.0f;
            }
            const auto& cfg = Config::Get();
            const auto kind = ClassifyQuestType(static_cast<std::uint32_t>(quest->GetType()));
            float mult = Math::ApplyGlobalPercent(1.0f, WeightFor(kind, cfg.questTypes));
            if (kind == QuestKind::Main) {
                mult *= cfg.mainQuestMultiplier;
            }
            return mult;
        }

        constexpr const char* kPlaceKeywords[] = {
            "LocTypeDragonLair",
            "LocTypeNordicRuin",
            "LocTypeDraugrCrypt",
            "LocTypeMilitaryFort",
            "LocTypeBanditCamp",
            "LocTypeForswornCamp",
            "LocTypeCamp",
            "LocTypeMine",
            "LocTypeCave",
            "LocTypeCity",
            "LocTypeTown",
            "LocTypeSettlement",
            "LocTypeDungeon",
            "LocTypeDwemerRuin",
            "LocTypeDwarvenAutomatons",
        };

        PlaceKind ClassifyLocation(RE::BGSLocation* location)
        {
            auto kind = PlaceKind::Default;
            if (!location) {
                return kind;
            }
            for (const auto* edid : kPlaceKeywords) {
                const auto* keyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(edid);
                if (keyword && location->HasKeyword(keyword)) {
                    kind = Stronger(kind, ClassifyPlaceKeyword(edid));
                }
            }
            return kind;
        }

        float PlaceMultiplier(RE::BGSLocation* location)
        {
            return Math::ApplyGlobalPercent(1.0f, WeightFor(ClassifyLocation(location), Config::Get().placeTypes));
        }

        std::string QuestLabel(const RE::TESQuest* quest)
        {
            if (!quest) {
                return "quest";
            }
            if (const auto* name = quest->GetName(); name && name[0]) {
                return name;
            }
            if (const auto* edid = quest->GetFormEditorID(); edid && edid[0]) {
                return edid;
            }
            return "quest";
        }

        void ConsiderLocationClear(RE::BGSLocation* location)
        {
            const auto& cfg = Config::Get();
            if (!cfg.AwardClears() || !location) {
                return;
            }
            if (!location->IsCleared()) {
                return;
            }

            const auto formID = location->GetFormID();
            auto& awarded = State().awardedClears;
            if (awarded.contains(formID)) {
                return;
            }
            awarded.insert(formID);

            std::string label = "dungeon";
            if (const auto* name = location->GetName(); name && name[0]) {
                label = name;
            } else if (const auto* edid = location->GetFormEditorID(); edid && edid[0]) {
                label = edid;
            }
            PlayerXp::Get().Award(
                AwardSource::DungeonClear,
                cfg.dungeonClearXP * PlaceMultiplier(location),
                label + " cleared");
        }

        void CheckPlayerLocation()
        {
            auto* player = RE::PlayerCharacter::GetSingleton();
            if (!player) {
                return;
            }
            ConsiderLocationClear(player->GetCurrentLocation());
        }

        bool HasKeywordEDID(RE::TESForm* form, const char* edid)
        {
            auto* keyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(edid);
            if (!keyword || !form) {
                return false;
            }
            if (auto* actor = form->As<RE::Actor>()) {
                return actor->HasKeyword(keyword);
            }
            return false;
        }

        bool PlayerIsBeastForm(RE::PlayerCharacter* player)
        {
            if (!player) {
                return false;
            }
            const auto* race = player->GetRace();
            const auto* edid = race ? race->GetFormEditorID() : nullptr;
            if (!edid || !edid[0]) {
                return false;
            }
            const std::string_view id{ edid };
            return id.find("Werewolf") != std::string_view::npos || id.find("VampireLord") != std::string_view::npos;
        }

        bool IsCraftingSkill(RE::ActorValue av)
        {
            return av == RE::ActorValue::kSmithing || av == RE::ActorValue::kAlchemy || av == RE::ActorValue::kEnchanting;
        }

        class Handler :
            public RE::BSTEventSink<RE::TESQuestStageEvent>,
            public RE::BSTEventSink<RE::TESCombatEvent>,
            public RE::BSTEventSink<RE::TESDeathEvent>,
            public RE::BSTEventSink<RE::BGSActorCellEvent>,
            public RE::BSTEventSink<RE::LocationDiscovery::Event>,
            public RE::BSTEventSink<RE::BooksRead::Event>,
            public RE::BSTEventSink<RE::SkillIncrease::Event>
        {
        public:
            static Handler& Get()
            {
                static Handler instance;
                return instance;
            }

            RE::BSEventNotifyControl ProcessEvent(const RE::TESQuestStageEvent* ev, RE::BSTEventSource<RE::TESQuestStageEvent>*) override
            {
                if (!ev) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                auto* quest = RE::TESForm::LookupByID<RE::TESQuest>(ev->formID);
                if (!quest) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                const auto& cfg = Config::Get();
                if (cfg.skipHiddenQuests && IsHiddenQuest(quest)) {
                    return RE::BSEventNotifyControl::kContinue;
                }
                if (cfg.skipMiscQuests &&
                    ClassifyQuestType(static_cast<std::uint32_t>(quest->GetType())) == QuestKind::Misc) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                const auto label = QuestLabel(quest);
                const float mult = QuestMultiplier(quest);

                if (cfg.awardQuestComplete && quest->IsCompleted()) {
                    if (State().awardedQuestCompletes.insert(quest->GetFormID()).second) {
                        PlayerXp::Get().Award(AwardSource::QuestComplete, cfg.questCompleteXP * mult, label);
                    }
                    return RE::BSEventNotifyControl::kContinue;
                }

                if (cfg.awardQuestStages && ev->stage != 0) {
                    const auto key = PackQuestStage(quest->GetFormID(), ev->stage);
                    if (State().awardedQuestStages.insert(key).second) {
                        PlayerXp::Get().Award(AwardSource::QuestStage, cfg.questStageXP * mult, label);
                    }
                }
                return RE::BSEventNotifyControl::kContinue;
            }

            RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* ev, RE::BSTEventSource<RE::TESCombatEvent>*) override
            {
                if (!ev) {
                    return RE::BSEventNotifyControl::kContinue;
                }
                // After a fight ends, the current cell may now be marked cleared.
                if (ev->newState.get() == RE::ACTOR_COMBAT_STATE::kNone && IsPlayerRef(ev->actor.get())) {
                    CheckPlayerLocation();
                }
                return RE::BSEventNotifyControl::kContinue;
            }

            RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent* ev, RE::BSTEventSource<RE::TESDeathEvent>*) override
            {
                const auto& cfg = Config::Get();
                if (!cfg.AwardKills() || !ev || !ev->actorDying) {
                    return RE::BSEventNotifyControl::kContinue;
                }
                if (!IsPlayerRef(ev->actorKiller.get())) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                auto* dying = ev->actorDying->As<RE::Actor>();
                if (!dying || dying->IsPlayerRef() || dying->IsGhost()) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                float xp = cfg.killXP;
                std::string_view detail = "defeat";
                if (HasKeywordEDID(dying, "ActorTypeDragon") || dying->GetLevel() >= 30) {
                    xp = cfg.bossKillXP;
                    detail = "boss";
                }

                auto* player = RE::PlayerCharacter::GetSingleton();
                float bonus = 0.0f;
                if (cfg.flavor.undeadCombatBonus > 0.0f && HasKeywordEDID(dying, "ActorTypeUndead")) {
                    bonus += cfg.flavor.undeadCombatBonus;
                    detail = "undead";
                }
                if (cfg.flavor.stealthCombatBonus > 0.0f && player && player->IsSneaking()) {
                    bonus += cfg.flavor.stealthCombatBonus;
                    detail = "ambush";
                }
                if (cfg.flavor.beastCombatBonus > 0.0f &&
                    (PlayerIsBeastForm(player) || HasKeywordEDID(dying, "ActorTypeAnimal"))) {
                    bonus += cfg.flavor.beastCombatBonus;
                    detail = "hunt";
                }
                xp *= (1.0f + bonus / 100.0f);

                PlayerXp::Get().Award(AwardSource::Kill, xp, detail);
                CheckPlayerLocation();
                return RE::BSEventNotifyControl::kContinue;
            }

            RE::BSEventNotifyControl ProcessEvent(const RE::BGSActorCellEvent* ev, RE::BSTEventSource<RE::BGSActorCellEvent>*) override
            {
                if (!ev) {
                    return RE::BSEventNotifyControl::kContinue;
                }
                const auto actor = ev->actor.get();
                if (!actor || !actor->IsPlayerRef()) {
                    return RE::BSEventNotifyControl::kContinue;
                }
                if (ev->flags.get() != RE::BGSActorCellEvent::CellFlag::kEnter) {
                    return RE::BSEventNotifyControl::kContinue;
                }
                CheckPlayerLocation();
                return RE::BSEventNotifyControl::kContinue;
            }

            RE::BSEventNotifyControl ProcessEvent(const RE::LocationDiscovery::Event* ev, RE::BSTEventSource<RE::LocationDiscovery::Event>*) override
            {
                const auto& cfg = Config::Get();
                if (!cfg.AwardDiscovery() || !ev) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                // Dedup by a simple hash of the worldspace token + pointer identity.
                const auto key = static_cast<std::uint32_t>(
                    std::hash<const void*>{}(ev->mapMarkerData) ^ std::hash<const char*>{}(ev->worldspaceID ? ev->worldspaceID : ""));
                if (!State().awardedDiscoveries.insert(key).second) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                auto* player = RE::PlayerCharacter::GetSingleton();
                auto* here = player ? player->GetCurrentLocation() : nullptr;
                PlayerXp::Get().Award(
                    AwardSource::Discovery,
                    cfg.discoveryXP * PlaceMultiplier(here),
                    "location discovered");
                return RE::BSEventNotifyControl::kContinue;
            }

            RE::BSEventNotifyControl ProcessEvent(const RE::BooksRead::Event* ev, RE::BSTEventSource<RE::BooksRead::Event>*) override
            {
                const auto& cfg = Config::Get();
                if (!cfg.AwardReading() || !ev || !ev->book) {
                    return RE::BSEventNotifyControl::kContinue;
                }
                if (!State().awardedBooks.insert(ev->book->GetFormID()).second) {
                    return RE::BSEventNotifyControl::kContinue;
                }
                const char* name = ev->book->GetName();
                PlayerXp::Get().Award(AwardSource::Reading, cfg.readingXP, name && name[0] ? name : "book");
                return RE::BSEventNotifyControl::kContinue;
            }

            RE::BSEventNotifyControl ProcessEvent(const RE::SkillIncrease::Event* ev, RE::BSTEventSource<RE::SkillIncrease::Event>*) override
            {
                const auto& cfg = Config::Get();
                if (!ev) {
                    return RE::BSEventNotifyControl::kContinue;
                }
                auto* player = RE::PlayerCharacter::GetSingleton();
                if (ev->player && player && ev->player != player) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                if (IsCraftingSkill(ev->actorValue)) {
                    if (!cfg.AwardCrafting()) {
                        return RE::BSEventNotifyControl::kContinue;
                    }
                    PlayerXp::Get().Award(AwardSource::Crafting, cfg.craftingXP, "craft");
                } else {
                    if (!cfg.AwardSkillUps()) {
                        return RE::BSEventNotifyControl::kContinue;
                    }
                    PlayerXp::Get().Award(AwardSource::SkillUp, cfg.skillUpXP, "training");
                }
                return RE::BSEventNotifyControl::kContinue;
            }
        };

        void SnapshotExistingClears()
        {
            auto& state = State();
            if (state.snapshottedClears) {
                return;
            }

            auto* handler = RE::TESDataHandler::GetSingleton();
            if (!handler) {
                return;
            }

            std::uint32_t count = 0;
            for (auto* location : handler->GetFormArray<RE::BGSLocation>()) {
                if (location && location->IsCleared()) {
                    state.awardedClears.insert(location->GetFormID());
                    ++count;
                }
            }
            state.snapshottedClears = true;
            logger::info("Snapshotted {} already-cleared locations so mid-play installs do not dump XP", count);
        }
    }

    void OnSaveLoaded()
    {
        SnapshotExistingClears();
        CheckPlayerLocation();
        PlayerXp::Get().SyncGlobal();
    }

    void RegisterEventSinks()
    {
        auto& handler = Handler::Get();
        auto* holder = RE::ScriptEventSourceHolder::GetSingleton();
        if (holder) {
            holder->AddEventSink<RE::TESQuestStageEvent>(&handler);
            holder->AddEventSink<RE::TESCombatEvent>(&handler);
            holder->AddEventSink<RE::TESDeathEvent>(&handler);
            holder->AddEventSink<RE::BGSActorCellEvent>(&handler);
        } else {
            logger::error("ScriptEventSourceHolder missing");
        }

        if (auto* discovery = RE::LocationDiscovery::GetEventSource()) {
            discovery->AddEventSink(&handler);
        } else {
            logger::warn("LocationDiscovery event source missing");
        }

        if (auto* books = RE::BooksRead::GetEventSource()) {
            books->AddEventSink(&handler);
        } else {
            logger::warn("BooksRead event source missing");
        }

        if (auto* skills = RE::SkillIncrease::GetEventSource()) {
            skills->AddEventSink(&handler);
        } else {
            logger::warn("SkillIncrease event source missing");
        }

        logger::info("Registered quest, discovery, cell, combat, death, book, and skill-up sinks");
    }
}
