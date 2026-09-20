Scriptname AdventureXPMCM extends SKI_ConfigBase
{Clean-room SkyUI MCM for AdventureXP 4.2.

Calls only AdventureXP.* natives. Do not add third-party XP plugin calls.}

int Function GetVersion()
	; 15 = Global XP reserved (unused for awards); XP Sources sliders are the scale.
	return 15
endFunction

string PAGE_GENERAL = "General"
string PAGE_SOURCES = "XP Sources"
string PAGE_QUESTS = "Quest Types"
string PAGE_DISCOVERY = "Discovery"
string PAGE_CLEARS = "Clears"
string PAGE_SKILLS = "Skills"

int _oidEnabled
int _oidShowMessages
int _oidPreset
int _oidGlobal
int _oidKilling
int _oidReading

int[] _catOids
int[] _questOids
int[] _discOids
int[] _clearOids
int[] _skillOids

string[] _catKeys
string[] _catLabels
string[] _questKeys
string[] _questLabels
string[] _placeKeys
string[] _skillKeys
string[] _skillLabels

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
	if Pages == none
		InitMenu()
	endIf
	CachePresets()
endEvent

Function InitMenu()
	ModName = "AdventureXP"

	Pages = new string[6]
	Pages[0] = PAGE_GENERAL
	Pages[1] = PAGE_SOURCES
	Pages[2] = PAGE_QUESTS
	Pages[3] = PAGE_DISCOVERY
	Pages[4] = PAGE_CLEARS
	Pages[5] = PAGE_SKILLS

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
	_catLabels[6] = "Skill-ups"

	_catOids = new int[7]

	_questKeys = new string[13]
	_questKeys[0] = "Objectives"
	_questKeys[1] = "None"
	_questKeys[2] = "Main"
	_questKeys[3] = "College"
	_questKeys[4] = "Thieves"
	_questKeys[5] = "Brotherhood"
	_questKeys[6] = "Companions"
	_questKeys[7] = "Misc"
	_questKeys[8] = "Daedric"
	_questKeys[9] = "Side"
	_questKeys[10] = "CivilWar"
	_questKeys[11] = "Dawnguard"
	_questKeys[12] = "Dragonborn"

	_questLabels = new string[13]
	_questLabels[0] = "Quest Objectives"
	_questLabels[1] = "Quest None"
	_questLabels[2] = "Main"
	_questLabels[3] = "College"
	_questLabels[4] = "Thieves"
	_questLabels[5] = "Brotherhood"
	_questLabels[6] = "Companions"
	_questLabels[7] = "Misc"
	_questLabels[8] = "Daedric"
	_questLabels[9] = "Side"
	_questLabels[10] = "Civil War"
	_questLabels[11] = "Dawnguard"
	_questLabels[12] = "Dragonborn"

	_questOids = new int[13]

	_placeKeys = new string[39]
	_placeKeys[0] = "Altar"
	_placeKeys[1] = "Camp"
	_placeKeys[2] = "Castle"
	_placeKeys[3] = "CastleKarstaag"
	_placeKeys[4] = "Cave"
	_placeKeys[5] = "City"
	_placeKeys[6] = "Clearing"
	_placeKeys[7] = "DaedricShrine"
	_placeKeys[8] = "Default"
	_placeKeys[9] = "Docks"
	_placeKeys[10] = "Doomstone"
	_placeKeys[11] = "DragonLair"
	_placeKeys[12] = "DwemerRuin"
	_placeKeys[13] = "Farm"
	_placeKeys[14] = "Fort"
	_placeKeys[15] = "GiantCamp"
	_placeKeys[16] = "Grove"
	_placeKeys[17] = "ImperialTower"
	_placeKeys[18] = "Landmark"
	_placeKeys[19] = "Lighthouse"
	_placeKeys[20] = "MilitaryCamp"
	_placeKeys[21] = "Mine"
	_placeKeys[22] = "MiraakTemple"
	_placeKeys[23] = "NordicDwelling"
	_placeKeys[24] = "NordicRuin"
	_placeKeys[25] = "NordicTower"
	_placeKeys[26] = "OrcStronghold"
	_placeKeys[27] = "Pass"
	_placeKeys[28] = "Rock"
	_placeKeys[29] = "Settlement"
	_placeKeys[30] = "Shack"
	_placeKeys[31] = "Shipwreck"
	_placeKeys[32] = "Smelter"
	_placeKeys[33] = "Stable"
	_placeKeys[34] = "StandingStone"
	_placeKeys[35] = "TelvanniTower"
	_placeKeys[36] = "Town"
	_placeKeys[37] = "WheatMill"
	_placeKeys[38] = "WoodMill"

	_discOids = new int[39]
	_clearOids = new int[39]

	_skillKeys = new string[18]
	_skillKeys[0] = "Alteration"
	_skillKeys[1] = "Conjuration"
	_skillKeys[2] = "Destruction"
	_skillKeys[3] = "Illusion"
	_skillKeys[4] = "Restoration"
	_skillKeys[5] = "Enchanting"
	_skillKeys[6] = "OneHanded"
	_skillKeys[7] = "TwoHanded"
	_skillKeys[8] = "Archery"
	_skillKeys[9] = "Block"
	_skillKeys[10] = "HeavyArmor"
	_skillKeys[11] = "Smithing"
	_skillKeys[12] = "LightArmor"
	_skillKeys[13] = "Sneak"
	_skillKeys[14] = "Lockpicking"
	_skillKeys[15] = "Pickpocket"
	_skillKeys[16] = "Speech"
	_skillKeys[17] = "Alchemy"

	_skillLabels = new string[18]
	_skillLabels[0] = "Alteration"
	_skillLabels[1] = "Conjuration"
	_skillLabels[2] = "Destruction"
	_skillLabels[3] = "Illusion"
	_skillLabels[4] = "Restoration"
	_skillLabels[5] = "Enchanting"
	_skillLabels[6] = "One-handed"
	_skillLabels[7] = "Two-handed"
	_skillLabels[8] = "Archery"
	_skillLabels[9] = "Block"
	_skillLabels[10] = "Heavy Armor"
	_skillLabels[11] = "Smithing"
	_skillLabels[12] = "Light Armor"
	_skillLabels[13] = "Sneak"
	_skillLabels[14] = "Lockpicking"
	_skillLabels[15] = "Pickpocket"
	_skillLabels[16] = "Speech"
	_skillLabels[17] = "Alchemy"

	_skillOids = new int[18]
