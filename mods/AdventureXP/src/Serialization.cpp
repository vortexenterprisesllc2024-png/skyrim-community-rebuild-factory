#include "AdventureXP/PCH.h"
#include "AdventureXP/Serialization.h"

namespace AdventureXP
{
    namespace
    {
        SaveState g_state;
        std::mutex g_mutex;

        template <class T>
        bool WriteSet(SKSE::SerializationInterface* intfc, const std::unordered_set<T>& set)
        {
            const auto count = static_cast<std::uint32_t>(set.size());
            if (!intfc->WriteRecordData(&count, sizeof(count))) {
                return false;
            }
            for (const auto value : set) {
                if (!intfc->WriteRecordData(&value, sizeof(value))) {
                    return false;
                }
            }
            return true;
        }

        template <class T>
        bool ReadSet(SKSE::SerializationInterface* intfc, std::unordered_set<T>& set)
        {
            set.clear();
            std::uint32_t count = 0;
            if (!intfc->ReadRecordData(&count, sizeof(count))) {
                return false;
            }
            for (std::uint32_t i = 0; i < count; ++i) {
                T value{};
                if (!intfc->ReadRecordData(&value, sizeof(value))) {
                    return false;
                }
                set.insert(value);
            }
            return true;
        }

        void Save(SKSE::SerializationInterface* intfc)
        {
            std::lock_guard lock(g_mutex);
            if (!intfc->OpenRecord('DATA', kSerializeVersion)) {
                logger::error("Failed to open serialization record");
                return;
            }

            const std::uint8_t snap = g_state.snapshottedClears ? 1 : 0;
            if (!intfc->WriteRecordData(&snap, sizeof(snap))) {
                return;
            }
            WriteSet(intfc, g_state.awardedClears);
            WriteSet(intfc, g_state.awardedDiscoveries);
            WriteSet(intfc, g_state.awardedQuestStages);
            WriteSet(intfc, g_state.awardedQuestCompletes);
            WriteSet(intfc, g_state.awardedBooks);
        }

        void Load(SKSE::SerializationInterface* intfc)
        {
            std::lock_guard lock(g_mutex);
            g_state = {};

            std::uint32_t type = 0;
            std::uint32_t version = 0;
            std::uint32_t length = 0;
            while (intfc->GetNextRecordInfo(type, version, length)) {
                if (type != 'DATA') {
                    continue;
                }
                std::uint8_t snap = 0;
                if (!intfc->ReadRecordData(&snap, sizeof(snap))) {
                    break;
                }
                g_state.snapshottedClears = snap != 0;
                ReadSet(intfc, g_state.awardedClears);
                ReadSet(intfc, g_state.awardedDiscoveries);
                ReadSet(intfc, g_state.awardedQuestStages);
                ReadSet(intfc, g_state.awardedQuestCompletes);
                ReadSet(intfc, g_state.awardedBooks);

                std::unordered_set<std::uint32_t> resolved;
                for (const auto oldID : g_state.awardedClears) {
                    std::uint32_t newID = 0;
                    if (intfc->ResolveFormID(oldID, newID)) {
                        resolved.insert(newID);
                    }
                }
                g_state.awardedClears = std::move(resolved);
            }
        }

        void Revert(SKSE::SerializationInterface*)
        {
            RevertState();
        }
    }

    SaveState& State()
    {
        return g_state;
    }

    void RevertState()
    {
        std::lock_guard lock(g_mutex);
        g_state = {};
    }

    void RegisterSerialization()
    {
        const auto* ser = SKSE::GetSerializationInterface();
        if (!ser) {
            logger::error("No serialization interface");
            return;
        }
        ser->SetUniqueID(kSerializeID);
        ser->SetSaveCallback(Save);
        ser->SetLoadCallback(Load);
        ser->SetRevertCallback(Revert);
    }
}
