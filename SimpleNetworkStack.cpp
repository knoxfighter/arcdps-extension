#include <iostream>

#include "SimpleNetworkStack.h"

ArcdpsExtension::SimpleNetworkStack::SimpleNetworkStack() {
	mHandle = curl_easy_init();

	if (!mHandle) {
		std::cout << "curl_easy_init() failed" << std::endl;
		exit(1);
	}

	mThread = std::move(std::jthread([this](const std::stop_token& stopToken) {
		runner(stopToken);
	}));
}

ArcdpsExtension::SimpleNetworkStack::Result ArcdpsExtension::SimpleNetworkStack::get(const QueueElement& pElement) {
	mLastResponseBuffer.clear();
	curl_easy_reset(mHandle);

	if (auto res = curl_easy_setopt(mHandle, CURLOPT_URL, pElement.Url.c_str()); res != CURLE_OK) {
		return std::unexpected(Error{ErrorType::OptUrlError, curl_easy_strerror(res)});
	}
	if (auto res = curl_easy_setopt(mHandle, CURLOPT_FOLLOWLOCATION, 1l); res != CURLE_OK) {
		return std::unexpected(Error{ErrorType::OptFollowLocationError, curl_easy_strerror(res)});
	}
	if (auto res = curl_easy_setopt(mHandle, CURLOPT_USERAGENT, mUserAgent.c_str())) {
		return std::unexpected(Error{ErrorType::OptUseragentError, curl_easy_strerror(res)});
	}

	FILE* fp = nullptr;
	// set write data if there is a response
	if (pElement.Filepath.empty()) {
		if (auto res = curl_easy_setopt(mHandle, CURLOPT_WRITEFUNCTION, ResponseBufferWriteFunction); res != CURLE_OK) {
			return std::unexpected(Error{ErrorType::OptWriteFuncError, curl_easy_strerror(res)});
		}
		if (auto res = curl_easy_setopt(mHandle, CURLOPT_WRITEDATA, &mLastResponseBuffer); res != CURLE_OK) {
			return std::unexpected(Error{ErrorType::OptWriteDataError, curl_easy_strerror(res)});
		}
	} else {
		if (auto res = curl_easy_setopt(mHandle, CURLOPT_WRITEFUNCTION, NULL); res != CURLE_OK) {
			return std::unexpected(Error{ErrorType::OptWriteFuncError, curl_easy_strerror(res)});
		}
		fopen_s(&fp, pElement.Filepath.string().c_str(), "wb");
		if (auto res = curl_easy_setopt(mHandle, CURLOPT_WRITEDATA, fp); res != CURLE_OK) {
			return std::unexpected(Error{ErrorType::OptWriteDataError, curl_easy_strerror(res)});
		}
	}

	auto res = curl_easy_perform(mHandle);
	if (fp != nullptr) {
		fclose(fp);
	}
	if (res == CURLE_OK) {
		curl_easy_getinfo(mHandle, CURLINFO_RESPONSE_CODE, &mLastResponseCode);
	} else {
		return std::unexpected(Error{ErrorType::PerformError, curl_easy_strerror(res)});
	}

	return Response{mLastResponseBuffer, mLastResponseCode};
}

ArcdpsExtension::SimpleNetworkStack::~SimpleNetworkStack() {
	if (mThread.joinable()) {
		mThread.request_stop();
		mThread.join();
	}
	curl_easy_cleanup(mHandle);
}

size_t ArcdpsExtension::SimpleNetworkStack::ResponseBufferWriteFunction(void* pContent, size_t pSize, size_t pNMemb, void* pUserP) {
	static_cast<std::string*>(pUserP)->append(static_cast<char*>(pContent), pSize * pNMemb);
	return pSize * pNMemb;
}

void ArcdpsExtension::SimpleNetworkStack::runner(const std::stop_token& pToken) {
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

		auto response = get(element);

		if (auto* func = std::get_if<ResultFunc>(&element.Callback)) {
			(*func)(response);
		} else if (auto* promise = std::get_if<ResultPromise>(&element.Callback)) {
			promise->set_value(response);
		}
	}
}

void ArcdpsExtension::SimpleNetworkStack::QueueGet(const std::string& pUrl, const std::filesystem::path& pFilepath) {
	std::lock_guard lock(mQueueMutex);

	mJobQueue.emplace(pUrl, std::monostate(), pFilepath);

	mQueueCv.notify_one();
}
void ArcdpsExtension::SimpleNetworkStack::QueueGet(const std::string& pUrl, const SimpleNetworkStack::ResultFunc& pFunc, const std::filesystem::path& pFilepath) {
	std::lock_guard lock(mQueueMutex);

	mJobQueue.emplace(pUrl, pFunc, pFilepath);

	mQueueCv.notify_one();
}
void ArcdpsExtension::SimpleNetworkStack::QueueGet(const std::string& pUrl, std::promise<Result> pPromise, const std::filesystem::path& pFilepath) {
	std::lock_guard lock(mQueueMutex);

	mJobQueue.emplace(pUrl, std::move(pPromise), pFilepath);

	mQueueCv.notify_one();
}
