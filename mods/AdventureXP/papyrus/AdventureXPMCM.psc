Scriptname AdventureXPMCM extends SKI_ConfigBase
{Clean-room SkyUI MCM for AdventureXP. Player options only.}

int Function GetVersion()
	return 4
endFunction

string PAGE_GENERAL = "General"
string PAGE_SOURCES = "XP Sources"
string PAGE_QUESTS = "Quest Types"
string PAGE_PLACES = "Places"

int _oidEnabled
int _oidPreset
int _oidGlobal

int[] _catOids
int[] _questOids
int[] _placeOids

string[] _catKeys
string[] _catLabels
string[] _questKeys
string[] _questLabels
string[] _placeKeys
string[] _placeLabels

string[] _presetIds
string[] _presetNames
int _presetCount = 0

event OnConfigInit()
	InitMenu()
endEvent

event OnVersionUpdate(int a_version)
	InitMenu()
endEvent

event OnConfigOpen()
	; Always refresh page list so old Weights / Questing names never stick empty
	InitMenu()
	CachePresets()
endEvent

Function InitMenu()
	ModName = "AdventureXP"

	Pages = new string[4]
	Pages[0] = PAGE_GENERAL
	Pages[1] = PAGE_SOURCES
	Pages[2] = PAGE_QUESTS
	Pages[3] = PAGE_PLACES

	_catKeys = new string[7]
	_catKeys[0] = "quest"
	_catKeys[1] = "discovery"
	_catKeys[2] = "clear"
	_catKeys[3] = "combat"
	_catKeys[4] = "reading"
	_catKeys[5] = "crafting"
	_catKeys[6] = "skillup"

	_catLabels = new string[7]
	_catLabels[0] = "Quests"
	_catLabels[1] = "Discovery"
	_catLabels[2] = "Clears"
	_catLabels[3] = "Combat"
	_catLabels[4] = "Reading"
	_catLabels[5] = "Crafting"
	_catLabels[6] = "Skill-Ups"

	_catOids = new int[7]

	_questKeys = new string[8]
	_questKeys[0] = "Main"
	_questKeys[1] = "Guild"
	_questKeys[2] = "Daedric"
	_questKeys[3] = "Side"
	_questKeys[4] = "Misc"
	_questKeys[5] = "CivilWar"
	_questKeys[6] = "DLC"
	_questKeys[7] = "Other"

	_questLabels = new string[8]
	_questLabels[0] = "Main Quest"
	_questLabels[1] = "Guild"
	_questLabels[2] = "Daedric"
	_questLabels[3] = "Side"
	_questLabels[4] = "Miscellaneous"
	_questLabels[5] = "Civil War"
	_questLabels[6] = "DLC"
	_questLabels[7] = "Other"

	_questOids = new int[8]

	_placeKeys = new string[10]
	_placeKeys[0] = "Default"
	_placeKeys[1] = "City"
	_placeKeys[2] = "Dungeon"
	_placeKeys[3] = "Cave"
	_placeKeys[4] = "Nordic"
	_placeKeys[5] = "Dwemer"
	_placeKeys[6] = "Fort"
	_placeKeys[7] = "Camp"
	_placeKeys[8] = "DragonLair"
	_placeKeys[9] = "Mine"

	_placeLabels = new string[10]
	_placeLabels[0] = "Default"
	_placeLabels[1] = "City"
	_placeLabels[2] = "Dungeon"
	_placeLabels[3] = "Cave"
	_placeLabels[4] = "Nordic"
	_placeLabels[5] = "Dwemer"
	_placeLabels[6] = "Fort"
	_placeLabels[7] = "Camp"
	_placeLabels[8] = "Dragon Lair"
	_placeLabels[9] = "Mine"

	_placeOids = new int[10]
endFunction

