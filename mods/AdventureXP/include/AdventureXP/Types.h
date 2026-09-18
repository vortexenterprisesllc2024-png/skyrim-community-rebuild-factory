#pragma once

// Independent quest / place classifiers. Public Creation Kit type numbers and
// vanilla location keywords only — not anyone else's INI schema.

#include "AdventureXP/XpMath.h"

#include <cstdint>
#include <string_view>

namespace AdventureXP
{
    enum class QuestKind : std::uint8_t
    {
        Hidden,
        Main,
        Guild,
        Daedric,
        Side,
        Misc,
        CivilWar,
        DLC,
        Other
    };

    enum class PlaceKind : std::uint8_t
    {
        Default,
        City,
        Dungeon,
        Cave,
        Nordic,
        Dwemer,
        Fort,
        Camp,
        Dragon,
        Mine
    };

    struct QuestTypeWeights
    {
        float main{ 100.0f };
        float guild{ 100.0f };
        float daedric{ 100.0f };
        float side{ 100.0f };
        float misc{ 100.0f };
        float civilWar{ 100.0f };
        float dlc{ 100.0f };
        float other{ 100.0f };
    };

    struct PlaceTypeWeights
    {
        float def{ 100.0f };
        float city{ 100.0f };
        float dungeon{ 100.0f };
        float cave{ 100.0f };
        float nordic{ 100.0f };
        float dwemer{ 100.0f };
        float fort{ 100.0f };
        float camp{ 100.0f };
        float dragon{ 100.0f };
        float mine{ 100.0f };
    };

    inline QuestKind ClassifyQuestType(std::uint32_t questType)
    {
        switch (questType) {
        case 0:
            return QuestKind::Hidden;
        case 1:
            return QuestKind::Main;
        case 2:
        case 3:
        case 4:
        case 5:
            return QuestKind::Guild;
        case 6:
            return QuestKind::Misc;
        case 7:
            return QuestKind::Daedric;
        case 8:
            return QuestKind::Side;
        case 9:
            return QuestKind::CivilWar;
        case 10:
        case 11:
            return QuestKind::DLC;
        default:
            return QuestKind::Other;
        }
    }

    inline float WeightFor(QuestKind kind, const QuestTypeWeights& weights)
    {
        switch (kind) {
        case QuestKind::Main:
            return weights.main;
        case QuestKind::Guild:
            return weights.guild;
        case QuestKind::Daedric:
            return weights.daedric;
        case QuestKind::Side:
            return weights.side;
        case QuestKind::Misc:
            return weights.misc;
        case QuestKind::CivilWar:
            return weights.civilWar;
        case QuestKind::DLC:
            return weights.dlc;
        case QuestKind::Hidden:
            return 0.0f;
        case QuestKind::Other:
            return weights.other;
        }
        return weights.other;
    }

    inline PlaceKind ClassifyPlaceKeyword(std::string_view editorId)
    {
        if (editorId.empty()) {
            return PlaceKind::Default;
        }
        if (editorId == "LocTypeDragonLair") {
            return PlaceKind::Dragon;
        }
        if (editorId == "LocTypeNordicRuin" || editorId == "LocTypeDraugrCrypt") {
            return PlaceKind::Nordic;
        }
        if (editorId.find("Dwemer") != std::string_view::npos || editorId.find("Dwarven") != std::string_view::npos) {
            return PlaceKind::Dwemer;
        }
        if (editorId == "LocTypeMilitaryFort") {
            return PlaceKind::Fort;
        }
        if (editorId == "LocTypeBanditCamp" || editorId == "LocTypeForswornCamp" || editorId == "LocTypeCamp") {
            return PlaceKind::Camp;
        }
        if (editorId == "LocTypeMine") {
            return PlaceKind::Mine;
        }
        if (editorId == "LocTypeCave") {
            return PlaceKind::Cave;
        }
        if (editorId == "LocTypeCity" || editorId == "LocTypeTown" || editorId == "LocTypeSettlement") {
            return PlaceKind::City;
        }
        if (editorId == "LocTypeDungeon") {
            return PlaceKind::Dungeon;
        }
        return PlaceKind::Default;
    }

    inline PlaceKind Stronger(PlaceKind a, PlaceKind b)
    {
        const auto rank = [](PlaceKind k) {
            switch (k) {
            case PlaceKind::Dragon:
                return 9;
            case PlaceKind::Dwemer:
                return 8;
            case PlaceKind::Nordic:
                return 7;
            case PlaceKind::Fort:
                return 6;
            case PlaceKind::Mine:
                return 5;
            case PlaceKind::Cave:
                return 4;
            case PlaceKind::Dungeon:
                return 3;
            case PlaceKind::Camp:
                return 2;
            case PlaceKind::City:
                return 1;
            case PlaceKind::Default:
                return 0;
            }
            return 0;
        };
        return rank(a) >= rank(b) ? a : b;
    }

    inline float WeightFor(PlaceKind kind, const PlaceTypeWeights& weights)
    {
        switch (kind) {
        case PlaceKind::City:
            return weights.city;
        case PlaceKind::Dungeon:
            return weights.dungeon;
        case PlaceKind::Cave:
            return weights.cave;
        case PlaceKind::Nordic:
            return weights.nordic;
        case PlaceKind::Dwemer:
            return weights.dwemer;
        case PlaceKind::Fort:
            return weights.fort;
        case PlaceKind::Camp:
            return weights.camp;
        case PlaceKind::Dragon:
            return weights.dragon;
        case PlaceKind::Mine:
            return weights.mine;
        case PlaceKind::Default:
            return weights.def;
        }
        return weights.def;
    }
}
