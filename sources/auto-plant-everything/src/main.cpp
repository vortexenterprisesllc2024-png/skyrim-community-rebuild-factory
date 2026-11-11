// Auto Plant Everything 1.0.0 - SKSE plugin (Skyrim SE/AE, Address Library)
//
// Makes everything harvestable in your load order plantable in Hearthfire-style
// planters - herbs, crops, mushrooms, eggs from nests, hanging moss, honeycomb -
// with no patch to rebuild when you add or remove mods. It rescans every time the
// game starts, in a few milliseconds, before the main menu.
//
// Order of work (all at startup, once per game launch):
//   1. Wait until every other mod has finished its startup edits (FormList
//      Manipulator INIs and similar), so their hand-made planter plants come first.
//   2. Merge the planter lists other mods ship in their plugins (e.g. Beyond
//      Skyrim's BSKflPlanterPlantableItem/PlantedFlora). Those use purpose-built
//      planter models, so they win over wild plants.
//   3. Scan every FLOR and TREE record. Anything that gives an ingredient or food
//      when harvested becomes plantable, using the most planter-like plant when
//      several give the same thing.
//
// The two Hearthfire lists are paired BY INDEX - plantable[i] grows planted[i] - so
// everything is collected and validated first, then appended in lockstep.

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// windows.h (pulled in by CommonLib) defines AddForm as AddFormA/AddFormW for the print spooler API.
#ifdef AddForm
#  undef AddForm
#endif

namespace {
    constexpr std::uint32_t kPlantableBase = 0x008247;  // flPlanterPlantableItem
    constexpr std::uint32_t kPlantedBase   = 0x008246;  // flPlanterPlantedFlora
    constexpr auto kHearthfire = "HearthFires.esm"sv;
    constexpr auto kVersion = "1.0.0"sv;

    struct Settings {
        bool enabled = true;
        bool scanHarvest = true;
        bool mergeModPlanterLists = true;
        bool skipTestRecords = true;
        bool includeFood = true;
        std::unordered_set<std::string> blacklist;         // EditorIDs, lowercased
        std::vector<std::string> excludeModelKeywords{     // lowercased substrings
            "deadanimals", "\\bottles\\", "winebottle", "mounted", "\\bones\\",
            "effects\\", "meadery", "skewer", "hagraven", "bucket", "wallbasket", "horkermeat"};
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

    std::vector<std::string> SplitList(const std::string& val) {
        std::vector<std::string> out;
        std::size_t start = 0;
        while (start <= val.size()) {
            const auto comma = val.find(',', start);
            auto item = Trim(val.substr(start, comma == std::string::npos ? std::string::npos : comma - start));
            if (!item.empty()) {
                out.push_back(ToLower(item));
            }
            if (comma == std::string::npos) break;
            start = comma + 1;
        }
        return out;
    }

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

