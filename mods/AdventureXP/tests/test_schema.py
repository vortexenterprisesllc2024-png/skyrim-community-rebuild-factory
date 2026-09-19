#!/usr/bin/env python3
"""INI, Types.h, MCM, and mapping doc stay in lockstep."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
INI = ROOT / "dist" / "SKSE" / "Plugins" / "AdventureXP.ini"
TYPES = ROOT / "include" / "AdventureXP" / "Types.h"
MCM = ROOT / "papyrus" / "AdventureXPMCM.psc"
MAP = ROOT / "docs" / "PLACE_MAPPINGS.md"

PLACE_KEYS = [
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
]

QUEST_KEYS = [
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
]

SKILL_KEYS = [
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
]


def ini_section(text: str, name: str) -> dict[str, str]:
    pattern = re.compile(rf"^\[{re.escape(name)}\]\s*$", re.I | re.M)
    match = pattern.search(text)
    if not match:
        raise AssertionError(f"missing INI section [{name}]")
    body = text[match.end() :]
    next_sec = re.search(r"^\[", body, re.M)
    if next_sec:
        body = body[: next_sec.start()]
    values: dict[str, str] = {}
    for line in body.splitlines():
        line = line.strip()
        if not line or line.startswith(";") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        values[key.strip()] = value.strip()
    return values


def test_ini_has_every_key() -> None:
    text = INI.read_text(encoding="utf-8")
    quests = ini_section(text, "QuestXP")
    discovery = ini_section(text, "Discovery")
    clears = ini_section(text, "Clears")
    general = ini_section(text, "General")
    assert general["bShowXPMessages"] in {"0", "1"}
    assert general["bAwardKilling"] in {"0", "1"}
    assert general["bAwardReading"] in {"0", "1"}
    for key in QUEST_KEYS:
        assert f"i{key}" in quests, key
        amount = int(quests[f"i{key}"])
        assert 0 <= amount <= 200, key
    for key in PLACE_KEYS:
        assert f"i{key}" in discovery, key
        assert f"i{key}" in clears, key
        assert 0 <= int(discovery[f"i{key}"]) <= 200, key
        assert 0 <= int(clears[f"i{key}"]) <= 200, key
    skills = ini_section(text, "Skills")
    assert len(SKILL_KEYS) == 18
    for key in SKILL_KEYS:
        assert f"f{key}" in skills, key
        weight = float(skills[f"f{key}"])
        assert 0 <= weight <= 100, key
    optional = ini_section(text, "Optional")
    assert optional["fReadingMult"] == "1.0"
    assert "fReadingXP" in optional
    assert optional["fSkillUpXP"] == "4.0"
    assert optional["fSkillUpLevelScale"] == "10.0"
    assert "floor(sqrt(goldValue) * fReadingMult)" in text
    assert "fSkillUpLevelScale" in text
    assert "ignores global" in text.lower()


def test_types_and_docs() -> None:
    header = TYPES.read_text(encoding="utf-8")
    mcm = MCM.read_text(encoding="utf-8")
    mapping = MAP.read_text(encoding="utf-8")
    for key in PLACE_KEYS:
        assert f'"{key}"' in header, key
        assert f'"{key}"' in mcm, key
        assert key in mapping, key
    for key in QUEST_KEYS:
        assert f'"{key}"' in header, key
        assert f'"{key}"' in mcm, key
    for key in SKILL_KEYS:
        assert f'"{key}"' in header, key
        assert f'"{key}"' in mcm, key
    assert "enum class SkillKind" in header
    assert "ParseSkill" in header
    assert "SkillFromActorValue" in header
    assert "GetSkillWeight" in mcm
    assert "SetSkillWeight" in mcm
    assert "PAGE_SKILLS" in mcm
    assert "LocTypeCity" in header
    assert "LocTypeDwarvenAutomatons" in header
    assert "CollegeofWinterholdFaction" in header
    assert "Dawnguard" in mapping
    assert "Dragonborn" in mapping
    assert "grouped" in mapping.lower()


if __name__ == "__main__":
    test_ini_has_every_key()
    test_types_and_docs()
    print("test_schema: ok")
    sys.exit(0)
