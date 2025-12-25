#pragma once

#include "arcdps_structs_slim.h"

#include <cstdlib>
#include <functional>
#include <mutex>
#include <set>
#include <thread>

namespace ArcdpsExtension {
	class EventSequencer {
	public:
		typedef std::function<uintptr_t(cbtevent* ev, ag* src, ag* dst, const char* skillname, uint64_t id, uint64_t revision)> CallbackSignature;

		explicit EventSequencer(const CallbackSignature& pCallback);
		virtual ~EventSequencer();

		// delete copy and move
		EventSequencer(const EventSequencer& pOther) = delete;
		EventSequencer(EventSequencer&& pOther) noexcept = delete;
		EventSequencer& operator=(const EventSequencer& pOther) = delete;
		EventSequencer& operator=(EventSequencer&& pOther) noexcept = delete;

		struct Event {
			struct : cbtevent {
				bool Present = false;
			} Ev;

			struct : ag {
				std::string NameStorage;
				bool Present = false;
			} Source;

			struct : ag {
				std::string NameStorage;
				bool Present = false;
			} Destination;

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
		std::jthread mThread;
		uint64_t mNextId = 2; // Events start with ID 2 for some reason (it is always like that and no plans to change)
		bool mThreadRunning = false;

		template<bool First = true>
		void Runner(uint64_t idToProcess) {
			std::unique_lock guard(mElementsMutex);

			// Process events with the expected ID or events with ID 0 as long as it's the first iteration, to avoid picking up events with ID 0 when calling Runner<false);
			// For events with ID 0, Runner<false>(0) will process all events with ID 0
			if (!mElements.empty() && (mElements.begin()->Id == idToProcess || (mElements.begin()->Id == 0 && First))) {
				mThreadRunning = true;
				auto item = mElements.extract(mElements.begin());
				guard.unlock();
				EventInternal(item.value());
				mThreadRunning = false;
				Runner<false>(item.value().Id);

				if (First && item.value().Id != 0) {
					++mNextId;
				}
			}
		}

		void EventInternal(Event& pElem);
	};
} // namespace ArcdpsExtension
