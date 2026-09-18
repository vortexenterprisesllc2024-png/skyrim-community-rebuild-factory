#include "AdventureXP/PCH.h"
#include "AdventureXP/Messages.h"

namespace AdventureXP
{
    void Notify(std::string_view text)
    {
        if (text.empty()) {
            return;
        }

        // Stock HUD toast ("You discovered X" / skill-up channel). CommonLib
        // v8.0.1 does not export RE::DebugNotification; call the Address
        // Library AE/SE IDs directly. No third-party SWF.
        const std::string copy{ text };
        SKSE::GetTaskInterface()->AddTask([copy]() {
            using func_t = void (*)(const char*, const char*, bool);
            static REL::Relocation<func_t> notify{ REL::RelocationID(52050, 52933) };
            notify(copy.c_str(), nullptr, true);
        });
    }
}
