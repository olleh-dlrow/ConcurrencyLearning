#include <thread>
#include <vector>
#include <random>
#include "ThreadSafeQueue.h"
#include "gtest/gtest.h"

TEST(ThreadSafeQueueTest, ProducerConsumer) {
	ThreadSafeQueue<int> queue;
	int products_per_producer = 1000, products_per_consumer = 1000;
	int consumer_cnt = 10, producer_cnt = 10;

	auto producer = [&queue, products_per_producer]() {
		int remain_products = products_per_producer;
		while (remain_products--) {
			int value = rand() % 1000;
			queue.push(value);
		}
	};

	auto consumer = [&queue, products_per_consumer]() {
		int remain_products = products_per_consumer;
		while (remain_products) {
			int value;
			queue.wait_and_pop(value);
			remain_products--;
		}
	};

	ASSERT_GE(products_per_producer * producer_cnt,
			  products_per_consumer * consumer_cnt);

	std::vector<std::thread> producer_tids, consumer_tids;
	for (int i = 0; i < producer_cnt; i++) {
		producer_tids.emplace_back(std::thread(producer));
	}
	for (int i = 0; i < consumer_cnt; i++) {
		consumer_tids.emplace_back(std::thread(consumer));
	}

	for (int i = 0; i < producer_cnt; i++) {
		producer_tids[i].join();
	}
	for (int i = 0; i < consumer_cnt; i++) {
		consumer_tids[i].join();
	}

	EXPECT_EQ(queue.size(), products_per_producer * producer_cnt
			- products_per_consumer * consumer_cnt);
}
