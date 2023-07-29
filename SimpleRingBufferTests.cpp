#include "SimpleRingBuffer.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <ranges>

TEST(SimpleRingBufferTests, PushBackTest) {
	RingBuffer<uint64_t> buffer(10);
	for (int i = 1; i < 7; ++i) {
		buffer.PushBack(i);
		EXPECT_EQ(buffer.Back(), i);
	}

	EXPECT_TRUE(std::ranges::equal(buffer, std::vector<uint64_t>{1, 2, 3, 4, 5, 6}));
	// EXPECT_THAT(buffer, ::testing::ContainerEq(std::vector<uint64_t>{1, 2, 3, 4, 5, 6}));

	for (int i = 7; i < 15; ++i) {
		buffer.PushBack(i);
		EXPECT_EQ(buffer.Back(), i);
	}

	EXPECT_TRUE(std::ranges::equal(buffer, std::vector<uint64_t>{5, 6, 7, 8, 9, 10, 11, 12, 13, 14}));
}

TEST(SimpleRingBufferTests, EmplaceBackTest) {
	RingBuffer<uint64_t> buffer(10);
	for (int i = 1; i < 7; ++i) {
		buffer.EmplaceBack(i);
		EXPECT_EQ(buffer.Back(), i);
	}

	EXPECT_TRUE(std::ranges::equal(buffer, std::vector<uint64_t>{1, 2, 3, 4, 5, 6}));
	// EXPECT_THAT(buffer, ::testing::ContainerEq(std::vector<uint64_t>{1, 2, 3, 4, 5, 6}));

	for (int i = 7; i < 15; ++i) {
		const int j = i;
		buffer.EmplaceBack(j);
		EXPECT_EQ(buffer.Back(), j);
	}

	EXPECT_TRUE(std::ranges::equal(buffer, std::vector<uint64_t>{5, 6, 7, 8, 9, 10, 11, 12, 13, 14}));
}

TEST(SimpleRingBufferTests, ClearTest) {
	RingBuffer<uint64_t> buffer(10);
	for (int i = 1; i < 15; ++i) {
		buffer.EmplaceBack(i);
	}

	EXPECT_EQ(buffer.Size(), 10);

	buffer.Clear();

	EXPECT_EQ(buffer.Size(), 0);
}

TEST(SimpleRingBufferTests, ResizeTest) {
	RingBuffer<uint64_t> buffer(10);

	// resize before anything is in there
	buffer.Resize(11);

	for (int i = 1; i < 15; ++i) {
		buffer.EmplaceBack(i);
	}

	EXPECT_EQ(buffer.Size(), 11);

	buffer.Resize(5);

	EXPECT_TRUE(std::ranges::equal(buffer, std::vector<uint64_t>{10, 11, 12, 13, 14}));
}

TEST(SimpleRingBufferTests, RandomAccessTest) {
	RingBuffer<uint64_t> buffer(10);
	for (int i = 1; i < 15; ++i) {
		buffer.EmplaceBack(i);
	}

	EXPECT_EQ(buffer[0], 5);
	EXPECT_EQ(buffer[1], 6);
	EXPECT_EQ(buffer[2], 7);
	EXPECT_EQ(buffer[3], 8);
	EXPECT_EQ(buffer[4], 9);
	EXPECT_EQ(buffer[5], 10);
	EXPECT_EQ(buffer[6], 11);
	EXPECT_EQ(buffer[7], 12);
	EXPECT_EQ(buffer[8], 13);
	EXPECT_EQ(buffer[9], 14);

	// on const buffer
	const RingBuffer<uint64_t> buffer2 = buffer;
	EXPECT_EQ(buffer2[0], 5);
	EXPECT_EQ(buffer2[1], 6);
	EXPECT_EQ(buffer2[2], 7);
	EXPECT_EQ(buffer2[3], 8);
	EXPECT_EQ(buffer2[4], 9);
	EXPECT_EQ(buffer2[5], 10);
	EXPECT_EQ(buffer2[6], 11);
	EXPECT_EQ(buffer2[7], 12);
	EXPECT_EQ(buffer2[8], 13);
	EXPECT_EQ(buffer2[9], 14);
}

