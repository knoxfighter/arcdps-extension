#pragma once

#include "arcdps_structs_slim.h"

#include <compare>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <set>
#include <string>
#include <thread>

namespace ArcdpsExtension {
	class EventSequencer {
	public:
		typedef std::function<uintptr_t(cbtevent* ev, ag* src, ag* dst, const char* skillname, uint64_t id, uint64_t revision)> CallbackSignature;

		explicit EventSequencer(CallbackSignature pCallback);
		virtual ~EventSequencer();

		// delete copy and move
		EventSequencer(const EventSequencer& pOther) = delete;
		EventSequencer(EventSequencer&& pOther) noexcept = delete;
		EventSequencer& operator=(const EventSequencer& pOther) = delete;
		EventSequencer& operator=(EventSequencer&& pOther) noexcept = delete;

		struct Event {
			struct CbtEvent : cbtevent {
				bool Present = false;
			};

			struct Agent : ag {
				std::string NameStorage;
				bool Present = false;
			};

			CbtEvent Ev;
			Agent Source;
			Agent Destination;

			const char* Skillname; // Skill names are guaranteed to be valid for the lifetime of the process so copying pointer is fine
			uint64_t Id;
			uint64_t Revision;

			std::strong_ordering operator<=>(const Event& pOther) const {
				return Id <=> pOther.Id;
			}

			Event(cbtevent* pEv, ag* pSrc, ag* pDst, const char* pSkillname, uint64_t pId, uint64_t pRevision)
				: Skillname(pSkillname),
				  Id(pId),
				  Revision(pRevision) {
				if (pEv) {
					*static_cast<cbtevent*>(&Ev) = *pEv;
					Ev.Present = true;
				}
				if (pSrc) {
					*static_cast<ag*>(&Source) = *pSrc;
					Source.Present = true;
					if (Source.name) {
						Source.NameStorage = pSrc->name;
					}
				}
				if (pDst) {
					*static_cast<ag*>(&Destination) = *pDst;
					Destination.Present = true;
					if (Destination.name) {
						Destination.NameStorage = pDst->name;
					}
				}
			}
		};

		void ProcessEvent(cbtevent* pEv, ag* pSrc, ag* pDst, const char* pSkillname, uint64_t pId, uint64_t pRevision);

		[[nodiscard]] bool EventsPending() const;

		/**
		 * Deletes all pending Events and resets all counters.
		 * This has no live api uses. Only use in tests!
		 */
		void Reset();

		void Shutdown();

	private:
		const CallbackSignature mCallback;
		std::multiset<Event> mElements;
		std::mutex mElementsMutex;
		std::condition_variable_any mNewElement;
		std::jthread mThread;
		uint64_t mNextId = 2; // Events start with ID 2 for some reason (it is always like that and no plans to change)
		bool mThreadRunning = false;

		void EventInternal(Event& pElem) const;
	};
} // namespace ArcdpsExtension
