#include "arcdps_structs_slim.h"
#include "EventSequencer.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <limits>
#include <mutex>
#include <random>
#include <ranges>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using namespace ArcdpsExtension;

std::vector<std::byte> random_bytes(
		std::size_t count,
		std::mt19937_64& rng
) {
	std::uniform_int_distribution dist(0, 255);

	std::vector<std::byte> out(count);
	std::ranges::generate(out, [&] {
		return static_cast<std::byte>(dist(rng));
	});

	return out;
}

template<typename T>
T* random_value(
		std::mt19937_64& rng
) {
	T* out = new T;
	auto b = random_bytes(sizeof(T), rng);
	memcpy(out, b.data(), sizeof(T));
	return out;
}

bool operator==(const decltype(EventSequencer::Event::Ev)& lhs, const cbtevent& rhs) {
	return lhs.time == rhs.time && lhs.src_agent == rhs.src_agent && lhs.dst_agent == rhs.dst_agent && lhs.value == rhs.value && lhs.buff_dmg == rhs.buff_dmg && lhs.overstack_value == rhs.overstack_value && lhs.skillid == rhs.skillid && lhs.src_instid == rhs.src_instid && lhs.dst_instid == rhs.dst_instid && lhs.src_master_instid == rhs.src_master_instid && lhs.dst_master_instid == rhs.dst_master_instid && lhs.iff == rhs.iff && lhs.buff == rhs.buff && lhs.result == rhs.result && lhs.is_activation == rhs.is_activation && lhs.is_buffremove == rhs.is_buffremove && lhs.is_ninety == rhs.is_ninety && lhs.is_fifty == rhs.is_fifty && lhs.is_moving == rhs.is_moving && lhs.is_statechange == rhs.is_statechange && lhs.is_flanking == rhs.is_flanking && lhs.is_shields == rhs.is_shields && lhs.is_offcycle == rhs.is_offcycle && lhs.pad61 == rhs.pad61 && lhs.pad62 == rhs.pad62 && lhs.pad63 == rhs.pad63 && lhs.pad64 == rhs.pad64;
}

bool operator==(const EventSequencer::Event::Agent& lhs, const ag& rhs) {
	return lhs.NameStorage == rhs.name && lhs.id == rhs.id && lhs.prof == rhs.prof && lhs.elite == rhs.elite && lhs.team == rhs.team;
}

namespace {
	std::vector<EventSequencer::Event> events;
}

class EventSequencerTests : public ::testing::Test {
public:
	static void SetUpTestSuite() {
		events.reserve(1000);

		std::mt19937_64 rng{std::random_device{}()};

		for (int i = 2; i < 1000; ++i) {
			auto* ev = random_value<cbtevent>(rng);
			auto* src = random_value<ag>(rng);
			auto src_name = std::string("src") + std::to_string(i);
			src->name = src_name.c_str();
			auto* dst = random_value<ag>(rng);
			auto dst_name = std::string("dst") + std::to_string(i);
			dst->name = dst_name.c_str();
			std::uniform_int_distribution dist(std::numeric_limits<std::uint64_t>::min(), std::numeric_limits<std::uint64_t>::max());
			const auto* skillname = reinterpret_cast<const char*>(dist(rng));
			events.emplace_back(ev, src, dst, skillname, i, 0);
			delete ev;
			delete src;
			delete dst;
		}

		std::ranges::shuffle(events, rng);
	}
};