endFunction

Function CachePresets()
	_presetCount = AdventureXP.GetPresetCount()
	if _presetCount < 0
		_presetCount = 0
	endIf
	if _presetCount > 24
		_presetCount = 24
	endIf

	_presetIds = Utility.CreateStringArray(24)
	_presetNames = Utility.CreateStringArray(24)

	int i = 0
	while i < _presetCount
		string id = AdventureXP.GetPresetId(i)
		_presetIds[i] = id
		_presetNames[i] = DisplayPresetName(id, AdventureXP.GetPresetName(i))
		i += 1
	endWhile
endFunction

event OnPageReset(string a_page)
	if a_page == "" || a_page == PAGE_GENERAL
		DrawGeneral()
	elseIf a_page == PAGE_SOURCES
		DrawSources()
	elseIf a_page == PAGE_QUESTS
		DrawQuests()
	elseIf a_page == PAGE_DISCOVERY
		DrawDiscovery()
	elseIf a_page == PAGE_CLEARS
		DrawClears()
	elseIf a_page == PAGE_SKILLS
		DrawSkills()
	endIf
endEvent

Function DrawGeneral()
	int flags = EnabledFlags()
	SetCursorFillMode(TOP_TO_BOTTOM)

	_oidEnabled = AddToggleOption("Award Adventure XP", AdventureXP.GetEnabled())
	_oidShowMessages = AddToggleOption("Show Messages", AdventureXP.GetShowMessages(), flags)
	_oidGlobal = AddSliderOption("Global XP", AdventureXP.GetGlobalXPPercent(), "{0}%", flags)
	_oidPreset = AddMenuOption("Playstyle Pack", ActivePresetLabel(), flags)
endFunction

Function DrawSources()
	int flags = EnabledFlags()
	SetCursorFillMode(TOP_TO_BOTTOM)

	_oidKilling = AddToggleOption("Enable Killing", AdventureXP.GetAwardKilling(), flags)
	_oidReading = AddToggleOption("Enable Reading", AdventureXP.GetAwardReading(), flags)

	int i = 0
	while i < 7
		int sliderFlags = flags
		if i == 3 && !AdventureXP.GetAwardKilling()
			sliderFlags = OPTION_FLAG_DISABLED
		endIf
		if i == 4 && !AdventureXP.GetAwardReading()
			sliderFlags = OPTION_FLAG_DISABLED
		endIf
		float value = AdventureXP.GetCategoryWeight(_catKeys[i])
		_catOids[i] = AddSliderOption(_catLabels[i], value, "{0}%", sliderFlags)
		i += 1
	endWhile
