#pragma once

#include "arcdps_structs_slim.h"
#include "EventSequencer.h"

#include <cstdint>
#include <format>
#include <string>

namespace ArcdpsExtension {
	/**
	 * For every combat event call `Event()`
	 * The virtual protected functions are called for every event.
	 * This will happen in the correct order and in a separate thread.
	 * Call `Shutdown()` in `mod_release` to stop execution of events and stop the thread.
	 */
	class CombatEventHandler {
	public:
		explicit CombatEventHandler()
			: mSequencer([this](cbtevent* ev, ag* src, ag* dst, const char* skillname, uint64_t id, uint64_t revision) -> uintptr_t { EventInternal(ev, src, dst, skillname, id, revision); return 0; }) {
		}
		virtual ~CombatEventHandler() {
			Shutdown();
		}

		void Event(cbtevent* pEvent, ag* pSrc, ag* pDst, const char* pSkillname, uint64_t pId, uint64_t pRevision = 1);

		bool EventsPending();

		/**
		 * Reset everything here aka. calls Reset on the sequencer.
		 * This has no live api uses. Only use in tests!
		 */
		void Reset() {
			mSequencer.Reset();
		}

		void Shutdown() {
			mSequencer.Shutdown();
		}

	protected:
		/**
		 * All events will call this before they are handled.
		 * If you decide to override this function, make sure to also call the parent one, else all other callbacks are never called.
		 * Parameters are 1to1 arcdps' parameters
		 */
		virtual void EventInternal(cbtevent* pEvent, ag* pSrc, ag* pDst, const char* pSkillname, uint64_t pId, uint64_t pRevision);

		/**
		 * Agent is added to the tracking
		 * @param pAccountName AccountName of the adedd agent
		 * @param pCharacterName CharacterName of the added agent
		 * @param pId The ID of the added agent
		 * @param pInstanceId Current Map instance ID, used to detect if players are on the same instance.
		 * @param pProfession The Profession of the added agent
		 * @param pElite The Elite specialization of the added agent
		 * @param pSelf `true` if the agent is the local player.
		 * @param pTeam The team this agent is part of.
		 * @param pSubgroup The subgroup in which this agent is currently (only updated on EnterCombat again)
		 */
		virtual void AgentAdded(const std::string& pAccountName, const std::string& pCharacterName, uintptr_t pId, uintptr_t pInstanceId, Prof pProfession, uint32_t pElite, bool pSelf, uint16_t pTeam, uint8_t pSubgroup) {
			Log("AgentAdded");
		}

		/**
		 * Agent is removed from tracking.
		 * @param pAccountName AccountName of the removed agent
		 * @param pCharacterName CharacterName of the removed agent
		 * @param pId The ID of the removed agent
		 * @param pSelf `true` if the agent is the local player.
		 */
		virtual void AgentRemoved(const std::string& pAccountName, const std::string& pCharacterName, uintptr_t pId, bool pSelf) {
			Log("AgentRemoved");
		}

		/**
		 * Changed target (this is only called for the local user).
		 * @param pId The ID of the agent that is now Targeted
		 */
		virtual void TargetChange(uintptr_t pId) {
			Log("TargetChange");
		}

		/**
		 * The agent with the `pAgentId` is now in combat.
		 * @param pTime Time of the event (Windows timegettime function aka. time since startup)
		 * @param pAgentId The ID of the agent
		 * @param pAgent The actual agent if it is needed with additional information (charactername)
		 */
		virtual void EnterCombat(uint64_t pTime, uintptr_t pAgentId, uint8_t pSubgroup, const ag& pAgent) {
			Log("EnterCombat");
		}

		/**
		 * The agent with the `pAgentId` is now out of combat.
		 * @param pTime Time of the event (Windows timegettime function aka. time since startup)
		 * @param pAgentId The ID of the agent
		 * @param pAgent The actual agent if it is needed with additional information (charactername)
		 */
		virtual void ExitCombat(uint64_t pTime, uintptr_t pAgentId, const ag& pAgent) {
			Log("ExitCombat");
		}

		/**
		 * The agent with the `pAgentId` is now in Alive (this is only called when the agent was dead/downed before).
		 * @param pTime Time of the event (Windows timegettime function aka. time since startup)
		 * @param pAgentId The ID of the agent
		 * @param pAgent The actual agent if it is needed with additional information (charactername)
		 */
		virtual void ChangeUp(uint64_t pTime, uintptr_t pAgentId, const ag& pAgent) {
			Log("ChangeUp");
		}

