#include "SimpleNetworkStack.h"

#include <gtest/gtest.h>

class SimpleNetworkStackTests : public ::testing::Test {
public:
	static void TearDownTestSuite() {
		g_singletonManagerInstance.Shutdown();
	}
};

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
