Scriptname AdventureXP Hidden
{Clean-room Papyrus API for AdventureXP 4.0. Newly written.

Do not call Experience / zax natives. MCM uses only these.}

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

Float Function GetQuestTypeWeight(String name) Global Native

Function SetQuestTypeWeight(String name, Float weight) Global Native

Float Function GetPlaceTypeWeight(String name) Global Native

Function SetPlaceTypeWeight(String name, Float weight) Global Native
