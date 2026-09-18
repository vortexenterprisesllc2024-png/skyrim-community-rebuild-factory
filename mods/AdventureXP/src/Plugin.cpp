#include "AdventureXP/PCH.h"
#include "AdventureXP/Config.h"
#include "AdventureXP/Events.h"
#include "AdventureXP/PlayerXp.h"
#include "AdventureXP/Globals.h"
#include "AdventureXP/Papyrus.h"
#include "AdventureXP/Serialization.h"
#include "AdventureXP/SettingsHook.h"
#include "AdventureXP/Version.h"

namespace
{
    void InitLog()
    {
        auto path = SKSE::log::log_directory();
        if (!path) {
            return;
        }
        *path /= "AdventureXP.log";
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
        auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));
        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);
        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%H:%M:%S] [%l] %v");
    }

    void OnMessage(SKSE::MessagingInterface::Message* message)
    {
        if (!message) {
            return;
        }

        switch (message->type) {
        case SKSE::MessagingInterface::kDataLoaded:
            AdventureXP::Config::Get().Load();
            AdventureXP::BindGlobals();
            AdventureXP::ApplySkillLevelingSetting();
            AdventureXP::RegisterEventSinks();
            logger::info(
                "AdventureXP {} ready (AE 1.7.104 / Address Library format 5, preset {})",
                ADVENTUREXP_VERSION_STRING,
                AdventureXP::Config::Get().presetId);
            break;
        case SKSE::MessagingInterface::kNewGame:
            AdventureXP::RevertState();
            AdventureXP::OnSaveLoaded();
            break;
        case SKSE::MessagingInterface::kPostLoadGame:
            AdventureXP::OnSaveLoaded();
            break;
        default:
            break;
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    InitLog();
    SKSE::Init(skse);

    const auto runtime = REL::Module::get().version();
    logger::info(
        "AdventureXP {} loading (SKSE interface {:08X}, runtime {}.{}.{}.{})",
        ADVENTUREXP_VERSION_STRING,
        skse->SKSEVersion(),
        runtime.major(),
        runtime.minor(),
        runtime.patch(),
        runtime.build());

    if (runtime < REL::Version{ 1, 6, 0, 0 }) {
        logger::warn("This plugin is written for AE 1.7.104 + SKSE 2.3.1; older runtimes are untested");
    }

    AdventureXP::RegisterSerialization();

    if (const auto* papyrus = SKSE::GetPapyrusInterface()) {
        papyrus->Register(AdventureXP::Papyrus::Register);
    } else {
        logger::warn("Papyrus interface missing — INI presets still work");
    }

    const auto* messaging = SKSE::GetMessagingInterface();
    if (!messaging || !messaging->RegisterListener(OnMessage)) {
        logger::error("Failed to register SKSE messaging listener");
        return false;
    }

    return true;
}
