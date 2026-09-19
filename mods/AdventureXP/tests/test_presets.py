#!/usr/bin/env python3
"""PlayStylePack keeps Taquitos RP deny zeros on the expanded lists."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PRESETS = ROOT / "include" / "AdventureXP" / "Presets.h"
INI = ROOT / "dist" / "SKSE" / "Plugins" / "AdventureXP.ini"
README = ROOT / "README.md"

PACKS = (
    "Adventurer",
    "Vigilant",
    "Summoner",
    "Thief",
    "Assassin",
    "Paladin",
    "Warrior",
    "Mage",
    "Ranger",
    "Bard",
    "Merchant",
    "Necromancer",
    "Beastblood",
    "Spellsword",
    "Monk",
    "Illusionist",
    "elementalist",
    "Battlemage",
    "Crafter",
)


def test_presets_header() -> None:
    text = PRESETS.read_text(encoding="utf-8")
    assert "struct PlayStylePack" in text
    assert "struct QuestTypeWeights" in text
    assert "struct PlaceTypeWeights" in text
    assert "struct CategoryWeights" in text
    assert "kPresets" in text
    assert ".daedric = 0" in text
    assert text.count(".daedric = 0") >= 2
    assert "kMerchant.categories.combat == 0" in text
    symbols = {"elementalist": "Elementalist"}
    for pack in PACKS:
        assert f'"{pack}"' in text, pack
        symbol = symbols.get(pack, pack)
        block = re.search(
            rf"// {re.escape(symbol)} —(?P<body>.*?)inline constexpr PlayStylePack k{symbol}",
            text,
            re.S,
        )
        assert block, f"missing pays/denies comment for {pack}"
        body = block.group("body").lower()
        assert "pays:" in body, pack
        assert "denies:" in body, pack
    assert text.count("inline constexpr PlayStylePack k") == 19
    assert "kIllusionist.skills[SkillKind::Illusion] == 100.f" in text
    assert "kIllusionist.skills[SkillKind::Alteration] == 0.f" in text
    assert "kVigilant.quests.daedric == 0" in text


def test_ini_and_readme_list_packs() -> None:
    ini = INI.read_text(encoding="utf-8")
    readme = README.read_text(encoding="utf-8")
    for pack in PACKS:
        assert pack in ini, pack
        assert pack in readme, pack
    assert "Conjurer" in readme
    assert "Conjurer" in ini


if __name__ == "__main__":
    test_presets_header()
    test_ini_and_readme_list_packs()
    print("test_presets: ok")
    sys.exit(0)