TEST(SimpleRingBufferTests, IteratorTest) {
	RingBuffer<uint64_t> buffer(10);
	buffer.EmplaceBack(1);
	buffer.EmplaceBack(2);
	buffer.EmplaceBack(3);
	buffer.EmplaceBack(4);
	buffer.EmplaceBack(5);
	buffer.EmplaceBack(6);

	int i = 1;
	for (uint64_t value : buffer) {
		EXPECT_EQ(value, i);
		++i;
	}

	buffer.EmplaceBack(7);
	buffer.EmplaceBack(8);
	buffer.EmplaceBack(9);
	buffer.EmplaceBack(10);
	buffer.EmplaceBack(11);
	buffer.EmplaceBack(12);
	buffer.EmplaceBack(13);
	buffer.EmplaceBack(14);

	i = 5;
	for (uint64_t value : buffer) {
		EXPECT_EQ(value, i);
		++i;
	}

	// on const
	const RingBuffer<uint64_t> buffer2 = buffer;
	i = 5;
	for (uint64_t value : buffer2) {
		EXPECT_EQ(value, i);
		++i;
	}
}

TEST(SimpleRingBufferTests, ReverseIteratorTest) {
	RingBuffer<uint64_t> buffer(10);
	buffer.EmplaceBack(1);
	buffer.EmplaceBack(2);
	buffer.EmplaceBack(3);
	buffer.EmplaceBack(4);
	buffer.EmplaceBack(5);
	buffer.EmplaceBack(6);

	int i = 6;
	for (uint64_t value : buffer | std::ranges::views::reverse) {
		EXPECT_EQ(value, i);
		--i;
	}

	buffer.EmplaceBack(7);
	buffer.EmplaceBack(8);
	buffer.EmplaceBack(9);
	buffer.EmplaceBack(10);
	buffer.EmplaceBack(11);
	buffer.EmplaceBack(12);
	buffer.EmplaceBack(13);
	buffer.EmplaceBack(14);

	i = 14;
	for (uint64_t value : buffer | std::ranges::views::reverse) {
		EXPECT_EQ(value, i);
		--i;
	}

	// on const
	const RingBuffer<uint64_t> buffer2 = buffer;
	i = 14;
	for (uint64_t value : buffer2 | std::ranges::views::reverse) {
		EXPECT_EQ(value, i);
		--i;
	}
}

TEST(SimpleRingBufferTests, IteratorOperatorMinusIterator) {
	RingBuffer<uint64_t> buffer(10);
	buffer.PushBack(1);
	buffer.PushBack(2);
	buffer.PushBack(3);
	buffer.PushBack(4);
	buffer.PushBack(5);
	buffer.PushBack(6);

	EXPECT_EQ(buffer.end() - buffer.begin(), 6);
	EXPECT_EQ(buffer.begin() - buffer.end(), -6);

	buffer.PushBack(7);
	buffer.PushBack(8);
	buffer.PushBack(9);
	buffer.PushBack(10);
	buffer.PushBack(11);
	buffer.PushBack(12);

	EXPECT_EQ(buffer.end() - buffer.begin(), 10);
	EXPECT_EQ(buffer.begin() - buffer.end(), -10);

	EXPECT_EQ((buffer.end() - 3) - (buffer.begin() + 1), 6);
	EXPECT_EQ((buffer.begin() + 1) - (buffer.end() - 3), -6);
}

