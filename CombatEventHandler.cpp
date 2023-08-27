#include "CombatEventHandler.h"

#include <string>

void ArcdpsExtension::CombatEventHandler::Event(cbtevent* pEvent, ag* pSrc, ag* pDst, const char* pSkillname, uint64_t pId, uint64_t pRevision) {
	mSequencer.ProcessEvent(pEvent, pSrc, pDst, pSkillname, pId, pRevision);
}

bool ArcdpsExtension::CombatEventHandler::EventsPending() {
	return mSequencer.EventsPending();
}

void ArcdpsExtension::CombatEventHandler::EventInternal(cbtevent* pEvent, ag* pSrc, ag* pDst, const char* pSkillname, uint64_t pId) {
	Log(std::format("pId: {}", pId));
	if (pEvent) {
		mLastEventTime = pEvent->time;

		if (pEvent->is_statechange) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
			switch (pEvent->is_statechange) {
				case CBTS_ENTERCOMBAT:
					EnterCombat(mLastEventTime, pEvent->src_agent, static_cast<uint8_t>(pEvent->dst_agent), *pSrc);
					break;
				case CBTS_EXITCOMBAT:
					ExitCombat(mLastEventTime, pEvent->src_agent, *pSrc);
					break;
				case CBTS_CHANGEUP:
					ChangeUp(mLastEventTime, pEvent->src_agent, *pSrc);
					break;
				case CBTS_CHANGEDEAD:
					ChangeDead(mLastEventTime, pEvent->src_agent, *pSrc);
					break;
				case CBTS_CHANGEDOWN:
					ChangeDown(mLastEventTime, pEvent->src_agent, *pSrc);
					break;
					//                case CBTS_SPAWN: // Not in realtime api
					//                case CBTS_DESPAWN: // Not in realtime api
					//                case CBTS_HEALTHUPDATE: // Not in realtime api
				case CBTS_LOGSTART:
					LogStart(mLastEventTime, pEvent->value, pEvent->buff_dmg, pEvent->src_agent);
					break;
				case CBTS_LOGEND:
					LogEnd(mLastEventTime, pEvent->value, pEvent->buff_dmg, pEvent->src_agent);
					break;
				case CBTS_WEAPSWAP:
					WeaponSwap(mLastEventTime, pEvent->src_agent, static_cast<WeaponSet>(pEvent->dst_agent), *pSrc);
					break;
					//                case CBTS_MAXHEALTHUPDATE: // Not in realtime api
					//                case CBTS_POINTOFVIEW: // Not in realtime api
					//                case CBTS_LANGUAGE: // Not in realtime api // Use unofficial extras to get updates of the language
					//                case CBTS_GWBUILD: // Not in realtime api
					//                case CBTS_SHARDID: // Not in realtime api
				case CBTS_REWARD:
					Reward(mLastEventTime, pEvent->src_agent, pEvent->dst_agent, pEvent->value);
					break;
				case CBTS_BUFFINITIAL: { // (statechange==18, buff==18, normal cbtevent otherwise)
					if (pEvent->buff == 18) {
						// gives all current boons on LogStart
						auto pad = reinterpret_cast<uint32_t*>(&pEvent->pad61);
						BuffInitial(mLastEventTime, pEvent, *pSrc, *pDst, pSkillname, pId, *pad);
					} else {
						BuffEvent(pEvent, pSrc, pDst, pSkillname, pId);
					}
					break;
				}
					//                case CBTS_POSITION: // Not in realtime api
					//                case CBTS_VELOCITY: // Not in realtime api
					//                case CBTS_FACING: // Not in realtime api
				case CBTS_TEAMCHANGE:
					TeamChange(mLastEventTime, pEvent->src_agent, pEvent->dst_agent, *pSrc);
					break;
					//                case CBTS_ATTACKTARGET: // Not in realtime api
					//                case CBTS_TARGETABLE: // Not in realtime api
					//                case CBTS_MAPID: // Not in realtime api
					//                case CBTS_REPLINFO: // Internal only, not used
				case CBTS_STACKACTIVE:
					StackActive(mLastEventTime, pEvent->src_agent, pEvent->dst_agent, *pSrc);
					break;
				case CBTS_STACKRESET: {
					auto pad = reinterpret_cast<uint32_t*>(&pEvent->pad61);
					StackReset(mLastEventTime, pEvent->src_agent, pEvent->value, *pad, *pSrc);
					break;
				}
					//                case CBTS_GUILD: // not relevant in live api
					//                case CBTS_BUFFINFO: // Not in realtime api
					//                case CBTS_BUFFFORMULA: // Not in realtime api
					//                case CBTS_SKILLINFO: // Not in realtime api
					//                case CBTS_SKILLTIMING: // Not in realtime api
					//                case CBTS_BREAKBARSTATE: // Not in realtime api
					//                case CBTS_BREAKBARPERCENT: // Not in realtime api
					//                case CBTS_ERROR: // Not in realtime api
					//                case CBTS_TAG: // Not useful information, it is either delayed or not there at all.
					//                case CBTS_BARRIERUPDATE: // Not in realtime api
				case CBTS_STATRESET:
					StatReset(mLastEventTime);
					break;
				case CBTS_EXTENSION:
					Extension(mLastEventTime, pEvent, pSrc, pDst, pSkillname, pId);
					break;
				case CBTS_APIDELAYED:
					Delayed(mLastEventTime, pEvent, pSrc, pDst, pSkillname, pId);
					break;
				case CBTS_INSTANCESTART:
					InstanceStart(mLastEventTime, pEvent->src_agent);
					break;
				case CBTS_TICKRATE:
					Tickrate(mLastEventTime, pEvent->src_agent);
					break;
				case CBTS_LAST90BEFOREDOWN:
					Last90BeforeDown(mLastEventTime, pEvent->src_agent, pEvent->dst_agent);
					break;
					//                case CBTS_EFFECT: // Not in realtime api
					//                case CBTS_IDTOGUID: // Not in realtime api
				case CBTS_LOGNPCUPDATE:
					LogNpcUpdate(mLastEventTime, static_cast<uint32_t>(pEvent->value), static_cast<uint32_t>(pEvent->buff_dmg), pEvent->src_agent);
			}
#pragma clang diagnostic pop
		} else if (pEvent->is_activation) {
			Activation(mLastEventTime, pEvent, *pSrc, *pDst, pSkillname, pId);
		} else if (pEvent->is_buffremove) {
			auto pad = reinterpret_cast<uint32_t*>(&pEvent->pad61);
			BuffRemove(mLastEventTime, pEvent, *pSrc, *pDst, pSkillname, pId, *pad);
		} else if (pEvent->buff) {
			BuffEvent(pEvent, pSrc, pDst, pSkillname, pId);
		} else {
			// Strike damage
			Strike(mLastEventTime, pEvent, *pSrc, *pDst, pSkillname, pId);
		}
	}
	/* pEvent is null. pDst will only be valid on tracking add. pSkillname will also be null */
	else {
		/* notify tracking change */
		if (!pSrc->elite) {
			// only run, when names are set and not null
			if (pSrc->name != nullptr && pSrc->name[0] != '\0' && pDst->name != nullptr && pDst->name[0] != '\0') {

				std::string accountname(pDst->name);

				// remove ':' at the beginning of the name.
				if (accountname.at(0) == ':') {
					accountname.erase(0, 1);
				}

				/* add */
				if (pSrc->prof) {
					AgentAdded(accountname, pSrc->name, pSrc->id, pDst->id, pDst->prof, pDst->elite, pDst->self, pSrc->team, static_cast<uint8_t>(pDst->team));
				}
				/* remove */
				else {
					AgentRemoved(accountname, pSrc->name, pSrc->id, pDst->self);
				}
			}
		}
		/* target change */
		else if (pSrc->elite == 1) {
			TargetChange(pSrc->id);
		}
	}
}

void ArcdpsExtension::CombatEventHandler::BuffEvent(cbtevent* pEvent, ag* pSrc, ag* pDst, const char* pSkillname, uint64_t pId) {
	if (pEvent->buff_dmg) {
		BuffDamage(mLastEventTime, pEvent, *pSrc, *pDst, pSkillname, pId);
	} else {
		// Buff apply event
		auto pad = reinterpret_cast<uint32_t*>(&pEvent->pad61);
		BuffApply(mLastEventTime, pEvent, *pSrc, *pDst, pSkillname, pId, *pad);
	}
}