TEST_F(EventSequencerTests, SingleThreaded) {
	uint64_t nextId = 2;
	std::thread::id threadId;

	auto callback = [&nextId, &threadId](cbtevent* ev, ag* src, ag* dst, const char* skillname, uint64_t id, uint64_t revision) -> uintptr_t {
		auto it = std::ranges::find_if(events, [&](const EventSequencer::Event& e) {
			return e.Id == id;
		});
		EXPECT_NE(it, events.end());

		EXPECT_EQ(it->Id, id);
		EXPECT_EQ(it->Ev, *ev);
		EXPECT_TRUE(it->Source == *src);
		EXPECT_TRUE(it->Destination == *dst);
		EXPECT_EQ(it->Skillname, skillname);
		EXPECT_EQ(it->Revision, revision);

		// also check if the order is correct
		EXPECT_EQ(it->Id, nextId);
		++nextId;

		// check if we are in the correct thread
		if (threadId == std::thread::id()) {
			threadId = std::this_thread::get_id();
		}
		EXPECT_EQ(threadId, std::this_thread::get_id());

		return 0;
	};

	EventSequencer sequencer(callback);

	for (auto& event : events) {
		auto& src = event.Source;
		src.name = src.NameStorage.c_str();
		auto& dst = event.Destination;
		dst.name = dst.NameStorage.c_str();
		sequencer.ProcessEvent(&event.Ev, &src, &dst, event.Skillname, event.Id, event.Revision);
	}

	// wait until all events are processed
	while (sequencer.EventsPending()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

TEST_F(EventSequencerTests, SingleThreadedZero) {
	std::mutex test_mutex;
	std::thread::id threadId;

	auto callback = [&](cbtevent* ev, ag* src, ag* dst, const char* skillname, uint64_t id, uint64_t revision) -> uintptr_t {
		if (threadId == std::thread::id()) {
			threadId = std::this_thread::get_id();
		}
		EXPECT_EQ(threadId, std::this_thread::get_id());

		return 0;
	};

	EventSequencer sequencer(callback);

	// Send first event to get correct thread
	auto e = std::ranges::find_if(events, [&](const EventSequencer::Event& e) {
		return e.Id == 2;
	});
	EXPECT_NE(e, events.end());
	sequencer.ProcessEvent(&e->Ev, &e->Source, &e->Destination, e->Skillname, e->Id, e->Revision);

	// wait until all events are processed
	while (sequencer.EventsPending()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	// send event 0
	auto event0 = events[0];
	event0.Id = 0;
	sequencer.ProcessEvent(&event0.Ev, &event0.Source, &event0.Destination, event0.Skillname, event0.Id, event0.Revision);

	// wait until all events are processed
	while (sequencer.EventsPending()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

TEST_F(EventSequencerTests, MultiThreaded) {
	uint64_t nextId = 2;
	std::thread::id threadId;

	auto callback = [&nextId, &threadId](cbtevent* ev, ag* src, ag* dst, const char* skillname, uint64_t id, uint64_t revision) -> uintptr_t {
		auto it = std::ranges::find_if(events, [&](const EventSequencer::Event& e) {
			return e.Id == id;
		});
		EXPECT_NE(it, events.end());

		EXPECT_EQ(it->Id, id);
		EXPECT_EQ(it->Ev, *ev);
		EXPECT_EQ(it->Source, *src);
		EXPECT_EQ(it->Destination, *dst);
		EXPECT_EQ(it->Skillname, skillname);
		EXPECT_EQ(it->Revision, revision);

		// also check if the order is correct
		EXPECT_EQ(it->Id, nextId);
		++nextId;

		// check if we are in the correct thread
		if (threadId == std::thread::id()) {
			threadId = std::this_thread::get_id();
		}
		EXPECT_EQ(threadId, std::this_thread::get_id());

		return 0;
	};

	EventSequencer sequencer(callback);

	// 4 threads
	constexpr uint64_t threadCount = 4;
	std::thread threads[threadCount];
	auto eventChunks = events | std::views::chunk((events.size() + threadCount - 1) / threadCount);
	for (uint64_t i = 0; i < threadCount; ++i) {
		threads[i] = std::thread([&sequencer, &eventChunks, i]() {
			for (auto& event : eventChunks[i]) {
				sequencer.ProcessEvent(&event.Ev, &event.Source, &event.Destination, event.Skillname, event.Id, event.Revision);
			}
		});
	}

	for (auto& thread : threads) {
		thread.join();
	}

	// wait until all events are processed
	while (sequencer.EventsPending()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
