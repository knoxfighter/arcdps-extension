#pragma once

#include "Singleton.h"
#include <condition_variable>
#include <curl/curl.h>
#include <expected>
#include <functional>
#include <future>
#include <queue>
#include <thread>

class SimpleNetworkStack : public Singleton<SimpleNetworkStack> {
public:
	SimpleNetworkStack();
	~SimpleNetworkStack();

	struct Response {
		std::string Message;
		long Code;
	};
	enum class ErrorType {
		PerformError,
		OptUrlError,
		OptFollowLocationError,
		OptWriteFuncError,
		OptWriteDataError,
	};
	struct Error {
		ErrorType Type;
		std::string Message;
	};
	using Result = std::expected<Response, Error>;
	using ResultFunc = std::function<void(const Result&)>;

	/**
     * Performs a Get-Request and will call pFunc when the response is gathered.
     * <br>
     * Usage:
     * @code
     * networkStack.QueueGet("https://example.com/", [](const SimpleNetworkStack::Result& response){
	 * 	if (response) {
	 * 		std::cout << response.value().Code << " --> " << response.value().Message << std::endl;
	 * 	}
	 * });
     * 	@endcode
     *
     * @param pUrl The URL to call
     * @param pFunc The function that will be called with the response
     */
	void QueueGet(const std::string& pUrl, const ResultFunc& pFunc);
	/**
     * Performs a Get-Request and will resolve the promise when the response is gathered.
     * <br>
     * Usage:
     * @code
     * std::promise<SimpleNetworkStack::Result> promise;
     * auto future = barrier.get_future();
     * QueueGet("https://example.com/", std::move(promise));
     * @endcode
     *
     * @param pUrl The URL to call
     * @param pPromise Promise that will resolved with the response
     */
	void QueueGet(const std::string& pUrl, std::promise<Result> pPromise);

private:
	enum class QueueElementType {
		Function,
		Promise,
	};

	struct QueueElement {
		QueueElementType Type;
		std::string Url;
		union {
			ResultFunc Func;
			std::promise<SimpleNetworkStack::Result> Promise;
		};

		QueueElement(QueueElementType pType, const std::string& pUrl, const ResultFunc& pFunc);
		QueueElement(SimpleNetworkStack::QueueElementType pType, const std::string& pUrl, std::promise<SimpleNetworkStack::Result> pPromise);

		~QueueElement();

		QueueElement(const QueueElement&) = delete;
		QueueElement(QueueElement&& pOther) noexcept;
		QueueElement& operator=(const QueueElement&) = delete;
		QueueElement& operator=(QueueElement&& pOther) noexcept;
	};

	CURL* mHandle = nullptr;
	std::string mLastResponseBuffer;
	long mLastResponseCode = 0;

	std::jthread mThread;
	std::queue<QueueElement> mJobQueue;
	std::mutex mQueueMutex;
	std::condition_variable_any mQueueCv;

	static size_t ResponseCallback(void* pContent, size_t pSize, size_t pNMemb, void* pUserP);
	SimpleNetworkStack::Result get(const std::string& pUrl);
	void runner(const std::stop_token& pToken);
};
