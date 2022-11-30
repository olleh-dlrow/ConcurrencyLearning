#include "ThreadSafeHashTable.h"
#include "gtest/gtest.h"
#include <random>
#include <algorithm>
#include <chrono>
#include <map>

TEST(ThreadSafeHashTableTest, CRUD) {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::srand(seed);
	ThreadSafeHashTable<int, int> hash_table;
	std::map<int, int> values;
	int num_insert_values = 5000;

	// init values
	for (int i = 0; i < num_insert_values; i++) {
		values.insert({ rand() % num_insert_values, rand() % num_insert_values });
	}
	std::vector< std::pair<int, int> > seq_values(values.begin(), values.end());

	// construct erase indices
	std::vector<int> erase_indices(values.size());
	for (int i = 0; i < values.size(); i++) {
		erase_indices[i] = i;
	}
	std::shuffle(erase_indices.begin(), erase_indices.end(), std::default_random_engine(seed));
	erase_indices.erase(erase_indices.begin() + erase_indices.size() / 2, erase_indices.end());

	// CRUD function
	auto inserter = [&hash_table, &seq_values](int start_index, int end_index) {
		for (int i = start_index; i < end_index; i++) {
			hash_table.insert_or_update(seq_values[i].first, seq_values[i].second);
		}
	};

	auto eraser = [&hash_table, &erase_indices, &seq_values](int start_index, int end_index) {
		for (int i = start_index; i < end_index; i++) {
			hash_table.erase(seq_values[erase_indices[i]].first);
		}
	};

	auto finder = [&hash_table, num_insert_values]() {
		int find_count = 1000;
		while (find_count--) {
			hash_table.get(rand() % num_insert_values);
		}
	};

	// multiple threads to test
	int num_inserters = 10, num_erasers = 10, num_finders = 10;
	std::vector< std::thread > inserter_tids, eraser_tids, finder_tids;

	int num_per_inserter = std::ceil((double)values.size() / num_inserters);
	for (int i = 0; i < num_inserters; i++) {
		int s = i * num_per_inserter, e = std::min((i + 1) * num_per_inserter, (int)values.size());
		inserter_tids.emplace_back(std::thread(inserter, s, e));
	}

	for (int i = 0; i < num_finders; i++) {
		finder_tids.emplace_back(finder);
	}

	int num_per_eraser = std::ceil((double)erase_indices.size() / num_erasers);
	for (int i = 0; i < num_erasers; i++) {
		int s = i * num_per_eraser, e = std::min((i + 1) * num_per_eraser, (int)erase_indices.size());
		eraser_tids.emplace_back(std::thread(eraser, s, e));
	}

	for (int i = 0; i < num_inserters; i++) {
		inserter_tids[i].join();
	}

	for (int i = 0; i < num_finders; i++) {
		finder_tids[i].join();
	}

	for (int i = 0; i < num_erasers; i++) {
		eraser_tids[i].join();
	}

	
	// erase values
	for (int i = 0; i < erase_indices.size(); i++) {
		values.erase(seq_values[erase_indices[i]].first);
	}

	// test
	for (auto it = values.begin(); it != values.end(); it++) {
		std::optional<int> opt_value = hash_table.get(it->first);
		ASSERT_TRUE(opt_value.has_value());
		ASSERT_EQ(it->second, opt_value.value()) << "inequal at key " << it->first;
	}
}