Function CachePresets()
	_presetCount = AdventureXP.GetPresetCount()
	if _presetCount < 0
		_presetCount = 0
	endIf
	if _presetCount > 16
		_presetCount = 16
	endIf

	_presetIds = Utility.CreateStringArray(16)
	_presetNames = Utility.CreateStringArray(16)

	int i = 0
	while i < _presetCount
		string id = AdventureXP.GetPresetId(i)
		_presetIds[i] = id
		_presetNames[i] = PrettyPresetName(id, AdventureXP.GetPresetName(i))
		i += 1
	endWhile
endFunction

string Function PrettyPresetName(string id, string raw)
	string key = id
	if key == ""
		key = raw
	endIf

	if key == "necromancer"
		return "Summoner"
	endIf
	if key == "summoner"
		return "Conjurer"
	endIf
	if key == "adventurer"
		return "Adventurer"
	endIf
	if key == "vigilant"
		return "Vigilant"
	endIf
	if key == "thief"
		return "Thief"
	endIf
	if key == "assassin"
		return "Assassin"
	endIf
	if key == "paladin"
		return "Paladin"
	endIf
	if key == "warrior"
		return "Warrior"
	endIf
	if key == "mage"
		return "Mage"
	endIf
	if key == "ranger"
		return "Ranger"
	endIf
	if key == "bard"
		return "Bard"
	endIf
	if key == "merchant"
		return "Merchant"
	endIf
	if key == "beastblood"
		return "Beastblood"
	endIf
	if key == "spellsword"
		return "Spellsword"
	endIf
	if key == "monk"
		return "Monk"
	endIf
	if key == "custom"
		return "Custom"
	endIf

	if raw != ""
		return raw
	endIf
	return id
endFunction

event OnPageReset(string a_page)
	; Current pages
	if a_page == "" || a_page == PAGE_GENERAL || a_page == "General"
		DrawGeneral()
		return
	endIf
	if a_page == PAGE_SOURCES || a_page == "XP Sources" || a_page == "Weights" || a_page == "Questing"
		; "Questing" legacy name was wrong bucket for sources in some builds — only Weights/XP Sources here
		if a_page == "Questing"
			DrawQuestTypes()
			return
		endIf
		DrawXpSources()
		return
	endIf
	if a_page == PAGE_QUESTS || a_page == "Quest Types" || a_page == "Quest types"
		DrawQuestTypes()
		return
	endIf
	if a_page == PAGE_PLACES || a_page == "Places" || a_page == "Exploring"
		DrawPlaces()
		return
	endIf

	; Fallback: never leave a blank page
	DrawGeneral()
endEvent

Function DrawGeneral()
	int flags = EnabledFlags()
	SetCursorFillMode(TOP_TO_BOTTOM)

	AddHeaderOption("General")
	_oidEnabled = AddToggleOption("Award Adventure XP", AdventureXP.GetEnabled())
	_oidGlobal = AddSliderOption("Global XP", ClampPct(AdventureXP.GetGlobalXPPercent()), "{0}%", flags)
	_oidPreset = AddMenuOption("Playstyle", ActivePresetLabel(), flags)
endFunction

Function DrawXpSources()
	int flags = EnabledFlags()
	SetCursorFillMode(TOP_TO_BOTTOM)
	AddHeaderOption("XP Sources")

	int i = 0
	while i < 7
		float value = ClampPct(AdventureXP.GetCategoryWeight(_catKeys[i]))
		_catOids[i] = AddSliderOption(_catLabels[i], value, "{0}%", flags)
		i += 1
	endWhile
endFunction

Function DrawQuestTypes()
	int flags = EnabledFlags()
	SetCursorFillMode(TOP_TO_BOTTOM)
	AddHeaderOption("Quest Types")

	int i = 0
	while i < 8
		float value = ClampPct(AdventureXP.GetQuestTypeWeight(_questKeys[i]))
		_questOids[i] = AddSliderOption(_questLabels[i], value, "{0}%", flags)
		i += 1
	endWhile
endFunction

Function DrawPlaces()
	int flags = EnabledFlags()
	SetCursorFillMode(TOP_TO_BOTTOM)
	AddHeaderOption("Places")

	int i = 0
	while i < 10
		float value = ClampPct(AdventureXP.GetPlaceTypeWeight(_placeKeys[i]))
		_placeOids[i] = AddSliderOption(_placeLabels[i], value, "{0}%", flags)
		i += 1
	endWhile