            if (key == "benabled")                    g_settings.enabled = on;
            else if (key == "bscanharvest")           g_settings.scanHarvest = on;
            else if (key == "bmergemodplanterlists")  g_settings.mergeModPlanterLists = on;
            else if (key == "bskiptestrecords")       g_settings.skipTestRecords = on;
            else if (key == "bincludefood")           g_settings.includeFood = on;
            else if (key == "blacklist") {
                for (auto& s : SplitList(val)) g_settings.blacklist.insert(s);
            } else if (key == "excludemodelkeywords") {
                g_settings.excludeModelKeywords = SplitList(val);
            }
        }
        SKSE::log::info("Settings: enabled={} scanHarvest={} mergeModPlanterLists={} includeFood={} "
                        "blacklist={} excludeModelKeywords={}",
                        g_settings.enabled, g_settings.scanHarvest, g_settings.mergeModPlanterLists,
                        g_settings.includeFood, g_settings.blacklist.size(),
                        g_settings.excludeModelKeywords.size());
    }

    // ---- FormList helpers -----------------------------------------------------
    // A FormList holds plugin entries in `forms` and anything added at runtime
    // (Papyrus AddForm, FormList Manipulator, this plugin) in scriptAddedTempForms.
    // The planter script reads both, plugin entries first, so both must be counted.
    std::size_t ListSize(const RE::BGSListForm* list) {
        if (!list) return 0;
        return list->forms.size() +
               (list->scriptAddedTempForms ? list->scriptAddedTempForms->size() : 0);
    }

    template <class F>
    void ForEachEntry(const RE::BGSListForm* list, F&& fn) {
        if (!list) return;
        for (auto* f : list->forms) fn(f);
        if (list->scriptAddedTempForms) {
            for (const auto id : *list->scriptAddedTempForms) fn(RE::TESForm::LookupByID(id));
        }
    }

    std::vector<RE::TESForm*> Entries(const RE::BGSListForm* list) {
        std::vector<RE::TESForm*> v;
        ForEachEntry(list, [&](RE::TESForm* f) { v.push_back(f); });
        return v;
    }

    // ---- what counts as a plant -------------------------------------------------
    bool IsHarvestable(const RE::TESForm* f) {
        return f && (f->Is(RE::FormType::Flora) || f->Is(RE::FormType::Tree));
    }

    bool IsPlantableItem(const RE::TESForm* f) {
        if (!f) return false;
        if (f->Is(RE::FormType::Ingredient)) return true;
        return g_settings.includeFood && f->Is(RE::FormType::AlchemyItem);
    }

    RE::TESBoundObject* ProduceOf(RE::TESForm* harvestable) {
        if (!harvestable) return nullptr;
        if (auto* flora = harvestable->As<RE::TESFlora>()) return flora->produceItem;
        if (auto* tree = harvestable->As<RE::TESObjectTREE>()) return tree->produceItem;
        return nullptr;
    }

    std::string ModelOf(RE::TESForm* harvestable) {
        const char* path = nullptr;
        if (auto* flora = harvestable ? harvestable->As<RE::TESFlora>() : nullptr) {
            path = flora->GetModel();
        } else if (auto* tree = harvestable ? harvestable->As<RE::TESObjectTREE>() : nullptr) {
            path = tree->GetModel();
        }
        return (path && *path) ? ToLower(path) : std::string{};
    }

    std::string EditorIDOf(const RE::TESForm* form) {
        // EditorIDs exist at runtime only with powerofthree's Tweaks (or similar).
        // Everything still works without them; only the Blacklist needs them.
        const char* raw = form ? form->GetFormEditorID() : nullptr;
        return (raw && *raw) ? ToLower(raw) : std::string{};
    }

    std::string SourceOf(const RE::TESForm* form) {
        const auto* file = form ? form->GetFile(0) : nullptr;
        return file ? std::string(file->GetFilename()) : std::string("(unknown)");
    }

    bool Excluded(RE::TESForm* item, RE::TESForm* harvestable) {
        for (auto* f : {item, harvestable}) {
            const auto edid = EditorIDOf(f);
            if (edid.empty()) continue;
            if (g_settings.skipTestRecords && edid.rfind("test", 0) == 0) return true;
            if (g_settings.blacklist.contains(edid)) return true;
        }
        const auto model = ModelOf(harvestable);
        for (const auto& kw : g_settings.excludeModelKeywords) {
            if (!kw.empty() && model.find(kw) != std::string::npos) return true;
        }
        return false;
    }

    // Lower is better: planter-made versions first, snowy variants last.
    int Score(RE::TESForm* harvestable) {
        int s = 0;
        const auto edid = EditorIDOf(harvestable);
        if (edid.find("byoh") != std::string::npos || edid.find("planter") != std::string::npos ||
            edid.find("house") != std::string::npos) {
            s -= 100;
        }
        const auto model = ModelOf(harvestable);
        if (model.find("snow") != std::string::npos) s += 20;
        if (model.find("hanging") != std::string::npos) s += 5;  // fine, but prefer ground plants
        s += static_cast<int>(model.size() / 10);
        return s;
    }

    struct Pair {
        RE::TESForm* item = nullptr;
        RE::TESForm* flora = nullptr;
        std::string origin;
    };

    // ---- step 2: other mods' planter lists ------------------------------------
    // Finds FormList pairs shaped like the Hearthfire lists: same length, one all
    // ingredients/food, the other all FLOR/TREE, and most planted entries produce
    // the item at the same index. No EditorIDs needed.
    std::vector<Pair> CollectModPlanterLists(RE::TESDataHandler* dh, const RE::BGSListForm* hfItems,
                                             const RE::BGSListForm* hfFlora) {
        std::vector<Pair> out;
        std::map<std::size_t, std::vector<RE::BGSListForm*>> itemLists, floraLists;
        for (auto* list : dh->GetFormArray<RE::BGSListForm>()) {
            if (!list || list == hfItems || list == hfFlora) continue;
            const auto entries = Entries(list);
            if (entries.empty()) continue;
            const bool allItems = std::all_of(entries.begin(), entries.end(), IsPlantableItem);
            const bool allFlora = std::all_of(entries.begin(), entries.end(), IsHarvestable);
            if (allItems) itemLists[entries.size()].push_back(list);
            else if (allFlora) floraLists[entries.size()].push_back(list);
        }
        for (auto& [size, flist] : floraLists) {
            auto it = itemLists.find(size);
            if (it == itemLists.end()) continue;
            for (auto* fl : flist) {
                const auto fe = Entries(fl);
                RE::BGSListForm* best = nullptr;
                std::size_t bestHits = 0;
                for (auto* il : it->second) {
                    const auto ie = Entries(il);
                    std::size_t hits = 0;
                    for (std::size_t i = 0; i < size; ++i) {
                        if (ProduceOf(fe[i]) == ie[i]) ++hits;
                    }
                    if (hits > bestHits) { bestHits = hits; best = il; }
                }
                if (!best || bestHits * 10 < size * 6) continue;  // need 60% to trust the pairing
                const auto ie = Entries(best);
                SKSE::log::info("  Mod planter list pair: {} items + {} plants from {} ({} of {} match)",
                                SourceOf(best), SourceOf(fl), SourceOf(fl), bestHits, size);
                for (std::size_t i = 0; i < size; ++i) {
                    out.push_back({ie[i], fe[i], "planter list in " + SourceOf(fl)});
                }
            }
        }
        return out;
    }

    void ScanAndAppend() {
        if (!g_settings.enabled) {
            SKSE::log::info("Disabled in INI; doing nothing.");
            return;
        }
        auto* dh = RE::TESDataHandler::GetSingleton();
        if (!dh) {
            SKSE::log::error("No data handler. Aborting.");
            return;
        }
        auto* plantable = dh->LookupForm<RE::BGSListForm>(kPlantableBase, kHearthfire);
        auto* planted   = dh->LookupForm<RE::BGSListForm>(kPlantedBase, kHearthfire);
        if (!plantable || !planted) {
            SKSE::log::error("Could not find the Hearthfire planter lists. Is HearthFires.esm loaded? No changes made.");
            return;
        }

        const std::size_t before = ListSize(plantable);
        if (before != ListSize(planted)) {
            SKSE::log::error("Planter lists are already uneven ({} items vs {} plants) before this mod ran. "
                             "Another plugin overrides flPlanterPlantableItem/flPlanterPlantedFlora with lists "
                             "of different lengths, so planters would grow the wrong plant. No changes made. "
                             "Check which plugin wins those two FormLists in xEdit and disable or fix it.",
                             before, ListSize(planted));
            return;
        }

        std::unordered_set<RE::FormID> have;
        ForEachEntry(plantable, [&](RE::TESForm* f) { if (f) have.insert(f->GetFormID()); });

        std::vector<Pair> chosen;
        std::unordered_map<std::string, std::size_t> bySource;

        // step 2: other mods' own planter lists keep their purpose-built plants
        if (g_settings.mergeModPlanterLists) {
            for (auto& p : CollectModPlanterLists(dh, plantable, planted)) {
                if (!p.item || !p.flora || have.contains(p.item->GetFormID())) continue;
                if (Excluded(p.item, p.flora)) continue;
                have.insert(p.item->GetFormID());
                ++bySource[p.origin];
                chosen.push_back(p);
            }
        }

        // step 3: everything harvestable in the world
        if (g_settings.scanHarvest) {
            std::unordered_map<RE::FormID, std::pair<RE::TESForm*, int>> best;  // item -> (plant, score)
            std::unordered_map<RE::FormID, RE::TESForm*> itemForms;
            const auto consider = [&](RE::TESForm* harvestable) {
                auto* item = ProduceOf(harvestable);
                if (!item || !IsPlantableItem(item)) return;
                if (have.contains(item->GetFormID())) return;
                if (Excluded(item, harvestable)) return;
                const int score = Score(harvestable);
                auto it = best.find(item->GetFormID());
                if (it == best.end() || score < it->second.second) {
                    best[item->GetFormID()] = {harvestable, score};
                    itemForms[item->GetFormID()] = item;
                }
            };
            for (auto* flora : dh->GetFormArray<RE::TESFlora>()) consider(flora);
            for (auto* tree : dh->GetFormArray<RE::TESObjectTREE>()) consider(tree);

            std::vector<Pair> scanned;
            for (auto& [id, pick] : best) {
                scanned.push_back({itemForms[id], pick.first, SourceOf(pick.first)});
            }
            std::sort(scanned.begin(), scanned.end(), [](const Pair& a, const Pair& b) {
                return a.flora->GetFormID() < b.flora->GetFormID();
            });
            for (auto& p : scanned) {
                have.insert(p.item->GetFormID());
                ++bySource[p.origin];
                chosen.push_back(p);
            }
        }

        if (chosen.empty()) {
            SKSE::log::info("Nothing new to add; {} plants already plantable.", before);
            return;
        }

        for (const auto& p : chosen) {
            plantable->AddForm(p.item);
            planted->AddForm(p.flora);
        }

        const std::size_t after = ListSize(plantable);
        if (after != ListSize(planted)) {
            SKSE::log::critical("Lists ended uneven ({} vs {}) after adding {} pairs. Planters may grow the "
                                "wrong plant this session. Please report this with your load order.",
                                after, ListSize(planted), chosen.size());
            return;
        }

        for (const auto& [src, n] : bySource) {
            SKSE::log::info("  {:4} from {}", n, src);
        }
        SKSE::log::info("Added {} plantable plants. Planter lists: {} -> {}.", chosen.size(), before, after);
    }
}

// Name, version (from CMake project VERSION), author and Address Library use are declared by
// add_commonlibsse_plugin() in the generated __AutoPlantEverythingPlugin.cpp.

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SKSE::log::info("Auto Plant Everything {} loaded.", kVersion);
    LoadSettings();

    auto* messaging = SKSE::GetMessagingInterface();
    if (!messaging) {
        SKSE::log::error("No messaging interface; cannot hook data-loaded.");
        return false;
    }
    messaging->RegisterListener([](SKSE::MessagingInterface::Message* msg) {
        if (msg && msg->type == SKSE::MessagingInterface::kDataLoaded) {
            // Run on the next main-thread task, i.e. after every other plugin has
            // handled kDataLoaded (FormList Manipulator INIs etc.), so their
            // hand-made planter plants are already in the lists and win.
            if (auto* tasks = SKSE::GetTaskInterface()) {
                tasks->AddTask([]() { ScanAndAppend(); });
            } else {
                ScanAndAppend();
            }
        }
    });
    return true;
}
