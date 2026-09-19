Scriptname AdventureXP Hidden
{Clean-room Papyrus API for AdventureXP 4.2. Newly written.

Do not call third-party XP plugin natives. MCM uses only these.}

String Function GetVersion() Global Native

Int Function GetPresetCount() Global Native

String Function GetPresetId(Int index) Global Native

String Function GetPresetName(Int index) Global Native

String Function GetActivePreset() Global Native

Bool Function ApplyPreset(String id) Global Native

Float Function GetCategoryWeight(String category) Global Native

Function SetCategoryWeight(String category, Float weight) Global Native

Float Function GetGlobalXPPercent() Global Native

Function SetGlobalXPPercent(Float percent) Global Native

Bool Function GetEnabled() Global Native

Function SetEnabled(Bool enabled) Global Native

Bool Function GetShowMessages() Global Native

Function SetShowMessages(Bool show) Global Native

Bool Function GetAwardKilling() Global Native

Function SetAwardKilling(Bool enabled) Global Native

Bool Function GetAwardReading() Global Native

Function SetAwardReading(Bool enabled) Global Native

Int Function GetQuestXP(String name) Global Native

Function SetQuestXP(String name, Int amount) Global Native

Int Function GetDiscoveryXP(String name) Global Native

Function SetDiscoveryXP(String name, Int amount) Global Native

Int Function GetClearXP(String name) Global Native

Function SetClearXP(String name, Int amount) Global Native

; Legacy aliases. Prefer Get/SetQuestXP and Get/SetDiscoveryXP / Get/SetClearXP.
Float Function GetQuestTypeWeight(String name) Global Native

Function SetQuestTypeWeight(String name, Float weight) Global Native

Float Function GetPlaceTypeWeight(String name) Global Native

Function SetPlaceTypeWeight(String name, Float weight) Global Native

Float Function GetSkillWeight(String skillName) Global Native

Function SetSkillWeight(String skillName, Float weight) Global Native