endFunction

Function DrawQuests()
	int flags = EnabledFlags()
	SetCursorFillMode(TOP_TO_BOTTOM)

	int i = 0
	while i < 13
		int value = AdventureXP.GetQuestXP(_questKeys[i])
		_questOids[i] = AddSliderOption(_questLabels[i], value, "{0}", flags)
		i += 1
	endWhile
endFunction

Function DrawDiscovery()
	int flags = EnabledFlags()
	SetCursorFillMode(TOP_TO_BOTTOM)
	AddHeaderOption("Discover")

	int i = 0
	while i < 20
		int value = AdventureXP.GetDiscoveryXP(_placeKeys[i])
		_discOids[i] = AddSliderOption(ToTitleCase(SplitCamel(_placeKeys[i])), value, "{0}", flags)
		i += 1
	endWhile

	SetCursorPosition(1)
	AddHeaderOption("Discover")
	while i < 39
		int value = AdventureXP.GetDiscoveryXP(_placeKeys[i])
		_discOids[i] = AddSliderOption(ToTitleCase(SplitCamel(_placeKeys[i])), value, "{0}", flags)
		i += 1
	endWhile
endFunction

Function DrawClears()
	int flags = EnabledFlags()
	SetCursorFillMode(TOP_TO_BOTTOM)
	AddHeaderOption("Clear")

	int i = 0
	while i < 20
		int value = AdventureXP.GetClearXP(_placeKeys[i])
		_clearOids[i] = AddSliderOption(ToTitleCase(SplitCamel(_placeKeys[i])), value, "{0}", flags)
		i += 1
	endWhile

	SetCursorPosition(1)
	AddHeaderOption("Clear")
	while i < 39
		int value = AdventureXP.GetClearXP(_placeKeys[i])
		_clearOids[i] = AddSliderOption(ToTitleCase(SplitCamel(_placeKeys[i])), value, "{0}", flags)
		i += 1
	endWhile
endFunction

Function DrawSkills()
	int flags = EnabledFlags()
	SetCursorFillMode(TOP_TO_BOTTOM)
	AddHeaderOption("Magicka")
	int i = 0
	while i < 6
		float value = AdventureXP.GetSkillWeight(_skillKeys[i])
		_skillOids[i] = AddSliderOption(_skillLabels[i], value, "{0}%", flags)
		i += 1
	endWhile

	SetCursorPosition(1)
	AddHeaderOption("Combat")
	while i < 12
		float value = AdventureXP.GetSkillWeight(_skillKeys[i])
		_skillOids[i] = AddSliderOption(_skillLabels[i], value, "{0}%", flags)
		i += 1
	endWhile
	AddHeaderOption("Stealth")
	while i < 18
		float value = AdventureXP.GetSkillWeight(_skillKeys[i])
		_skillOids[i] = AddSliderOption(_skillLabels[i], value, "{0}%", flags)
		i += 1
	endWhile
endFunction

event OnOptionSelect(int a_option)
	if a_option == _oidEnabled
		bool nextValue = !AdventureXP.GetEnabled()
		AdventureXP.SetEnabled(nextValue)
		SetToggleOptionValue(a_option, nextValue)
		ApplyGeneralSiblingFlags()
		return
	endIf

	if a_option == _oidShowMessages
		bool nextValue = !AdventureXP.GetShowMessages()
		AdventureXP.SetShowMessages(nextValue)
		SetToggleOptionValue(a_option, nextValue)
		return
	endIf

	if a_option == _oidKilling
		bool nextValue = !AdventureXP.GetAwardKilling()
		AdventureXP.SetAwardKilling(nextValue)
		SetToggleOptionValue(a_option, nextValue)
		SetOptionFlags(_catOids[3], SourceSliderFlags("combat"))
		return
	endIf

	if a_option == _oidReading
		bool nextValue = !AdventureXP.GetAwardReading()
		AdventureXP.SetAwardReading(nextValue)
		SetToggleOptionValue(a_option, nextValue)
		SetOptionFlags(_catOids[4], SourceSliderFlags("reading"))
	endIf
endEvent

