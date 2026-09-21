// Auto Plant Everything - SKSE plugin
//
// On kDataLoaded, walks every FLOR and TREE record in the load order, finds the
// ingredient each one produces when harvested, and appends those pairs to the two
// Hearthfire planter FormLists so modded plants show up in planters.
//
// Lineage: scan logic from the Kimi build (which got the FormList lookup right);
// settings file and packaging from the Qwen build. Fixes over both are marked FIX.
//
// The two lists are paired BY INDEX - plantable[i] grows planted[i] - so they must
// always come out the same length. This version collects every candidate pair first,
// validates, and only then commits. Both earlier versions appended as they scanned,
// which meant a mid-scan problem left the lists permanently uneven.

#include "PCH.h"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace {
    constexpr std::uint32_t kPlantableBase = 0x008247;  // flPlanterPlantableItem
    constexpr std::uint32_t kPlantedBase   = 0x008246;  // flPlanterPlantedFlora
    constexpr auto kHearthfire = "HearthFires.esm"sv;

    struct Settings {
        bool enabled = true;
        bool scanHarvest = true;
        bool skipDuplicates = true;
        bool skipTestRecords = true;
        std::unordered_set<std::string> blacklist;  // EditorIDs, lowercased
    };

    Settings g_settings;

    std::string ToLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    std::string Trim(std::string s) {
        const auto notSpace = [](unsigned char c) { return !std::isspace(c); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
        s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
        return s;
    }

    // FIX (Qwen shipped this INI, Kimi ignored it): actually read the settings file.
    void LoadSettings() {
        std::ifstream in("Data/SKSE/Plugins/AutoPlantEverything.ini");
        if (!in) {
            SKSE::log::info("No INI found; using defaults.");
            return;
        }
        std::string line;
        while (std::getline(in, line)) {
            line = Trim(line);
            if (line.empty() || line[0] == '#' || line[0] == ';' || line[0] == '[') {
                continue;
            }
            const auto eq = line.find('=');
            if (eq == std::string::npos) {
                continue;
            }
            const std::string key = ToLower(Trim(line.substr(0, eq)));
            const std::string val = Trim(line.substr(eq + 1));
            const bool on = (val == "1" || ToLower(val) == "true");

            if (key == "benabled")            g_settings.enabled = on;
            else if (key == "bscanharvest")   g_settings.scanHarvest = on;
            else if (key == "bskipduplicates")g_settings.skipDuplicates = on;
            else if (key == "bskiptestrecords") g_settings.skipTestRecords = on;
            else if (key == "blacklist") {
                std::size_t start = 0;
                while (start <= val.size()) {
                    const auto comma = val.find(',', start);
                    auto item = Trim(val.substr(start, comma - start));
                    if (!item.empty()) {
                        g_settings.blacklist.insert(ToLower(item));
                    }
                    if (comma == std::string::npos) break;
                    start = comma + 1;
                }
            }
        }
        SKSE::log::info("Settings: enabled={} scanHarvest={} skipDuplicates={} blacklist={}",
                        g_settings.enabled, g_settings.scanHarvest,
                        g_settings.skipDuplicates, g_settings.blacklist.size());
    }

    // NOTE ON QUEST ITEMS. Kimi's version tested whether the EditorID contained the
    // substring "quest", which misses nearly every real quest item (they rarely say so
    // in the EditorID) and false-flags innocent ones. There is no reliable static
    // "is quest item" flag on a base object - in Skyrim that status is usually applied
    // at runtime through quest aliases, which do not exist yet at kDataLoaded. So this
    // build does not pretend to detect them. Instead it filters what it honestly can
    // (dev/test records) and exposes a user blacklist in the INI. If a quest plant ever
    // does show up in a planter, add its EditorID to Blacklist= and it is gone next load.
    bool Excluded(const RE::TESForm* form) {
        if (!form) {
            return true;
        }
        const char* rawEdid = form->GetFormEditorID();
        if (!rawEdid || !*rawEdid) {
            return false;  // no EditorID is not itself a reason to skip
        }
        const std::string edid = ToLower(rawEdid);
        if (g_settings.skipTestRecords && edid.rfind("test", 0) == 0) {
            return true;   // e.g. TestTreeFloraMountainFlower01Blue, a dev leftover
        }
        return g_settings.blacklist.contains(edid);
    }

    RE::TESBoundObject* ProduceOf(RE::TESForm* harvestable) {
        if (!harvestable) {
            return nullptr;
        }
        if (auto* flora = harvestable->As<RE::TESFlora>()) {
            return flora->produceItem;
        }
        if (auto* tree = harvestable->As<RE::TESObjectTREE>()) {
            return tree->produceItem;
        }
        return nullptr;
    }

    struct Pair {
        RE::TESForm* item = nullptr;
        RE::TESForm* flora = nullptr;
    };

    void ScanAndAppend() {
        if (!g_settings.enabled) {
            SKSE::log::info("Disabled in INI; doing nothing.");
            return;
        }
        if (!g_settings.scanHarvest) {
            SKSE::log::info("bScanHarvest=0; nothing else is implemented yet.");
            return;
        }

        auto* dh = RE::TESDataHandler::GetSingleton();
        if (!dh) {
            SKSE::log::error("No data handler. Aborting.");
            return;
        }

        // FIX (Kimi called this twice per candidate form, inside the loop - tens of
        // thousands of redundant lookups on a big load order): resolve once.
        auto* plantable = dh->LookupForm<RE::BGSListForm>(kPlantableBase, kHearthfire);
        auto* planted   = dh->LookupForm<RE::BGSListForm>(kPlantedBase, kHearthfire);
        if (!plantable || !planted) {
            SKSE::log::error("Could not resolve the Hearthfire planter FormLists. "
                             "Is HearthFires.esm loaded? Aborting without changes.");
            return;
        }

        const std::size_t before = plantable->forms.size();
        if (before != planted->forms.size()) {
            SKSE::log::error("Planter lists are already uneven ({} vs {}). Something else "
                             "broke them. Failing closed - no changes made.",
                             before, planted->forms.size());
            return;
        }

        // Existing contents, so we never add the same ingredient twice.
        std::unordered_set<RE::FormID> have;
        have.reserve(before * 2);
        for (auto* f : plantable->forms) {
            if (f) {
                have.insert(f->GetFormID());
            }
        }

        // ---- collect ------------------------------------------------------------
        // FIX (both earlier versions appended as they went, so a problem partway
        // through left the lists permanently uneven): gather everything first.
        std::vector<Pair> candidates;

        const auto consider = [&](RE::TESForm* harvestable) {
            auto* item = ProduceOf(harvestable);
            if (!item || !harvestable) {
                return;
            }
            if (Excluded(item) || Excluded(harvestable)) {
                return;
            }
            const auto id = item->GetFormID();
            if (g_settings.skipDuplicates && have.contains(id)) {
                return;
            }
            have.insert(id);
            candidates.push_back({item, harvestable});
        };

        for (auto* flora : dh->GetFormArray<RE::TESFlora>()) {
            consider(flora);
        }
        for (auto* tree : dh->GetFormArray<RE::TESObjectTREE>()) {
            consider(tree);
        }

        if (candidates.empty()) {
            SKSE::log::info("Nothing new to add; {} pairs already present.", before);
            return;
        }

        // ---- commit -------------------------------------------------------------
        for (const auto& p : candidates) {
            plantable->AddForm(p.item);
            planted->AddForm(p.flora);
        }

        const std::size_t after = plantable->forms.size();
        if (after != planted->forms.size()) {
            // Should be impossible given lockstep appends, but if it ever happens the
            // lists are paired by index and a mismatch means planting the wrong thing.
            SKSE::log::critical("Lists ended uneven ({} vs {}) after appending {} pairs. "
                                "Planters may misbehave this session - restart the game.",
                                after, planted->forms.size(), candidates.size());
            return;
        }

        SKSE::log::info("Added {} pairs. Planter lists: {} -> {}.",
                        candidates.size(), before, after);
    }
}