		/**
		 * The agent with the `pAgentId` is now in Dead.
		 * @param pTime Time of the event (Windows timegettime function aka. time since startup)
		 * @param pAgentId The ID of the agent
		 * @param pAgent The actual agent if it is needed with additional information (charactername)
		 */
		virtual void ChangeDead(uint64_t pTime, uintptr_t pAgentId, const ag& pAgent) {
			Log("ChangeDead");
		}

		/**
		 * The agent with the `pAgentId` is now in Downstate.
		 * @param pTime Time of the event (Windows timegettime function aka. time since startup)
		 * @param pAgentId The ID of the agent
		 * @param pAgent The actual agent if it is needed with additional information (charactername)
		 */
		virtual void ChangeDown(uint64_t pTime, uintptr_t pAgentId, const ag& pAgent) {
			Log("ChangeDown");
		}

		/**
		 * @param pTime Time of the event (Windows timegettime function aka. time since startup)
		 * @param pServerTime Current unix timestamp on the server (UTC)
		 * @param pLocalTime Current unix timestamp of local user time
		 * @param pSpeciesId Species ID of the boss that this Log is for (normally 1).
		 */
		virtual void LogStart(uint64_t pTime, uint32_t pServerTime, uint32_t pLocalTime, uintptr_t pSpeciesId) {
			Log("LogStart");
		}

		/**
		 * @param pTime Time of the event (Windows timegettime function aka. time since startup)
		 * @param pServerTime Current unix timestamp on the server (UTC)
		 * @param pLocalTime Current unix timestamp of local user time
		 * @param pSpeciesId Species ID of the boss that this Log is for.
		 */
		virtual void LogEnd(uint64_t pTime, uint32_t pServerTime, uint32_t pLocalTime, uintptr_t pSpeciesId) {
			Log("LogEnd");
		}

		/**
		 * @param pTime Time of the event (Windows timegettime function aka. time since startup)
		 * @param pServerTime Current unix timestamp on the server (UTC)
		 * @param pLocalTime Current unix timestamp of local user time
		 * @param pSpeciesId Species ID of the boss that this Log is for.
		 */
		virtual void LogNpcUpdate(uint64_t pTime, uint32_t pServerTime, uint32_t pLocalTime, uintptr_t pSpeciesId) {
			Log("LogNpcUpdate");
		}

		/**
		 * @param pTime Time of the event (Windows timegettime function aka. time since startup)
		 * @param pAgentId The Id of the agent
		 * @param pWeaponSet The weaponSet which was swapped to
		 * @param pAgent The actual agent if it is needed with additional information (charactername)
		 */
		virtual void WeaponSwap(uint64_t pTime, uintptr_t pAgentId, WeaponSet pWeaponSet, const ag& pAgent) {
			Log("WeaponSwap");
		}

		/**
		 * Reward is only called for the local user (these are the wiggly boxes you get).
		 * @param pTime Time of the event (Windows timegettime function aka. time since startup)
		 * @param pSelfId The Id of the local user
		 * @param pRewardId The Id of the reward you get (not always unique, but mostly)
		 * @param pRewardType The Type of the reward
		 */
		virtual void Reward(uint64_t pTime, uintptr_t pSelfId, uintptr_t pRewardId, int32_t pRewardType) {
			Log("Reward");
		}

		/**
		 * @param pTime Time of the event (Windows timegettime function aka. time since startup)
		 * @param pAgentId The Id of the agent
		 * @param pNewTeam The new team the agent is now part if
		 * @param pAgent The actual agent if it is needed with additional information (charactername)
		 */
		virtual void TeamChange(uint64_t pTime, uintptr_t pAgentId, uintptr_t pNewTeam, const ag& pAgent) {
			Log("TeamChange");
		}

		/**
		 * @param pTime Time of the event (Windows timegettime function aka. time since startup)
		 * @param pAgentId The Id of the agent with the buff
		 * @param pStackId The StackId that is marked as active
		 * @param pAgent The actual agent if it is needed with additional information (charactername)
		 */
		virtual void StackActive(uint64_t pTime, uintptr_t pAgentId, uintptr_t pStackId, const ag& pAgent) {
			Log("StackActive");
		}

		/**
		 * Reset the stack to given duration, also disables the stack
		 * @param pTime Time of the event (Windows timegettime function aka. time since startup)
		 * @param pAgentId The Id of the agent with the buff
		 * @param pDuration The Duration to reset to
		 * @param pStackId The StackId that is changed and marked inactive
		 * @param pAgent The actual agent if it is needed with additional information (charactername)
		 */
		virtual void StackReset(uint64_t pTime, uintptr_t pAgentId, uintptr_t pDuration, uint32_t pStackId, const ag& pAgent) {
			Log(std::format("StackReset|agentName {}|duration {}|stackId {}", pAgent.name, pDuration, pStackId));
		}