event OnOptionSliderOpen(int a_option)
	if a_option == _oidGlobal
		OpenPercentSlider(AdventureXP.GetGlobalXPPercent(), 100.0)
		return
	endIf

	int cat = FindOid(_catOids, a_option)
	if cat >= 0
		OpenPercentSlider(AdventureXP.GetCategoryWeight(_catKeys[cat]), DefaultCategoryWeight(_catKeys[cat]))
		return
	endIf

	int quest = FindOid(_questOids, a_option)
	if quest >= 0
		OpenAmountSlider(AdventureXP.GetQuestXP(_questKeys[quest]), DefaultQuestXP(_questKeys[quest]))
		return
	endIf

	int disc = FindOid(_discOids, a_option)
	if disc >= 0
		OpenAmountSlider(AdventureXP.GetDiscoveryXP(_placeKeys[disc]), DefaultDiscoveryXP(_placeKeys[disc]))
		return
	endIf

	int clear = FindOid(_clearOids, a_option)
	if clear >= 0
		OpenAmountSlider(AdventureXP.GetClearXP(_placeKeys[clear]), DefaultClearXP(_placeKeys[clear]))
		return
	endIf

	int skill = FindOid(_skillOids, a_option)
	if skill >= 0
		OpenPercentSlider(AdventureXP.GetSkillWeight(_skillKeys[skill]), 100.0)
	endIf
endEvent

event OnOptionSliderAccept(int a_option, float a_value)
	if a_option == _oidGlobal
		AdventureXP.SetGlobalXPPercent(a_value)
		SetSliderOptionValue(a_option, a_value, "{0}%")
		return
	endIf

	int cat = FindOid(_catOids, a_option)
	if cat >= 0
		AdventureXP.SetCategoryWeight(_catKeys[cat], a_value)
		SetSliderOptionValue(a_option, a_value, "{0}%")
		return
	endIf

	int quest = FindOid(_questOids, a_option)
	if quest >= 0
		AdventureXP.SetQuestXP(_questKeys[quest], a_value as int)
		SetSliderOptionValue(a_option, a_value, "{0}")
		return
	endIf

	int disc = FindOid(_discOids, a_option)
	if disc >= 0
		AdventureXP.SetDiscoveryXP(_placeKeys[disc], a_value as int)
		SetSliderOptionValue(a_option, a_value, "{0}")
		return
	endIf

	int clear = FindOid(_clearOids, a_option)
	if clear >= 0
		AdventureXP.SetClearXP(_placeKeys[clear], a_value as int)
		SetSliderOptionValue(a_option, a_value, "{0}")
		return
	endIf

	int skill = FindOid(_skillOids, a_option)
	if skill >= 0
		AdventureXP.SetSkillWeight(_skillKeys[skill], a_value)
		SetSliderOptionValue(a_option, a_value, "{0}%")
	endIf
endEvent

event OnOptionMenuOpen(int a_option)
	if a_option != _oidPreset
		return
	endIf

	CachePresets()
	if _presetCount <= 0
		string[] empty = new string[1]
		empty[0] = "No packs available"
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
	if AdventureXP.ApplyPreset(id)
		SetMenuOptionValue(a_option, _presetNames[a_index])
	else
		ShowMessage("Could not apply that pack.", false, "$OK", "")
	endIf
endEvent

