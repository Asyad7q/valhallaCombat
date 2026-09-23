#include "include/AI.h"
#include "include/data.h"
#include "include/stunHandler.h"
void AI::action_PerformEldenCounter(RE::Actor* a_actor) {
	a_actor->NotifyAnimationGraph(data::AnimEvent_GuardCounter);
}

bool AI::getShouldTimedBlock(RE::Actor* actor) {
	return !stunHandler::GetSingleton()->getIsStunBroken(actor);
}
