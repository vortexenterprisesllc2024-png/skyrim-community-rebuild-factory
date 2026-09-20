#pragma once

// Engine-free quest-stage award policy. The Windows plugin fills
// QuestStageAwardFacts from TESQuest / BGSQuestObjective; host tests
// drive the same DecideQuestStageAward() with fixtures.

#include <cstdint>

namespace AdventureXP {

struct QuestStageAwardFacts {
	std::uint16_t stage = 0;
	bool alreadyAwarded = false;
	bool startupOrShutdown = false;
	bool journalMatchesStage = false;
	bool journalTextVisible = false;
	bool visibleObjectiveAdvanced = false;
};

enum class QuestStageAwardDecision {
	Skip,
	Award,
};

// QUEST_OBJECTIVE_STATE values from CommonLib QuestObjectiveStates.h.
// Only the *Displayed variants are things the player can see in the journal.
inline constexpr int kObjectiveDormant = 0;
inline constexpr int kObjectiveDisplayed = 1;
inline constexpr int kObjectiveCompleted = 2;
inline constexpr int kObjectiveCompletedDisplayed = 3;
inline constexpr int kObjectiveFailed = 4;
inline constexpr int kObjectiveFailedDisplayed = 5;

inline bool IsJournalVisibleObjectiveState(int state)
{
	return state == kObjectiveDisplayed ||
		state == kObjectiveCompletedDisplayed ||
		state == kObjectiveFailedDisplayed;
}

inline bool HasVisibleText(const char* text)
{
	if (!text) {
		return false;
	}
	for (const char* p = text; *p; ++p) {
		if (*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') {
			return true;
		}
	}
	return false;
}

// An objective counts as a player-visible advance only when its NNAM text is
// non-empty and the state *changed* into a displayed variant. "Any objective
// is already displayed" is the wrong test: Way of the Voice keeps "Speak to
// the Greybeards" up the whole 7000 Steps climb while silent trigger stages
// fire underneath.
inline bool ObjectiveAdvancedVisibly(int previous, int current, bool hasDisplayText)
{
	if (!hasDisplayText) {
		return false;
	}
	if (previous == current) {
		return false;
	}
	return IsJournalVisibleObjectiveState(current);
}

inline QuestStageAwardDecision DecideQuestStageAward(
	bool awardStages,
	bool awardSilent,
	const QuestStageAwardFacts& facts)
{
	if (!awardStages || facts.alreadyAwarded) {
		return QuestStageAwardDecision::Skip;
	}
	// Stage 0 is quest-start / empty. 4.2.10 skipped it; keep that even when
	// bAwardSilentQuestStages=1.
	if (facts.stage == 0) {
		return QuestStageAwardDecision::Skip;
	}
	if (awardSilent) {
		return QuestStageAwardDecision::Award;
	}
	if (facts.startupOrShutdown) {
		return QuestStageAwardDecision::Skip;
	}
	if (facts.journalMatchesStage && facts.journalTextVisible) {
		return QuestStageAwardDecision::Award;
	}
	if (facts.visibleObjectiveAdvanced) {
		return QuestStageAwardDecision::Award;
	}
	return QuestStageAwardDecision::Skip;
}

}  // namespace AdventureXP
