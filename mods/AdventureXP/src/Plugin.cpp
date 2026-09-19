#include "AdventureXP/PCH.h"
#include "AdventureXP/Config.h"
#include "AdventureXP/Events.h"
#include "AdventureXP/Papyrus.h"
#include "AdventureXP/SettingsHook.h"
#include "AdventureXP/Version.h"

#include <shlobj.h>

namespace
{
    // True only after SKSEPluginLoad confirmed AE 1.7.104.x. Event sinks,
    // Papyrus, INI load that can touch REL, and Address Library lookups stay
    // behind this gate so a foreign runtime cannot trip CommonLib's
    // MessageBox+Terminate during splash.
    bool g_runtimeSupported{ false };

    std::filesystem::path DocumentsSkseFolder()
    {
        PWSTR docs = nullptr;
        const HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &docs);
        std::unique_ptr<wchar_t, decltype(&CoTaskMemFree)> known(docs, &CoTaskMemFree);
        if (FAILED(hr) || !known) {
            return {};
        }
        // Steam / WinStore AE. Do not call SKSE::log::log_directory() here:
        // that helper goes through REL::Module::IsVR() and can resolve the
        // Address Library before we have a log file.
        return std::filesystem::path{ known.get() } / L"My Games" / L"Skyrim Special Edition" / L"SKSE";
    }

    std::filesystem::path DllFolder()
    {
        HMODULE self = nullptr;
        if (!GetModuleHandleExW(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCWSTR>(&DllFolder),
                &self)) {
            return {};
        }
        wchar_t buf[MAX_PATH]{};
        if (GetModuleFileNameW(self, buf, MAX_PATH) == 0) {
            return {};
        }
        return std::filesystem::path{ buf }.parent_path();
    }

    void InitLog()
    {
        std::filesystem::path dir = DocumentsSkseFolder();
        if (dir.empty()) {
            dir = DllFolder();
        }
        if (dir.empty()) {
            dir = std::filesystem::temp_directory_path();
        }

        std::error_code ec;
        std::filesystem::create_directories(dir, ec);

        auto path = dir / "AdventureXP.log";
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
        auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));
        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);
        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%H:%M:%S] [%l] %v");
        logger::info(
            "AdventureXP {} log open at {}",
            ADVENTUREXP_VERSION_STRING,
            path.string());
        spdlog::default_logger()->flush();
    }

    bool IsSupportedRuntime(REL::Version runtime)
    {
        return runtime.major() == 1 && runtime.minor() == 7 && runtime.patch() == 104;
    }

    void LoadIni()
    {
        auto ini = DllFolder() / "AdventureXP.ini";
        std::error_code ec;
        if (ini.empty() || !std::filesystem::exists(ini, ec)) {
            ini = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "AdventureXP.ini";
        }
        AdventureXP::Config::Get().Load(ini);
        logger::info("Loaded INI from {} (preset {})", ini.string(), AdventureXP::Config::Get().preset);
    }

    void OnMessage(SKSE::MessagingInterface::Message* message)
    {
        if (!message || !g_runtimeSupported) {
            return;
        }

        switch (message->type) {
        case SKSE::MessagingInterface::kDataLoaded:
            // First REL / Address Library use happens here, after splash
            // and after AdventureXP.log exists. CommonLib v8 Relocation
            // constructors resolve IDs eagerly (IDDB + possible
            // report_and_fail MessageBox behind the Bethesda swirl).
            LoadIni();
            AdventureXP::ApplySkillLevelingSetting();
            AdventureXP::Events::Register();
            if (const auto* papyrus = SKSE::GetPapyrusInterface()) {
                papyrus->Register(AdventureXP::Papyrus::Register);
            } else {
                logger::warn("Papyrus interface missing — INI presets still work");
            }
            logger::info(
                "AdventureXP {} ready (AE 1.7.104 / Address Library format 5, preset {})",
                ADVENTUREXP_VERSION_STRING,
                AdventureXP::Config::Get().preset);
            break;
        default:
            break;
        }
    }

    void RegisterRuntimeFeatures()
    {
        const auto* messaging = SKSE::GetMessagingInterface();
        if (!messaging || !messaging->RegisterListener(OnMessage)) {
            logger::error("Failed to register SKSE messaging listener — staying loaded as a no-op");
            g_runtimeSupported = false;
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    InitLog();

    // Keep our file logger. CommonLib v8 SKSE::Init defaults to {.log=true}
    // and would replace it (same footgun as Description Framework / OIF).
    SKSE::Init(skse, { .log = false });

    // SKSE's packed runtime version — no Address Library, no REL::Module.
    const auto runtime = skse->RuntimeVersion();
    logger::info(
        "AdventureXP {} loading (SKSE interface {:08X}, runtime {}.{}.{}.{})",
        ADVENTUREXP_VERSION_STRING,
        skse->SKSEVersion(),
        runtime.major(),
        runtime.minor(),
        runtime.patch(),
        runtime.build());

    if (!IsSupportedRuntime(runtime)) {
        logger::warn(
            "Runtime {}.{}.{}.{} is not AE 1.7.104.x — not registering sinks, hooks, or Papyrus. "
            "Plugin stays loaded so SKSE continues.",
            runtime.major(),
            runtime.minor(),
            runtime.patch(),
            runtime.build());
        return true;
    }

    g_runtimeSupported = true;
    RegisterRuntimeFeatures();
    logger::info("AE 1.7.104.x confirmed; sinks/hooks deferred until kDataLoaded");
    return true;
}