		/**
		 * Reset all stats (this is there when the arcdps api resets all it's stats. e.g. after the Xera pre)
		 * @param pTime Time of the event (Windows timegettime function aka. time since startup)
		 */
		virtual void StatReset(uint64_t pTime) {
			Log("StatReset");
		}

		/**
		 * Extension is used when extensions send events over arcdps (the only addon i know that does this is the healing stats addon).
		 * If you want to parse anything of that life, you have to pare it yourself.
		 */
		virtual void Extension(uint64_t pTime, cbtevent* pEvent, ag* pSrc, ag* pDst, const char* pSkillname, uint64_t pId) {
			Log("Extension");
		}

		/**
		 * This is called by arcdps for events that are delayed as anti-cheat-measure.
		 * Mostly known for hidden buffs that you get when you get a mechanic.
		 * This has to be parsed by yourself, cause i don't know how this works exactly.
		 * If you want to use this you can also parse the events previously and send them over the correct channels.
		 */
		virtual void Delayed(uint64_t pTime, cbtevent* pEvent, ag* pSrc, ag* pDst, const char* pSkillname, uint64_t pId) {
			Log("Delayed");
		}

		/**
		 * Roughly the log-relative ms that the server started the instance.
		 * @param pTime Time of the event (Windows timegettime function aka time since startup)
		 * @param pStartTime roughly the log-relative ms that the server started the instance
		 */
		virtual void InstanceStart(uint64_t pTime, uintptr_t pStartTime) {
			Log("InstanceStart");
		}

		/**
		 * Called every 500ms
		 * @param pTime Time of the event (Windows timegettime function aka time since startup)
		 * @param pData = 25 - tickrate (when tickrate < 21)
		 */
		virtual void Tickrate(uint64_t pTime, uintptr_t pData) {
			Log("Tickrate");
		}

		/**
		 * @param pTime Time of the event (Windows timegettime function aka time since startup)
		 * @param pEnemyAgent enemy agent that went down
		 * @param pSinceTime time in ms since last 90% (for downs contribution)
		 */
		virtual void Last90BeforeDown(uint64_t pTime, uintptr_t pEnemyAgent, uintptr_t pSinceTime) {
			Log("Last90BeforeDown");
		}

		/**
		 * Activation Event, not further deduced.
		 * For more information see the arcdps documentation.
		 * If you need deduction, change these files and create a PR.
		 */
		virtual void Activation(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId) {
			Log("Activation");
		}

		/**
		 * BuffRemove event, not further deduced.
		 * This is called every time a stack is removed.
		 * For more information see the arcdps documentation.
		 * If you need deduction, change these files and create a PR.
		 */
		virtual void BuffRemove(uint64_t pTime, const cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId, uint32_t pStackId) {
			Log("BuffRemove");
		}

		/**
		 * BuffDamage event, not further deduced.
		 * This is called every time the buff simulation shows damage (mostly for condis, which are also buffs).
		 * For more information see the arcdps documentation.
		 * If you need deduction, change these files and create a PR.
		 */
		virtual void BuffDamage(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId) {
			Log("BuffDamage");
		}

		/**
		 * BuffApply event, not further deduced.
		 * This is called every time a stack of a buff is added.
		 * For more information see the arcdps documentation.
		 * If you need deduction, change these files and create a PR.
		 */
		virtual void BuffApply(uint64_t pTime, const cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId, uint32_t pStackId) {
			Log("BuffApply");
		}

		/**
		 * Strike event, not further deduced.
		 * This is called for every hit on something.
		 * For more information see the arcdps documentation.
		 * If you need deduction, change these files and create a PR.
		 */
		virtual void Strike(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId) {
			Log("Strike");
		}

		virtual void BuffInitial(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId, uint32_t pStackId) {
			Log("BuffInitial");
		}

		virtual void Log(const std::string& pText) {}

		/**
		 * The time of the currently executed Event. Reset every executed event.
		 */
		uint64_t mLastEventTime = 0;

	private:
		EventSequencer mSequencer;

		void BuffEvent(cbtevent* pEvent, ag* pSrc, ag* pDst, const char* pSkillname, uint64_t pId);
	};
} // namespace ArcdpsExtension
