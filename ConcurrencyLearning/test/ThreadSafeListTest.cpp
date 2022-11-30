#include "ThreadSafeList.h"
#include "gtest/gtest.h"

#include <vector>

TEST(ThreadSafeListTest, CRUD) {
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::srand(seed);

    std::vector<int> value_set = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37
    };

    int list_length = 3000;
    ThreadSafeList<int> list;

    auto inserter = [&list, &value_set](int num_insert) {
        for(int i = 0; i < num_insert; i++) {
            list.push_front(value_set[rand() % (int)value_set.size()]);
        }
    };

    auto adder = [&list](int value) {
        list.for_each([value](int& data) {
            data += value;
        });
    };

    auto finder = [&list, &value_set](int num_find) {
        for(int i = 0; i < num_find; i++) {
            list.find_first_if([&value_set](const int& data){
                return data == value_set[rand() % (int)value_set.size()];
            });
        }
    };

    auto remover = [&list](int value) {
        while(list.find_first_if([value](const int& data){
            return data == value;
        }).has_value()) {
            list.remove_if([value](const int& data){
                return data == value;
            });
        }
    };

    int num_inserters = 10, num_adders = 10 * 2, num_finders = 10, num_removers = value_set.size();
    int num_per_insert = std::ceil((double)list_length / num_inserters);

    std::vector<std::thread> inserter_tids, adder_tids, finder_tids, remover_tids;
    
    // insert
    for(int i = 0; i < num_inserters; i++) {
        int s = i * num_per_insert, e = std::min((i + 1) * num_per_insert, list_length);
        inserter_tids.emplace_back(std::thread(inserter, e - s));    
    }

    for(int i = 0; i < num_inserters; i++) {
        inserter_tids[i].join();
    }

    // add and sub
    ASSERT_EQ(num_adders % 2, 0);
    for(int i = 0; i < num_adders/2; i++) {
        adder_tids.emplace_back(std::thread(adder, 5));
    }

    for(int i = 0; i < num_adders/2; i++) {
        adder_tids.emplace_back(std::thread(adder, -5));
    }

    for(int i = 0; i < num_adders; i++) {
        adder_tids[i].join();
    }

    // random find
    for(int i = 0; i < num_finders; i++) {
        finder_tids.emplace_back(std::thread(finder, 100));
    }

    // remove all
    for(int i = 0; i < num_removers; i++) {
        remover_tids.emplace_back(std::thread(remover, value_set[i]));
    }

    for(int i = 0; i < num_finders; i++) {
        finder_tids[i].join();
    }

    for(int i = 0; i < num_removers; i++) {
        remover_tids[i].join();
    }

    for(int i = 0; i < value_set.size(); i++) {
        int value = value_set[i];
        EXPECT_FALSE(list.find_first_if([value](const int& data){
            return data == value;       
        }).has_value());
    }
}
