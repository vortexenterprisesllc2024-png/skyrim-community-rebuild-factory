#!/usr/bin/env python3
"""AdventureXPMCM may call only AdventureXP.* natives."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MCM = ROOT / "papyrus" / "AdventureXPMCM.psc"
API = ROOT / "papyrus" / "AdventureXP.psc"

NATIVE_CALL = re.compile(r"\b([A-Za-z][A-Za-z0-9_]*)\s*\.\s*([A-Za-z][A-Za-z0-9_]*)\s*\(")
FUNCTION_DEF = re.compile(
    r"^\s*(?:(?:String|Int|Float|Bool)\s+)?Function\s+([A-Za-z][A-Za-z0-9_]*)",
    re.IGNORECASE,
)

API_NATIVES = (
    "GetVersion",
    "GetPresetCount",
    "GetPresetId",
    "GetPresetName",
    "GetActivePreset",
    "ApplyPreset",
    "GetCategoryWeight",
    "SetCategoryWeight",
    "GetGlobalXPPercent",
    "SetGlobalXPPercent",
    "GetEnabled",
    "SetEnabled",
    "GetShowMessages",
    "SetShowMessages",
    "GetAwardKilling",
    "SetAwardKilling",
    "GetAwardReading",
    "SetAwardReading",
    "GetQuestXP",
    "SetQuestXP",
    "GetDiscoveryXP",
    "SetDiscoveryXP",
    "GetClearXP",
    "SetClearXP",
    "GetQuestTypeWeight",
    "SetQuestTypeWeight",
    "GetPlaceTypeWeight",
    "SetPlaceTypeWeight",
    "GetSkillWeight",
    "SetSkillWeight",
)

REQUIRED_NATIVE_USES = (
    "GetPresetCount",
    "GetPresetId",
    "GetPresetName",
    "GetActivePreset",
    "ApplyPreset",
    "GetCategoryWeight",
    "SetCategoryWeight",
    "GetGlobalXPPercent",
    "SetGlobalXPPercent",
    "GetEnabled",
    "SetEnabled",
    "GetShowMessages",
    "SetShowMessages",
    "GetAwardKilling",
    "SetAwardKilling",
    "GetAwardReading",
    "SetAwardReading",
    "GetQuestXP",
    "SetQuestXP",
    "GetDiscoveryXP",
    "SetDiscoveryXP",
    "GetClearXP",
    "SetClearXP",
    "GetSkillWeight",
    "SetSkillWeight",
)

QUEST_KEYS = (
    "Objectives",
    "None",
    "Main",
    "College",
    "Thieves",
    "Brotherhood",
    "Companions",
    "Misc",
    "Daedric",
    "Side",
    "CivilWar",
    "Dawnguard",
    "Dragonborn",
)

SKILL_KEYS = (
    "Alteration",
    "Conjuration",
    "Destruction",
    "Illusion",
    "Restoration",
    "Enchanting",
    "OneHanded",
    "TwoHanded",
    "Archery",
    "Block",
    "HeavyArmor",
    "Smithing",
    "LightArmor",
    "Sneak",
    "Lockpicking",
    "Pickpocket",
    "Speech",
    "Alchemy",
)

PLACE_KEYS = (
    "Altar",
    "Camp",
    "Castle",
    "CastleKarstaag",
    "Cave",
    "City",
    "Clearing",
    "DaedricShrine",
    "Default",
    "Docks",
    "Doomstone",
    "DragonLair",
    "DwemerRuin",
    "Farm",
    "Fort",
    "GiantCamp",
    "Grove",
    "ImperialTower",
    "Landmark",
    "Lighthouse",
    "MilitaryCamp",
    "Mine",
    "MiraakTemple",
    "NordicDwelling",
    "NordicRuin",
    "NordicTower",
    "OrcStronghold",
    "Pass",
    "Rock",
    "Settlement",
    "Shack",
    "Shipwreck",
    "Smelter",
    "Stable",
    "StandingStone",
    "TelvanniTower",
    "Town",
    "WheatMill",
    "WoodMill",
)


def api_natives() -> set[str]:
    names: set[str] = set()
    for line in API.read_text(encoding="utf-8").splitlines():
        match = FUNCTION_DEF.match(line)
        if match and "Native" in line:
            names.add(match.group(1))
    return names


def test_mcm_only_uses_adventurexp_natives() -> None:
    text = MCM.read_text(encoding="utf-8")
    assert "Scriptname AdventureXPMCM extends SKI_ConfigBase" in text
    assert "Experience." not in text
    assert "ExperienceMCM" not in text

    natives = api_natives()
    assert natives == set(API_NATIVES), natives
    assert "int Function GetVersion()" in text
    assert "return 14" in text
    assert '_catLabels[5] = "Crafting"' in text
    assert "ToTitleCase(SplitCamel(_placeKeys[i]))" in text
    assert "Skills" in text
    assert "Magicka" in text
    assert "Combat" in text
    assert "Stealth" in text
    assert "Illusion" in text
    assert "One-handed" in text
    assert "SetSliderDialogInterval(1.0)" in text
    assert "XP Sources" in text
    assert "Quest Types" in text
    assert "Discovery" in text
    assert "Clears" in text
    assert "No packs available" in text
    assert 'return "Summoner"' in text
    assert 'return "Conjurer"' in text
    assert 'id == "Necromancer"' in text
    assert 'id == "Summoner"' in text
    assert "SetOptionFlags" in text
    assert "ApplyGeneralSiblingFlags" in text
    assert "ForcePageReset" not in text
    assert "SetOptionFlags(_oidEnabled" not in text

    for key in QUEST_KEYS:
        assert f'_questKeys' in text
        assert f'"{key}"' in text, key
    for key in PLACE_KEYS:
        assert f'"{key}"' in text, key
    for key in SKILL_KEYS:
        assert f'"{key}"' in text, key

    assert '"Guild"' not in text
    assert '"DLC"' not in text

    for banned in (
        "Reload needed",
        "How this saves",
        "Packs vs sliders",
        "AdventureXP.dll",
        "fGlobalXPPercent",
        "Plugin version",
        "(unavailable)",
    ):
        assert banned not in text, banned

    select_fn = re.search(r"event OnOptionSelect\(int a_option\)(?P<body>.*?)\nendEvent", text, re.S)
    default_fn = re.search(r"event OnOptionDefault\(int a_option\)(?P<body>.*?)\nendEvent", text, re.S)
    assert select_fn and default_fn
    for body in (select_fn.group("body"), default_fn.group("body")):
        enabled = re.search(r"if a_option == _oidEnabled\n(?P<body>.*?)\n\tendIf", body, re.S)
        assert enabled, "missing enable toggle handler"
        assert "SetToggleOptionValue" in enabled.group("body")
        assert "ApplyGeneralSiblingFlags" in enabled.group("body")
        assert "ForcePageReset" not in enabled.group("body")
        assert "SetOptionFlags(_oidEnabled" not in enabled.group("body")

    sibling_fn = re.search(
        r"Function ApplyGeneralSiblingFlags\(\)(?P<body>.*?)\nendFunction",
        text,
        re.S,
    )
    assert sibling_fn, "missing ApplyGeneralSiblingFlags"
    sibling = sibling_fn.group("body")
    assert "SetOptionFlags(_oidShowMessages, flags)" in sibling
    assert "SetOptionFlags(_oidGlobal, flags)" in sibling
    assert "SetOptionFlags(_oidPreset, flags)" in sibling
    assert "_oidEnabled" not in sibling
    assert "_oidKilling" not in sibling
    assert "_catOids" not in sibling

    foreign: list[str] = []
    used: set[str] = set()
    for qualifier, func in NATIVE_CALL.findall(text):
        if qualifier == "AdventureXP":
            used.add(func)
            if func not in natives:
                foreign.append(f"AdventureXP.{func} (not in AdventureXP.psc)")
        elif qualifier in {"Experience", "ExperienceMCM"}:
            foreign.append(f"{qualifier}.{func}")
    if foreign:
        raise AssertionError("forbidden or unknown native calls: " + ", ".join(foreign))
    missing = [name for name in REQUIRED_NATIVE_USES if name not in used]
    if missing:
        raise AssertionError("MCM never calls: " + ", ".join(missing))


def test_api_is_hidden_global() -> None:
    text = API.read_text(encoding="utf-8")
    assert text.startswith("Scriptname AdventureXP Hidden")
    assert "Experience." not in text


if __name__ == "__main__":
    test_mcm_only_uses_adventurexp_natives()
    test_api_is_hidden_global()
    print("test_mcm_scripts: ok")
    sys.exit(0)
