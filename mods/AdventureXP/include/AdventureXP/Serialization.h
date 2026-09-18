#pragma once

#include <cstdint>
#include <unordered_set>

namespace SKSE
{
    class SerializationInterface;
}

namespace AdventureXP
{
    constexpr std::uint32_t kSerializeID = 'AXP1';
    constexpr std::uint32_t kSerializeVersion = 1;

    struct SaveState
    {
        bool snapshottedClears{ false };
        std::unordered_set<std::uint32_t> awardedClears;
        std::unordered_set<std::uint32_t> awardedDiscoveries;
        std::unordered_set<std::uint64_t> awardedQuestStages;
        std::unordered_set<std::uint32_t> awardedQuestCompletes;
        std::unordered_set<std::uint32_t> awardedBooks;
    };

    SaveState& State();
    void RegisterSerialization();
    void RevertState();
}
