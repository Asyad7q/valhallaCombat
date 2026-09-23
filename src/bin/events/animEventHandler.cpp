#include "include/events.h"

/*
 * Valhalla Combat - No Stamina build
 *
 * The original animation-event handling in this file was used only for
 * Valhalla's stamina/attack-stamina system. It is intentionally disabled.
 */

void animEventHandler::ProcessEvent(
    RE::BSTEventSink<RE::BSAnimationGraphEvent>*,
    RE::BSAnimationGraphEvent*,
    RE::BSTEventSource<RE::BSAnimationGraphEvent>*)
{
}

EventResult animEventHandler::ProcessEvent_NPC(
    RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink,
    RE::BSAnimationGraphEvent* a_event,
    RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
{
    return _ProcessEvent_NPC(a_sink, a_event, a_eventSource);
}

EventResult animEventHandler::ProcessEvent_PC(
    RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink,
    RE::BSAnimationGraphEvent* a_event,
    RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
{
    return _ProcessEvent_PC(a_sink, a_event, a_eventSource);
}
