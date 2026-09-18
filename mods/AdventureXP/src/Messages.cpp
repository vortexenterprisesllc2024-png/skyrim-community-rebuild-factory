#include "AdventureXP/PCH.h"
#include "AdventureXP/Messages.h"

namespace AdventureXP
{
    void Notify(std::string_view text)
    {
        if (text.empty()) {
            return;
        }

        // DebugNotification is the stock HUD toast (same channel as
        // "You discovered X" / skill-up text). No third-party SWF.
        const std::string copy{ text };
        SKSE::GetTaskInterface()->AddTask([copy]() {
            RE::DebugNotification(copy.c_str(), nullptr, true);
        });
    }
}
