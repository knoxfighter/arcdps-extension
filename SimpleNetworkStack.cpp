#include <cstring>
#include <iostream>

#include "SimpleNetworkStack.h"


SimpleNetworkStack::SimpleNetworkStack() {
	mHandle = curl_easy_init();

	if (!mHandle) {
		std::cout << "curl_easy_init() failed" << std::endl;
		exit(1);
	}

	mThread = std::move(std::jthread([this](const std::stop_token& stopToken) {
		runner(stopToken);
	}));
}

SimpleNetworkStack::Result SimpleNetworkStack::get(const std::string& pUrl) {
	mLastResponseBuffer.clear();
	curl_easy_reset(mHandle);

	if (auto res = curl_easy_setopt(mHandle, CURLOPT_URL, pUrl.c_str()); res != CURLE_OK) {
		return std::unexpected(Error{ErrorType::OptUrlError, curl_easy_strerror(res)});
	}
	if (auto res = curl_easy_setopt(mHandle, CURLOPT_FOLLOWLOCATION, 1l); res != CURLE_OK) {
		return std::unexpected(Error{ErrorType::OptFollowLocationError, curl_easy_strerror(res)});
	}
	if (auto res = curl_easy_setopt(mHandle, CURLOPT_WRITEFUNCTION, ResponseCallback); res != CURLE_OK) {
		return std::unexpected(Error{ErrorType::OptWriteFuncError, curl_easy_strerror(res)});
	}
	if (auto res = curl_easy_setopt(mHandle, CURLOPT_WRITEDATA, &mLastResponseBuffer); res != CURLE_OK) {
		return std::unexpected(Error{ErrorType::OptWriteDataError, curl_easy_strerror(res)});
	}

	auto res = curl_easy_perform(mHandle);
	if (res == CURLE_OK) {
		curl_easy_getinfo(mHandle, CURLINFO_RESPONSE_CODE, &mLastResponseCode);
	} else {
		return std::unexpected(Error{ErrorType::PerformError, curl_easy_strerror(res)});
	}

	return Response{mLastResponseBuffer, mLastResponseCode};
}

SimpleNetworkStack::~SimpleNetworkStack() {
	if (mThread.joinable()) {
		mThread.request_stop();
		mThread.join();
	}
	curl_easy_cleanup(mHandle);
}

size_t SimpleNetworkStack::ResponseCallback(void* pContent, size_t pSize, size_t pNMemb, void* pUserP) {
	static_cast<std::string*>(pUserP)->append(static_cast<char*>(pContent), pSize * pNMemb);
	return pSize * pNMemb;
}

void SimpleNetworkStack::runner(const std::stop_token& pToken) {
	while (true) {
		std::unique_lock lock(mQueueMutex);

		// wait until something returned
		mQueueCv.wait(lock, pToken, [this]() {
			return !mJobQueue.empty();
		});
		if (pToken.stop_requested()) {
			break;
		}
		auto element = std::move(mJobQueue.front());
		mJobQueue.pop();

		lock.unlock();

		auto response = get(element.Url);

		switch (element.Type) {
			case QueueElementType::Function:
				element.Func(response);
				break;
			case QueueElementType::Promise:
				element.Promise.set_value(response);
				break;
		}
	}
}

void SimpleNetworkStack::QueueGet(const std::string& pUrl, const SimpleNetworkStack::ResultFunc& pFunc) {
	// queue element and notify cv
	std::lock_guard lock(mQueueMutex);

	mJobQueue.emplace(QueueElementType::Function, pUrl, pFunc);

	mQueueCv.notify_one();
}

void SimpleNetworkStack::QueueGet(const std::string& pUrl, std::promise<Result> pPromise) {
	std::lock_guard lock(mQueueMutex);

	mJobQueue.emplace(QueueElementType::Promise, pUrl, std::move(pPromise));

	mQueueCv.notify_one();
}

SimpleNetworkStack::QueueElement::QueueElement(SimpleNetworkStack::QueueElement&& pOther) noexcept {
	switch (Type) {
		case QueueElementType::Function:
			Func.~function();
			break;
		case QueueElementType::Promise:
			Promise.~promise();
			break;
	}

	// reset self, if this is not done, there will be a sigsegv when the Promise is moved
	std::memset(this, 0, sizeof(SimpleNetworkStack::QueueElement));

	Type = pOther.Type;
	Url = std::move(pOther.Url);
	switch (Type) {
		case QueueElementType::Function:
			Func = pOther.Func;
			break;
		case QueueElementType::Promise:
			Promise = std::move(pOther.Promise);
			break;
	}
}

SimpleNetworkStack::QueueElement& SimpleNetworkStack::QueueElement::operator=(SimpleNetworkStack::QueueElement&& pOther) noexcept {
	Type = pOther.Type;
	Url = pOther.Url;
	switch (Type) {
		case QueueElementType::Function:
			Func = pOther.Func;
			break;
		case QueueElementType::Promise:
			Promise = std::move(pOther.Promise);
			break;
	}
	return *this;
}

SimpleNetworkStack::QueueElement::~QueueElement() {
	switch (Type) {
		case QueueElementType::Function:
			Func.~function();
			break;
		case QueueElementType::Promise:
			Promise.~promise();
			break;
	}
}

SimpleNetworkStack::QueueElement::QueueElement(SimpleNetworkStack::QueueElementType pType, const std::string& pUrl, const SimpleNetworkStack::ResultFunc& pFunc) : Type(pType), Url(pUrl), Func(pFunc) {}
SimpleNetworkStack::QueueElement::QueueElement(SimpleNetworkStack::QueueElementType pType, const std::string& pUrl, std::promise<SimpleNetworkStack::Result> pPromise) : Type(pType), Url(pUrl), Promise(std::move(pPromise)) {}