TEST(SimpleRingBufferTests, IteratorOperatorMinusNumber) {
	RingBuffer<uint64_t> buffer(10);
	buffer.PushBack(1);
	buffer.PushBack(2);
	buffer.PushBack(3);
	buffer.PushBack(4);
	buffer.PushBack(5);
	buffer.PushBack(6);

	// check the pointer of the iterator directly, to not have dependency on `Iterator::operator+`
	EXPECT_EQ((buffer.end() - 3).operator->(), buffer.begin().operator->() + 3);

	// check begin iterator, when going to begin
	EXPECT_EQ(buffer.end() - 6, buffer.begin());

	// with dependency on `operator+`
	EXPECT_EQ(buffer.end() - 5, buffer.begin() + 1);

	// check with mCurrent in the midst of elements
	buffer.PushBack(7);
	buffer.PushBack(8);
	buffer.PushBack(9);
	buffer.PushBack(10);
	buffer.PushBack(11);
	buffer.PushBack(12);
	buffer.PushBack(13);
	buffer.PushBack(14);

	// check the pointer of the iterator directly, to not have dependency on `Iterator::operator+`
	EXPECT_EQ((buffer.end() - 3).operator->(), buffer.begin().operator->() - 3);

	// check begin iterator, when going to begin
	EXPECT_EQ(buffer.end() - 10, buffer.begin());

	// with dependency on `operator+`
	EXPECT_EQ(buffer.end() - 9, buffer.begin() + 1);
}

TEST(SimpleRingBufferTests, IteratorOperatorPlusNumber) {
	RingBuffer<uint64_t> buffer(10);
	buffer.PushBack(1);
	buffer.PushBack(2);
	buffer.PushBack(3);
	buffer.PushBack(4);
	buffer.PushBack(5);
	buffer.PushBack(6);

	// check the pointer of the iterator directly, to not have dependency on `Iterator::operator+`
	EXPECT_EQ((buffer.begin() + 3).operator->(), buffer.begin().operator->() + 3);

	// check end iterator, when going to end
	EXPECT_EQ(buffer.begin() + 6, buffer.end());

	// with dependency on `operator-`
	EXPECT_EQ(buffer.begin() + 5, buffer.end() - 1);

	// // check with mCurrent in the midst of elements
	buffer.PushBack(7);
	buffer.PushBack(8);
	buffer.PushBack(9);
	buffer.PushBack(10);
	buffer.PushBack(11);
	buffer.PushBack(12);
	buffer.PushBack(13);
	buffer.PushBack(14);

	// check the pointer of the iterator directly, to not have dependency on `Iterator::operator+`
	EXPECT_EQ((buffer.begin() + 3).operator->(), buffer.begin().operator->() + 3);

	// check begin iterator, when going to begin
	EXPECT_EQ(buffer.begin() + 10, buffer.end());

	// with dependency on `operator+`
	EXPECT_EQ(buffer.begin() + 9, buffer.end() - 1);
}

TEST(SimpleRingBufferTests, IteratorRandomAccessTest) {
	RingBuffer<uint64_t> buffer(10);
	for (int i = 1; i < 15; ++i) {
		buffer.EmplaceBack(i);
	}

	const auto& it = buffer.begin();

	EXPECT_EQ(it[0], 5);
	EXPECT_EQ(it[1], 6);
	EXPECT_EQ(it[2], 7);
	EXPECT_EQ(it[3], 8);
	EXPECT_EQ(it[4], 9);
	EXPECT_EQ(it[5], 10);
	EXPECT_EQ(it[6], 11);
	EXPECT_EQ(it[7], 12);
	EXPECT_EQ(it[8], 13);
	EXPECT_EQ(it[9], 14);
}

TEST(SimpleRingBufferTests, IteratorOrderingTest) {
	RingBuffer<uint64_t> buffer(10);
	for (int i = 1; i < 15; ++i) {
		buffer.EmplaceBack(i);
	}

	EXPECT_GT(buffer.end(), buffer.begin());
	EXPECT_GT(buffer.end() - 1, buffer.begin() + 1);
	EXPECT_LT(buffer.begin(), buffer.end());
	EXPECT_LT(buffer.begin() + 1, buffer.end() - 1);
}