event OnOptionDefault(int a_option)
	if a_option == _oidEnabled
		AdventureXP.SetEnabled(true)
		SetToggleOptionValue(a_option, true)
		ApplyGeneralSiblingFlags()
		return
	endIf

	if a_option == _oidShowMessages
		AdventureXP.SetShowMessages(true)
		SetToggleOptionValue(a_option, true)
		return
	endIf

	if a_option == _oidKilling
		AdventureXP.SetAwardKilling(false)
		SetToggleOptionValue(a_option, false)
		SetOptionFlags(_catOids[3], SourceSliderFlags("combat"))
		return
	endIf

	if a_option == _oidReading
		AdventureXP.SetAwardReading(false)
		SetToggleOptionValue(a_option, false)
		SetOptionFlags(_catOids[4], SourceSliderFlags("reading"))
		return
	endIf

	if a_option == _oidGlobal
		AdventureXP.SetGlobalXPPercent(100.0)
		SetSliderOptionValue(a_option, 100.0, "{0}%")
		return
	endIf

	if a_option == _oidPreset
		if AdventureXP.ApplyPreset("Adventurer")
			SetMenuOptionValue(a_option, DisplayPresetName("Adventurer", "Adventurer"))
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
		int value = DefaultQuestXP(_questKeys[quest])
		AdventureXP.SetQuestXP(_questKeys[quest], value)
		SetSliderOptionValue(a_option, value, "{0}")
		return
	endIf

	int disc = FindOid(_discOids, a_option)
	if disc >= 0
		int value = DefaultDiscoveryXP(_placeKeys[disc])
		AdventureXP.SetDiscoveryXP(_placeKeys[disc], value)
		SetSliderOptionValue(a_option, value, "{0}")
		return
	endIf

	int clear = FindOid(_clearOids, a_option)
	if clear >= 0
		int value = DefaultClearXP(_placeKeys[clear])
		AdventureXP.SetClearXP(_placeKeys[clear], value)
		SetSliderOptionValue(a_option, value, "{0}")
		return
	endIf

	int skill = FindOid(_skillOids, a_option)
	if skill >= 0
		AdventureXP.SetSkillWeight(_skillKeys[skill], 100.0)
		SetSliderOptionValue(a_option, 100.0, "{0}%")
	endIf
endEvent

event OnOptionHighlight(int a_option)
	if a_option == _oidEnabled
		SetInfoText("Award player XP for quests, discovery, and other adventure.")
	elseIf a_option == _oidShowMessages
		SetInfoText("Show a short on-screen note when XP is awarded.")
	elseIf a_option == _oidPreset
		SetInfoText("Choose a playstyle pack. Packs write quest types, places, and source sliders.")
	elseIf a_option == _oidGlobal
		SetInfoText("Reserved. Awards use the XP Sources sliders, not this global.")
	elseIf a_option == _oidKilling
		SetInfoText("Allow XP from kills. Combat slider is ignored while this is off.")
	elseIf a_option == _oidReading
		SetInfoText("Allow XP from books. Award is floor(sqrt(book gold) * fReadingMult), then reading weight. Ignores global.")
	else
		int cat = FindOid(_catOids, a_option)
		if cat >= 0
			SetInfoText(_catLabels[cat] + " master scale. 100 is normal.")
			return
		endIf
		int quest = FindOid(_questOids, a_option)
		if quest >= 0
			SetInfoText(_questLabels[quest] + " XP. 0–200.")
			return
		endIf
		int disc = FindOid(_discOids, a_option)
		if disc >= 0
			SetInfoText("XP for discovering a " + ToTitleCase(SplitCamel(_placeKeys[disc])) + ".")
			return
		endIf
		int clear = FindOid(_clearOids, a_option)
		if clear >= 0
			SetInfoText("XP for clearing a " + ToTitleCase(SplitCamel(_placeKeys[clear])) + ".")
			return
		endIf
		int skill = FindOid(_skillOids, a_option)
		if skill >= 0
			SetInfoText(_skillLabels[skill] + " skill-ups. Used when Skill-ups is above 0.")
		endIf
	endIf
endEvent

Function ApplyGeneralSiblingFlags()
	{General page siblings only. Leave the award toggle clickable. Do not touch other pages; SkyUI reuses option ids.}
	int flags = EnabledFlags()
	SetOptionFlags(_oidShowMessages, flags)
	SetOptionFlags(_oidGlobal, flags)
	SetOptionFlags(_oidPreset, flags)
endFunction

int Function EnabledFlags()
	if AdventureXP.GetEnabled()
		return OPTION_FLAG_NONE
	endIf
	return OPTION_FLAG_DISABLED
endFunction

int Function SourceSliderFlags(string key)
	if !AdventureXP.GetEnabled()
		return OPTION_FLAG_DISABLED
	endIf
	if key == "combat" && !AdventureXP.GetAwardKilling()
		return OPTION_FLAG_DISABLED
	endIf
	if key == "reading" && !AdventureXP.GetAwardReading()
		return OPTION_FLAG_DISABLED
	endIf
	return OPTION_FLAG_NONE
endFunction

string Function ActivePresetLabel()
	string active = AdventureXP.GetActivePreset()
	if active == ""
		return "None"
	endIf
	int index = FindPresetIndex(active)
	if index >= 0
		return _presetNames[index]
	endIf
	return DisplayPresetName(active, active)
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
	SetSliderDialogStartValue(current)
	SetSliderDialogDefaultValue(defaultValue)
	SetSliderDialogRange(0.0, 100.0)
	SetSliderDialogInterval(1.0)
