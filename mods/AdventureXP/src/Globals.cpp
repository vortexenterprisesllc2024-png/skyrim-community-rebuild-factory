#include "AdventureXP/PCH.h"
#include "AdventureXP/Globals.h"

namespace AdventureXP
{
    namespace
    {
        RE::TESGlobal* g_percent{ nullptr };

        RE::TESGlobal* LookupPercent()
        {
            if (g_percent) {
                return g_percent;
            }
            g_percent = RE::TESForm::LookupByEditorID<RE::TESGlobal>("AdventureXP_Percent");
            return g_percent;
        }
    }

    void BindGlobals()
    {
        if (LookupPercent()) {
            logger::info("Bound TESGlobal AdventureXP_Percent from AdventureXP.esl");
        } else {
            logger::warn("AdventureXP_Percent global not found — ESL missing or not loaded. Plugin still awards XP.");
        }
    }

    void SetPercentGlobal(float percent)
    {
        if (auto* glob = LookupPercent()) {
            glob->value = percent;
        }
    }
}
