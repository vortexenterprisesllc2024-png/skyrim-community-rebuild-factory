#pragma once

// Event sinks and classifiers. Windows build links CommonLibSSE-NG.

namespace AdventureXP::Events {

void Register();
void Unregister();
// Snapshot current objective states after a save/new game so already-visible
// journal objectives do not look like advances on the next silent stage.
void RememberQuestObjectiveBaseline();

}  // namespace AdventureXP::Events
