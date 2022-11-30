#include "gtest/gtest.h"
#include "SpinLockMutex.h"
#include <thread>

int two_thread_add(const int step_count, const bool use_mutex) {
	using namespace std::literals;
	int value = 0;
	SpinLockMutex mutex;
	auto adder = [&value, use_mutex, &mutex, step_count]() {
		for (int i = 0; i < step_count; i++) {
			if (use_mutex) {
				mutex.lock();
			}
			value += 1;
			if (use_mutex) {
				mutex.unlock();
			}
		}
	};

	std::thread tid1(adder);
	std::thread tid2(adder);

	tid1.join();
	tid2.join();

	return value;
}

TEST(SpinLockMutexTest, TwoThreadAddWithMutex) {
	int step_count = 100000;
	EXPECT_EQ(two_thread_add(step_count, true), step_count * 2);
}

TEST(SpinLockMutexTest, TwoThreadAddDirectly) {
	int step_count = 100000;
	int expect = two_thread_add(step_count, false);

	EXPECT_GE(expect, step_count);
	EXPECT_LE(expect, step_count * 2);
}