endFunction

Function OpenAmountSlider(float current, float defaultValue)
	SetSliderDialogStartValue(current)
	SetSliderDialogDefaultValue(defaultValue)
	SetSliderDialogRange(0.0, 200.0)
	SetSliderDialogInterval(1.0)
endFunction

float Function DefaultCategoryWeight(string key)
	if key == "combat" || key == "reading" || key == "crafting" || key == "skillup"
		return 0.0
	endIf
	return 100.0
endFunction

int Function DefaultQuestXP(string key)
	if key == "Objectives"
		return 12
	elseIf key == "None"
		return 80
	elseIf key == "Main"
		return 120
	elseIf key == "Misc"
		return 50
	elseIf key == "Daedric"
		return 100
	elseIf key == "Dawnguard" || key == "Dragonborn"
		return 90
	endIf
	return 80
endFunction

int Function DefaultDiscoveryXP(string key)
	if key == "City" || key == "DragonLair" || key == "CastleKarstaag" || key == "MiraakTemple"
		return 40
	elseIf key == "Castle" || key == "DwemerRuin" || key == "TelvanniTower"
		return 35
	elseIf key == "Cave" || key == "Default" || key == "DaedricShrine" || key == "Fort" || key == "NordicRuin" || key == "NordicTower" || key == "OrcStronghold" || key == "Town"
		return 30
	elseIf key == "Altar" || key == "Docks" || key == "Doomstone" || key == "GiantCamp" || key == "ImperialTower" || key == "Lighthouse" || key == "Mine" || key == "NordicDwelling" || key == "Shipwreck" || key == "StandingStone" || key == "Settlement"
		return 25
	elseIf key == "Camp" || key == "Clearing" || key == "Grove" || key == "Landmark" || key == "MilitaryCamp" || key == "Pass"
		return 20
	endIf
	return 15
endFunction

int Function DefaultClearXP(string key)
	if key == "DragonLair"
		return 150
	elseIf key == "MiraakTemple"
		return 140
	elseIf key == "CastleKarstaag" || key == "DwemerRuin"
		return 120
	elseIf key == "Fort" || key == "NordicRuin" || key == "TelvanniTower"
		return 110
	elseIf key == "Cave" || key == "Default" || key == "NordicTower" || key == "OrcStronghold"
		return 100
	elseIf key == "GiantCamp" || key == "Mine"
		return 90
	elseIf key == "DaedricShrine" || key == "ImperialTower" || key == "MilitaryCamp" || key == "NordicDwelling" || key == "Shipwreck"
		return 80
	elseIf key == "Grove"
		return 70
	elseIf key == "Camp"
		return 60
	elseIf key == "Pass"
		return 50
	elseIf key == "Clearing"
		return 40
	endIf
	return 0
endFunction

string Function DisplayPresetName(string id, string name)
	if id == "DestructionMage" || id == "destructionmage"
		return "Elementalist"
	endIf
	if id == "Necromancer"
		return "Summoner"
	elseIf id == "Summoner"
		return "Conjurer"
	endIf
	if name != ""
		return ToTitleCase(name)
	endIf
	return ToTitleCase(id)
endFunction

string Function SplitCamel(string text)
	if text == ""
		return text
	endIf
	int len = StringUtil.GetLength(text)
	string out = ""
	int i = 0
	while i < len
		string ch = StringUtil.GetNthChar(text, i)
		int o = StringUtil.AsOrd(ch)
		if i > 0 && o >= 65 && o <= 90
			out += " "
		endIf
		out += ch
		i += 1
	endWhile
	return out
endFunction

string Function ToTitleCase(string text)
	if text == ""
		return text
	endIf
	int len = StringUtil.GetLength(text)
	string out = ""
	bool cap = true
	int i = 0
	while i < len
		string ch = StringUtil.GetNthChar(text, i)
		int o = StringUtil.AsOrd(ch)
		if o == 32 || o == 45
			out += ch
			cap = true
		elseIf o == 95
			out += " "
			cap = true
		elseIf cap
			if o >= 97 && o <= 122
				out += StringUtil.AsChar(o - 32)
			else
				out += ch
			endIf
			cap = false
		else
			if o >= 65 && o <= 90
				out += StringUtil.AsChar(o + 32)
			else
				out += ch
			endIf
		endIf
		i += 1
	endWhile
	return out
endFunction
