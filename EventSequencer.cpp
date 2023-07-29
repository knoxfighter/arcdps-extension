#include "EventSequencer.h"

#include <algorithm>

void EventSequencer::ProcessEvent(cbtevent* pEv, ag* pSrc, ag* pDst, const char* pSkillname, uint64_t pId, uint64_t pRevision) {
	std::lock_guard guard(mElementsMutex);
	if (pId == 0) {
		if (mElements.empty()) {
			mCallback(pEv, pSrc, pDst, pSkillname, pId, pRevision);
			// mPriorityElements.emplace(pEv, pSrc, pDst, pSkillname, mNextId, pRevision);
			return;
		}

		// insert it in the prio queue
		mElements.emplace(pEv, pSrc, pDst, pSkillname, mLastId, pRevision);
	} else {
		mLastId = pId;
		mElements.emplace(pEv, pSrc, pDst, pSkillname, pId, pRevision);
	}
}

bool EventSequencer::EventsPending() const {
	return !mElements.empty() || mThreadRunning;
}

void EventSequencer::Reset() {
	std::unique_lock guard(mElementsMutex);
	mElements.clear();
	mNextId = 2;
	mLastId = 2;
}

EventSequencer::EventSequencer(const CallbackSignature& pCallback) : mCallback(pCallback) {
	using namespace std::chrono_literals;
	mThread = std::jthread([this](std::stop_token stoken) {
		while (!stoken.stop_requested()) {
			while (!stoken.stop_requested() && !mElements.empty()) {
				Runner();
			}

			if (stoken.stop_requested()) return;

			std::this_thread::sleep_for(100ms);
		}
	});
}

EventSequencer::~EventSequencer() {
	Shutdown();
}

void EventSequencer::EventInternal(Event& pElem) {
	cbtevent* event = nullptr;
	if (pElem.Ev.Present) {
		event = &pElem.Ev;
	}

	ag* source = nullptr;
	if (pElem.Source.Present) {
		source = &pElem.Source;
		if (source->name) {
			source->name = pElem.Source.NameStorage.c_str();
		}
	}

	ag* dest = nullptr;
	if (pElem.Destination.Present) {
		dest = &pElem.Destination;
		if (dest->name) {
			dest->name = pElem.Destination.NameStorage.c_str();
		}
	}

	mCallback(event, source, dest, pElem.Skillname, pElem.Id, pElem.Revision);
}

void EventSequencer::Shutdown() {
	if (mThread.joinable()) {
		mThread.request_stop();
		mThread.join();
	}
}
