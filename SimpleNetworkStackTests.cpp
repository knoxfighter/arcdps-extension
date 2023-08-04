#include "SimpleNetworkStack.h"

#include <gtest/gtest.h>

namespace {
	const std::filesystem::path TEMPFILE = std::filesystem::temp_directory_path() / "out.tmp";
}

class SimpleNetworkStackTests : public ::testing::Test {
public:
	static void TearDownTestSuite() {
		g_singletonManagerInstance.Shutdown();
	}

protected:
	void TearDown() override {
		if (std::filesystem::exists(TEMPFILE)) {
			std::filesystem::remove(TEMPFILE);
		}

		::testing::Test::TearDown();
	}
};

TEST_F(SimpleNetworkStackTests, DISABLED_Get) {
	auto& networkStack = SimpleNetworkStack::instance();

	// FIXME call and forget - how to handle?
	networkStack.QueueGet("https://google.com");
}

TEST_F(SimpleNetworkStackTests, GetCallback) {
	auto& networkStack = SimpleNetworkStack::instance();

	std::promise<void> promise;
	auto future = promise.get_future();
	networkStack.QueueGet("https://google.com", [&promise, this](const SimpleNetworkStack::Result& response) {
		ASSERT_TRUE(response.has_value());
		ASSERT_EQ(response.value().Code, 200);
		promise.set_value();
	});
	future.get();
}

TEST_F(SimpleNetworkStackTests, GetPromise) {
	auto& networkStack = SimpleNetworkStack::instance();

	std::promise<SimpleNetworkStack::Result> promise;
	auto future = promise.get_future();
	networkStack.QueueGet("https://google.com", std::move(promise));
	auto response = future.get();
	ASSERT_TRUE(response.has_value());
	ASSERT_EQ(response.value().Code, 200);
}

TEST_F(SimpleNetworkStackTests, DISABLED_Download) {
	auto& networkStack = SimpleNetworkStack::instance();

	// FIXME call and forget - how to handle?
	networkStack.QueueGet("https://example.com/", TEMPFILE);
}

TEST_F(SimpleNetworkStackTests, DownloadCallback) {
	auto& networkStack = SimpleNetworkStack::instance();

	std::promise<void> promise;
	auto future = promise.get_future();
	networkStack.QueueGet(
			"https://example.com/",
			[&promise, this](const SimpleNetworkStack::Result& response) {
				ASSERT_TRUE(response.has_value());
				ASSERT_EQ(response.value().Code, 200);
				ASSERT_TRUE(std::filesystem::exists(TEMPFILE));
				promise.set_value();
			},
			TEMPFILE
	);
	future.get();
}

TEST_F(SimpleNetworkStackTests, DownloadPromise) {
	auto& networkStack = SimpleNetworkStack::instance();

	std::promise<SimpleNetworkStack::Result> promise;
	auto future = promise.get_future();
	networkStack.QueueGet("https://example.com/", std::move(promise), TEMPFILE);
	auto response = future.get();
	ASSERT_TRUE(response.has_value());
	ASSERT_EQ(response.value().Code, 200);
	ASSERT_TRUE(std::filesystem::exists(TEMPFILE));
}