endFunction

event OnOptionSelect(int a_option)
	if a_option == _oidEnabled
		bool nextValue = !AdventureXP.GetEnabled()
		AdventureXP.SetEnabled(nextValue)
		SetToggleOptionValue(_oidEnabled, nextValue)
		; Do not ForcePageReset — that ate the next click. Grey/ungrey siblings in place.
		int flags = OPTION_FLAG_NONE
		if !nextValue
			flags = OPTION_FLAG_DISABLED
		endIf
		if _oidGlobal >= 0
			SetOptionFlags(_oidGlobal, flags)
		endIf
		if _oidPreset >= 0
			SetOptionFlags(_oidPreset, flags)
		endIf
	endIf
endEvent

event OnOptionSliderOpen(int a_option)
	if a_option == _oidGlobal
		OpenPercentSlider(ClampPct(AdventureXP.GetGlobalXPPercent()), 100.0)
		return
	endIf

	int cat = FindOid(_catOids, a_option)
	if cat >= 0
		OpenPercentSlider(ClampPct(AdventureXP.GetCategoryWeight(_catKeys[cat])), DefaultCategoryWeight(_catKeys[cat]))
		return
	endIf

	int quest = FindOid(_questOids, a_option)
	if quest >= 0
		OpenPercentSlider(ClampPct(AdventureXP.GetQuestTypeWeight(_questKeys[quest])), 100.0)
		return
	endIf

	int place = FindOid(_placeOids, a_option)
	if place >= 0
		OpenPercentSlider(ClampPct(AdventureXP.GetPlaceTypeWeight(_placeKeys[place])), 100.0)
	endIf
endEvent

event OnOptionSliderAccept(int a_option, float a_value)
	float value = ClampPct(a_value)

	if a_option == _oidGlobal
		AdventureXP.SetGlobalXPPercent(value)
		SetSliderOptionValue(a_option, value, "{0}%")
		return
	endIf

	int cat = FindOid(_catOids, a_option)
	if cat >= 0
		AdventureXP.SetCategoryWeight(_catKeys[cat], value)
		SetSliderOptionValue(a_option, value, "{0}%")
		return
	endIf

	int quest = FindOid(_questOids, a_option)
	if quest >= 0
		AdventureXP.SetQuestTypeWeight(_questKeys[quest], value)
		SetSliderOptionValue(a_option, value, "{0}%")
		return
	endIf

	int place = FindOid(_placeOids, a_option)
	if place >= 0
		AdventureXP.SetPlaceTypeWeight(_placeKeys[place], value)
		SetSliderOptionValue(a_option, value, "{0}%")
	endIf
endEvent

event OnOptionMenuOpen(int a_option)
	if a_option != _oidPreset
		return
	endIf

	CachePresets()
	if _presetCount <= 0
		string[] empty = new string[1]
		empty[0] = "No Packs Available"
		SetMenuDialogOptions(empty)
		SetMenuDialogStartIndex(0)
		SetMenuDialogDefaultIndex(0)
		return
	endIf

	string[] names = Utility.CreateStringArray(_presetCount)
	int i = 0
	while i < _presetCount
		names[i] = _presetNames[i]
		i += 1
	endWhile

	int start = FindPresetIndex(AdventureXP.GetActivePreset())
	if start < 0
		start = 0
	endIf

	SetMenuDialogOptions(names)
	SetMenuDialogStartIndex(start)
	SetMenuDialogDefaultIndex(0)
endEvent

event OnOptionMenuAccept(int a_option, int a_index)
	if a_option != _oidPreset
		return
	endIf
	if a_index < 0 || a_index >= _presetCount
		return
	endIf

	string id = _presetIds[a_index]
	bool ok = AdventureXP.ApplyPreset(id)
	if ok
		SetMenuOptionValue(a_option, _presetNames[a_index])
		ForcePageReset()
	else
		ShowMessage("Could Not Apply That Playstyle.", false, "$OK", "")
	endIf
