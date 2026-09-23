#include "include/events.h"
#include "include/stunHandler.h"

EventResult cellLoadEventHandler::ProcessEvent(const RE::TESCellFullyLoadedEvent*, RE::BSTEventSource<RE::TESCellFullyLoadedEvent>*) {
	//DEBUG("cell load event");
	return EventResult::kContinue;
}
