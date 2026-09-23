#include "include/Hooks.h"
#include "include/Utils.h"
#include "include/blockHandler.h"
#include "ValhallaCombat.hpp"
#include "include/stunHandler.h"
#include "include/hitProcessor.h"
#include "include/AI.h"
#include "include/settings.h"
namespace Hooks
{
	void Hook_OnRestoreActorValue::RestoreActorValue(
		RE::Actor* a_actor,
		RE::ActorValue a_actorValue,
		float a_val)
	{
		if (a_actorValue == RE::ActorValue::kHealth) {
			stunHandler::GetSingleton()->modStun(a_actor, a_val);
		}

		_RestoreActorValue(a_actor, a_actorValue, a_val);
	}

//#pragma region getStaggerMagnitude_Weapon
//	float Hook_OnGetStaggerMagnitude::getStaggerMagnitude_Weapon(RE::ActorValueOwner* a1, RE::ActorValueOwner* a2, RE::TESObjectWEAP* a3, float a4)
//	{
//		if (settings::bBalanceToggle) {
//			return 0;
//		}
//		return _getStaggerMagnitude_Weapon(a1, a2, a3, a4);
//	}
//
//	float Hook_OnGetStaggerMagnitude::getStaggerManitude_Bash(uintptr_t a1, uintptr_t a2)
//	{
//		if (settings::bBalanceToggle) {
//			return 0;
//		}
//		return _getStaggerManitude_Bash(a1, a2);
//
//	}

#pragma endregion
#pragma region MeleeHit
	void Hook_OnMeleeHit::processHit(RE::Actor* victim, RE::HitData& hitData)
	{
		using HITFLAG = RE::HitData::Flag;
		auto aggressor = hitData.aggressor.get().get();
		if (!victim || !aggressor || victim->IsDead()) {
			_ProcessHit(victim, hitData);
			return;
		}
		hitProcessor::GetSingleton()->processHit(aggressor, victim, hitData);
		_ProcessHit(victim, hitData);
	}

#pragma endregion

//#pragma region MainUpdate
//	void Hook_MainUpdate::Update(RE::Main* a_this, float a2)
//	{
//		ValhallaCombat::GetSingleton()->update();
//		_Update(a_this, a2);
//	}
//#pragma endregion

#pragma region projectileHit
	/*Decide whether the collision is a actor-projectile collision. If it is, initialize a deflection attempt by the actor.
@param a_projectile: the projectile to be deflected.
@param a_AllCdPointCollector: pointer to the container storing all collision points.
@return whether a successful deflection is performed by the actor.*/
	inline bool shouldIgnoreHit(RE::Projectile* a_projectile, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
	{
		if (a_AllCdPointCollector) {
			for (auto& hit : a_AllCdPointCollector->hits) {
				auto refrA = RE::TESHavokUtilities::FindCollidableRef(*hit.rootCollidableA);
				auto refrB = RE::TESHavokUtilities::FindCollidableRef(*hit.rootCollidableB);
				if (refrA && refrA->formType == RE::FormType::ActorCharacter) {
					return blockHandler::GetSingleton()->processProjectileBlock(refrA->As<RE::Actor>(), a_projectile, const_cast<RE::hkpCollidable*>(hit.rootCollidableB));
				}
				if (refrB && refrB->formType == RE::FormType::ActorCharacter) {
					return blockHandler::GetSingleton()->processProjectileBlock(refrB->As<RE::Actor>(), a_projectile, const_cast<RE::hkpCollidable*>(hit.rootCollidableA));
				}
			}
		}
		return false;
	}
	void Hook_OnProjectileCollision::OnArrowCollision(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
	{
		//DEBUG("hooked arrow collission vfunc");
		if (shouldIgnoreHit(a_this, a_AllCdPointCollector)) {
			return;
		};
		_arrowCollission(a_this, a_AllCdPointCollector);
	};

	void Hook_OnProjectileCollision::OnMissileCollision(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
	{
		//DEBUG("hooked missile collission vfunc");
		if (shouldIgnoreHit(a_this, a_AllCdPointCollector)) {
			return;
		};
		_missileCollission(a_this, a_AllCdPointCollector);
	}
#pragma endregion

	void Hook_OnMeleeCollision::processHit(RE::Actor* a_aggressor, RE::Actor* a_victim, std::int64_t a_int1, bool a_bool, void* a_unkptr)
	{
		if (settings::bTimedBlockToggle && blockHandler::GetSingleton()->processMeleeTimedBlock(a_victim, a_aggressor)) {
			return;
		}
		if (settings::bTackleToggle && blockHandler::GetSingleton()->processMeleeTackle(a_victim, a_aggressor)) {
			return;
		}
		_ProcessHit(a_aggressor, a_victim, a_int1, a_bool, a_unkptr);
	}

	static void unblock_delayed_taskfunc(RE::AttackBlockHandler* a_this, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data) 
	{
		auto player = RE::PlayerCharacter::GetSingleton();
		if (player &&!blockHandler::GetSingleton()->isBlockKeyHeld() && (player->IsBlocking() || player->AsActorState()->GetAttackState() == RE::ATTACK_STATE_ENUM::kBash) || player->AsActorState()->actorState2.staggered) {
			if (a_event) {
				a_this->ProcessButton(a_event, a_data);
			}
		}

		if (a_event) {
			delete (a_event);
		}
	}
	
	static void unblock_delayed_threadfunc(RE::AttackBlockHandler* a_this, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data, float a_time) 
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(a_time * 1000))); // wait for, by default, 0.3 sec
		auto task = SKSE::GetTaskInterface();
		if (task) {
			task->AddTask([a_this, a_event, a_data]() {
				unblock_delayed_taskfunc(a_this, a_event, a_data);
			});
		}
	}
	void Hook_AttackBlockHandler_OnProcessButton::ProcessButton(RE::AttackBlockHandler* a_this, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data)
	{
		if (a_event->QUserEvent() == "Left Attack/Block") {
			blockHandler* blockHandler = blockHandler::GetSingleton();
			if (a_event->IsDown()) {
				if (settings::bTimedBlockToggle || settings::bTimedBlockProjectileToggle) {
					blockHandler->onBlockKeyDown();
				}
				if (settings::bTackleToggle) {
					blockHandler->onTackleKeyDown();
				}
			} else if (a_event->IsUp()) {
				if (settings::bTimedBlockToggle || settings::bTimedBlockProjectileToggle) {
					blockHandler->onBlockKeyUp();
				}
				if (settings::bBlockCommitmentToggle) {/* Block commitment; Immediately releasing the block key after pressing it will not cause the actor to immediately unblock.*/
					auto pc = RE::PlayerCharacter::GetSingleton();
					if (pc && pc->IsBlocking() && a_event->HeldDuration() < settings::fBlockCommitmentTime) { //do not process this request until later
						RE::ButtonEvent* releaseEvent = RE::ButtonEvent::Create(a_event->GetDevice(), "forceRelease", a_event->GetIDCode(), a_event->Value(), a_event->HeldDuration());  // event to unblock, will be fired later.
						if (releaseEvent) {
							float delay_time = settings::fBlockCommitmentTime - a_event->HeldDuration();
							std::jthread t(unblock_delayed_threadfunc, a_this, releaseEvent, a_data, delay_time);
							t.detach();
							return; //discard the event
						}
					}
				}
			}
		}
		_ProcessButton(a_this, a_event, a_data);
	}
}