// FIX (this is the line that decides whether the plugin loads at all): Kimi pinned
// CompatibleVersions({ SKSE::RUNTIME_LATEST }), which locks the DLL to whatever runtime
// CommonLib called "latest" at compile time and makes SKSE refuse to load it on anything
// else - the single most likely reason it would end up parked like the others.
// UsesAddressLibrary() is the version-independent declaration NG plugins use instead.
//
// API SPELLING - this matters, and it is not guesswork. Jo's rebuild factory
// (ci/build-oif.ps1) had to patch exactly these two calls when it moved to the
// CommonLibSSE-NG 8.x commit that supports 1.7.104 / Address Library format 5:
//     v.UsesAddressLibrary(true)  ->  v.UsesAddressLibrary()   // no argument in 8.x
//     v.HasNoStructUse(true)      ->  v.UsesNoStructs()        // renamed in 8.x
// The 8.x spellings are used below because that is the CommonLib this will be built
// against. If you ever build against NG 3.7.x instead, swap them back.
//
// The block is a constexpr function, not an immediately-invoked lambda. constinit
// requires a constant initializer, and SKSE reads these bytes out of the DLL image.
constexpr SKSE::PluginVersionData MakePluginVersion() noexcept {
    SKSE::PluginVersionData v{};
    v.PluginName("AutoPlantEverything");
    v.PluginVersion(REL::Version(0, 2, 0, 0));
    v.AuthorName("Jo Hoeflich");
    v.UsesAddressLibrary();
    v.UsesNoStructs();
    return v;
}

extern "C" [[maybe_unused]] __declspec(dllexport) constinit SKSE::PluginVersionData SKSEPlugin_Version = MakePluginVersion();

// NG 8.x defines SKSEPluginLoad as a macro that expands to the real SKSEPlugin_Load
// export. Writing `bool SKSEPluginLoad(...)` after including SKSE.h is a syntax error.
SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SKSE::log::info("Auto Plant Everything 0.2.0 loaded.");

    LoadSettings();

    if (auto* messaging = SKSE::GetMessagingInterface()) {
        messaging->RegisterListener([](SKSE::MessagingInterface::Message* msg) {
            if (msg && msg->type == SKSE::MessagingInterface::kDataLoaded) {
                ScanAndAppend();
            }
        });
    } else {
        SKSE::log::error("No messaging interface; cannot hook data-loaded.");
        return false;
    }
    return true;
}
