#pragma once

#include "Singleton.h"

#include <condition_variable>
#include <cstddef>
#include <curl/curl.h>
#include <expected>
#include <filesystem>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stop_token>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>
#include <variant>

namespace ArcdpsExtension {
	class SimpleNetworkStack final : public Singleton<SimpleNetworkStack> {
	public:
		SimpleNetworkStack();
		~SimpleNetworkStack() override;

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
			OptUseragentError,
		};
		struct Error {
			ErrorType Type;
			std::string Message;
		};
		using Result = std::expected<Response, Error>;
		using ResultFunc = std::function<void(const Result&)>;
		using ResultPromise = std::promise<Result>;

		/**
	     * Performs a Get-Request.
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
	     * @param pFilepath Optional filepath to save the response to
	     */
		void QueueGet(const std::string& pUrl, const std::filesystem::path& pFilepath = "");

		/**
	     * Performs a Get-Request and will call pFunc when the response is gathered.
	     * If a filepath is provided, the response will be saved in that file and the Result will have an empty string.
	     * <br>
	     * Usage:
	     * @code
	     * networkStack.QueueGet("https://example.com/", [](const SimpleNetworkStack::Result& response){
		 * 	if (response) {
		 * 		std::cout << response.value().Code << " --> " << response.value().Message << std::endl;
		 * 	}
		 * });
	     * @endcode
	     *
	     * @param pUrl The URL to call
	     * @param pFunc The function that will be called with the response
	     * @param pFilepath Optional filepath to save the response to
	     */
		void QueueGet(const std::string& pUrl, const ResultFunc& pFunc, const std::filesystem::path& pFilepath = "");

		/**
	     * Performs a Get-Request and will resolve the promise when the response is gathered.
	     * If a filepath is provided, the response will be saved in that file and the Result will have an empty string.
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
	     * @param pFilepath Optional filepath to save the response to
	     */
		void QueueGet(const std::string& pUrl, ResultPromise pPromise, const std::filesystem::path& pFilepath = "");

		/**
		 * Set a different user-agent instead of the default "ArcdpsExtension/1.0"
		 * @param pUserAgent value to set
		 */
		void SetUserAgent(const std::string& pUserAgent) {
			mUserAgent = pUserAgent;
		}

		/**
		 * URL encode a string. Wrapper for `curl_easy_escape`.
		 * @param pStr string to encode
		 * @return encoded string
		 */
		[[nodiscard]] std::string UrlEncode(std::string_view pStr) const;

	private:
		struct QueueElement {
			using Variant = std::variant<std::monostate, ResultFunc, ResultPromise>;
			std::string Url;
			Variant Callback;
			// only set this when a download should take place
			std::filesystem::path Filepath;

			QueueElement(std::string pUrl, Variant pCallback, std::filesystem::path pFilepath) : Url(std::move(pUrl)), Callback(std::move(pCallback)), Filepath(std::move(pFilepath)) {}
			~QueueElement() = default;

			QueueElement(const QueueElement&) = delete;
			QueueElement(QueueElement&& pOther) noexcept = default;
			QueueElement& operator=(const QueueElement&) = delete;
			QueueElement& operator=(QueueElement&& pOther) noexcept = default;
		};

		CURL* mHandle = nullptr;
		std::string mLastResponseBuffer;
		long mLastResponseCode = 0;

		std::jthread mThread;
		std::queue<QueueElement> mJobQueue;
		std::mutex mQueueMutex;
		std::condition_variable_any mQueueCv;

		std::string mUserAgent = "ArcdpsExtension/1.0";

		static size_t ResponseBufferWriteFunction(void* pContent, size_t pSize, size_t pNMemb, void* pUserP);
		SimpleNetworkStack::Result get(const QueueElement& pElement);
		void runner(const std::stop_token& pToken);
	};
} // namespace ArcdpsExtension