endEvent

event OnOptionDefault(int a_option)
	if a_option == _oidEnabled
		AdventureXP.SetEnabled(true)
		SetToggleOptionValue(a_option, true)
		ForcePageReset()
		return
	endIf

	if a_option == _oidGlobal
		AdventureXP.SetGlobalXPPercent(100.0)
		SetSliderOptionValue(a_option, 100.0, "{0}%")
		return
	endIf

	if a_option == _oidPreset
		if AdventureXP.ApplyPreset("Adventurer")
			ForcePageReset()
		endIf
		return
	endIf

	int cat = FindOid(_catOids, a_option)
	if cat >= 0
		float value = DefaultCategoryWeight(_catKeys[cat])
		AdventureXP.SetCategoryWeight(_catKeys[cat], value)
		SetSliderOptionValue(a_option, value, "{0}%")
		return
	endIf

	int quest = FindOid(_questOids, a_option)
	if quest >= 0
		AdventureXP.SetQuestTypeWeight(_questKeys[quest], 100.0)
		SetSliderOptionValue(a_option, 100.0, "{0}%")
		return
	endIf

	int place = FindOid(_placeOids, a_option)
	if place >= 0
		AdventureXP.SetPlaceTypeWeight(_placeKeys[place], 100.0)
		SetSliderOptionValue(a_option, 100.0, "{0}%")
	endIf
endEvent

event OnOptionHighlight(int a_option)
	if a_option == _oidEnabled
		SetInfoText("Turns adventure XP awards on or off.")
	elseIf a_option == _oidPreset
		SetInfoText("Applies a playstyle pack to the XP source sliders.")
	elseIf a_option == _oidGlobal
		SetInfoText("Scales all adventure XP. 100% is normal.")
	else
		int cat = FindOid(_catOids, a_option)
		if cat >= 0
			SetInfoText("How much XP from " + _catLabels[cat] + ". 0% turns it off.")
			return
		endIf
		int quest = FindOid(_questOids, a_option)
		if quest >= 0
			SetInfoText("Scale for " + _questLabels[quest] + " quests. 100% is normal.")
			return
		endIf
		int place = FindOid(_placeOids, a_option)
		if place >= 0
			SetInfoText("Scale for " + _placeLabels[place] + ". 100% is normal.")
		endIf
	endIf
endEvent

int Function EnabledFlags()
	if AdventureXP.GetEnabled()
		return OPTION_FLAG_NONE
	endIf
	return OPTION_FLAG_DISABLED
endFunction

string Function ActivePresetLabel()
	string active = AdventureXP.GetActivePreset()
	if active == ""
		return "Custom"
	endIf
	int index = FindPresetIndex(active)
	if index >= 0
		return _presetNames[index]
	endIf
	return PrettyPresetName(active, active)
endFunction

int Function FindPresetIndex(string id)
	int i = 0
	while i < _presetCount
		if _presetIds[i] == id
			return i
		endIf
		i += 1
	endWhile
	return -1
endFunction

int Function FindOid(int[] oids, int option)
	if oids == none
		return -1
	endIf
	int i = 0
	int n = oids.Length
	while i < n
		if oids[i] == option
			return i
		endIf
		i += 1
	endWhile
	return -1
endFunction

Function OpenPercentSlider(float current, float defaultValue)
	SetSliderDialogStartValue(ClampPct(current))
	SetSliderDialogDefaultValue(ClampPct(defaultValue))
	SetSliderDialogRange(0.0, 100.0)
	SetSliderDialogInterval(1.0)
endFunction

float Function ClampPct(float value)
	if value < 0.0
		return 0.0
	endIf
	if value > 100.0
		return 100.0
	endIf
	return value
endFunction

float Function DefaultCategoryWeight(string key)
	if key == "combat" || key == "reading" || key == "crafting" || key == "skillup"
		return 0.0
	endIf
	return 100.0
endFunction