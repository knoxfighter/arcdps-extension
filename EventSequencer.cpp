#include "EventSequencer.h"

#include <algorithm>
#include <chrono>
#include <utility>

void ArcdpsExtension::EventSequencer::ProcessEvent(cbtevent* pEv, ag* pSrc, ag* pDst, const char* pSkillname, uint64_t pId, uint64_t pRevision) {
	std::lock_guard guard(mElementsMutex);
	mElements.emplace(pEv, pSrc, pDst, pSkillname, pId, pRevision);
	mNewElement.notify_all();
}

bool ArcdpsExtension::EventSequencer::EventsPending() const {
	return !mElements.empty() || mThreadRunning;
}

void ArcdpsExtension::EventSequencer::Reset() {
	std::unique_lock guard(mElementsMutex);
	mElements.clear();
	mNextId = 2;
}

ArcdpsExtension::EventSequencer::EventSequencer(CallbackSignature pCallback) : mCallback(std::move(pCallback)) {
	using namespace std::chrono_literals;
	mThread = std::jthread([this](std::stop_token stoken) {
		std::unique_lock guard(mElementsMutex, std::defer_lock);
		while (!stoken.stop_requested()) {
			guard.lock();
			mNewElement.wait(guard, stoken, [this] {
				return !mElements.empty() && mElements.begin()->Id <= mNextId;
			});
			if (stoken.stop_requested()) return;

			// If we get here, the predicate is already checked and we can assume that mElements is not empty
			auto item = mElements.extract(mElements.begin());
			guard.unlock();
			Event& event = item.value();
			EventInternal(event);

			if (event.Id == mNextId) {
				++mNextId;
			}
		}
	});
}

ArcdpsExtension::EventSequencer::~EventSequencer() {
	Shutdown();
}

void ArcdpsExtension::EventSequencer::EventInternal(Event& pElem) const {
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

void ArcdpsExtension::EventSequencer::Shutdown() {
	if (mThread.joinable()) {
		mThread.request_stop();
		mThread.join();
	}
}
