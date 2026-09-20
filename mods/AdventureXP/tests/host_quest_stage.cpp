#include "AdventureXP/QuestStage.h"

#include <cassert>
#include <iostream>

int main()
{
	using namespace AdventureXP;

	assert(IsJournalVisibleObjectiveState(kObjectiveDisplayed));
	assert(IsJournalVisibleObjectiveState(kObjectiveCompletedDisplayed));
	assert(IsJournalVisibleObjectiveState(kObjectiveFailedDisplayed));
	assert(!IsJournalVisibleObjectiveState(kObjectiveDormant));
	assert(!IsJournalVisibleObjectiveState(kObjectiveCompleted));
	assert(!IsJournalVisibleObjectiveState(kObjectiveFailed));

	assert(!HasVisibleText(nullptr));
	assert(!HasVisibleText(""));
	assert(!HasVisibleText(" \t\n"));
	assert(HasVisibleText("Speak to the Greybeards"));

	// Already-displayed objective + new silent stage is NOT an advance.
	assert(!ObjectiveAdvancedVisibly(kObjectiveDisplayed, kObjectiveDisplayed, true));
	// Completing a visible objective is an advance.
	assert(ObjectiveAdvancedVisibly(kObjectiveDisplayed, kObjectiveCompletedDisplayed, true));
	// Newly displayed with text is an advance.
	assert(ObjectiveAdvancedVisibly(kObjectiveDormant, kObjectiveDisplayed, true));
	// Script-only complete (not displayed) and empty NNAM are not visible.
	assert(!ObjectiveAdvancedVisibly(kObjectiveDormant, kObjectiveCompleted, true));
	assert(!ObjectiveAdvancedVisibly(kObjectiveDormant, kObjectiveDisplayed, false));

	QuestStageAwardFacts climb;
	climb.stage = 15;
	climb.journalMatchesStage = false;
	climb.journalTextVisible = false;
	climb.visibleObjectiveAdvanced = false;
	assert(
		DecideQuestStageAward(true, false, climb) == QuestStageAwardDecision::Skip);

	QuestStageAwardFacts journal;
	journal.stage = 10;
	journal.journalMatchesStage = true;
	journal.journalTextVisible = true;
	assert(
		DecideQuestStageAward(true, false, journal) == QuestStageAwardDecision::Award);

	QuestStageAwardFacts emptyJournal;
	emptyJournal.stage = 10;
	emptyJournal.journalMatchesStage = true;
	emptyJournal.journalTextVisible = false;
	assert(
		DecideQuestStageAward(true, false, emptyJournal) == QuestStageAwardDecision::Skip);

	QuestStageAwardFacts objective;
	objective.stage = 20;
	objective.visibleObjectiveAdvanced = true;
	assert(
		DecideQuestStageAward(true, false, objective) == QuestStageAwardDecision::Award);

	QuestStageAwardFacts startup;
	startup.stage = 10;
	startup.startupOrShutdown = true;
	startup.journalMatchesStage = true;
	startup.journalTextVisible = true;
	assert(
		DecideQuestStageAward(true, false, startup) == QuestStageAwardDecision::Skip);

	QuestStageAwardFacts zero;
	zero.stage = 0;
	zero.journalMatchesStage = true;
	zero.journalTextVisible = true;
	assert(DecideQuestStageAward(true, false, zero) == QuestStageAwardDecision::Skip);
	assert(DecideQuestStageAward(true, true, zero) == QuestStageAwardDecision::Skip);

	QuestStageAwardFacts silent;
	silent.stage = 15;
	assert(DecideQuestStageAward(true, true, silent) == QuestStageAwardDecision::Award);

	QuestStageAwardFacts dup;
	dup.stage = 10;
	dup.alreadyAwarded = true;
	dup.journalMatchesStage = true;
	dup.journalTextVisible = true;
	assert(DecideQuestStageAward(true, false, dup) == QuestStageAwardDecision::Skip);
	assert(DecideQuestStageAward(true, true, dup) == QuestStageAwardDecision::Skip);

	QuestStageAwardFacts off;
	off.stage = 10;
	off.journalMatchesStage = true;
	off.journalTextVisible = true;
	assert(DecideQuestStageAward(false, false, off) == QuestStageAwardDecision::Skip);

	std::cout << "host_quest_stage: ok\n";
	return 0;
}
