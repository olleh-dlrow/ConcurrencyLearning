#define _SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING

#include "ParallelAlgorithm.h"
#include "gtest/gtest.h"

#include <iostream>
#include <vector>

const int PARALLEL = 1, ASYNC = 2, STD = 0, SINGLE = 3, THREAD_POOL = 4;

struct TestElem {
    TestElem(int val=0):
    value(val) {

    }

    void handle(std::chrono::nanoseconds wait_time=handle_time) const {
        if(handle_time.count() == 0)return;
        std::this_thread::sleep_for(wait_time);
    }

    bool operator==(const TestElem& rhs) const {
        handle();
        return rhs.value == value;
    }

    TestElem operator+(const TestElem& rhs) const {
        handle();
        return TestElem(value + rhs.value);
    }

    TestElem& operator+=(const TestElem& rhs) {
        handle();
        value += rhs.value;
        return *this;
    }

    bool operator<(const TestElem& rhs) const {
        handle();
        return value < rhs.value;
    }

    bool operator<=(const TestElem& rhs) const {
        handle();
        return value <= rhs.value;
    }

    static const std::chrono::nanoseconds default_time;

    int value;
    static std::chrono::nanoseconds handle_time;
};

const std::chrono::nanoseconds TestElem::default_time = std::chrono::nanoseconds(400);
std::chrono::nanoseconds TestElem::handle_time = TestElem::default_time;

///////////////////////////////////////
// for_each
///////////////////////////////////////

class ParallelForEachTest: public testing::Test {
protected:
    void SetUp() override {
        values = std::vector<TestElem>(100);
    }

    void for_each_test(int state) {
        auto handler = [this](const TestElem& value) {
            value.handle();
        };

        if(state == ASYNC) {
            async_for_each(values.begin(), values.end(), handler);
        } else if(state == PARALLEL) {
            parallel_for_each(values.begin(), values.end(), handler);
        } else if(state == STD) {
            std::for_each(values.begin(), values.end(), handler);
        }
    }

    std::vector<TestElem> values;
};



TEST_F(ParallelForEachTest, Async) {
    for_each_test(ASYNC);
}

TEST_F(ParallelForEachTest, Std) {
    for_each_test(STD);
}

TEST_F(ParallelForEachTest, Parallel) {
    for_each_test(PARALLEL);
}

///////////////////////////////////////
// find
///////////////////////////////////////

class ParallelFindTest: public testing::Test {
protected:
    void SetUp() override {
        unsigned seed = 234;
        srand(seed);
        find_value = TestElem(235);
        values = std::vector<TestElem>(100);

        int num_split = 4;
        int split_size = values.size() / num_split;
        int pos = split_size * (num_split - 1) + rand() % split_size;

        find_it = values.begin();
        std::advance(find_it, pos);

        *find_it = find_value;
    }

    void find_test(int state) {
        decltype(values)::iterator it;
        if(state == PARALLEL) {
            it = parallel_find(values.begin(), values.end(), find_value);
        } else if(state == STD) {
            it = std::find(values.begin(), values.end(), find_value);
        } else if(state == SINGLE) {
            for(it = values.begin(); it != values.end(); ++it) {
                if(*it == find_value) {
                    break;
                }
            }
        }

        EXPECT_EQ(it, find_it);
        EXPECT_EQ(*it, find_value);
    }

    std::vector<TestElem> values;
    TestElem find_value;
    decltype(values)::iterator find_it;
};

TEST_F(ParallelFindTest, Parallel) {
    find_test(PARALLEL);
}

TEST_F(ParallelFindTest, Std) {
    find_test(STD);
}

TEST_F(ParallelFindTest, Single) {
    find_test(SINGLE);
}

///////////////////////////////////////
// accumulate
///////////////////////////////////////

class ParallelAccumulateTest: public testing::Test {
protected:
    void SetUp() override {
        values = std::vector<TestElem>(100, TestElem(1));
        sum.value = 100;
    }

    void accumulate_test(int state) {
        TestElem result(0);
        if(state == PARALLEL) {
            result = parallel_accumulate(values.begin(), values.end(), result);
        } else if(state == STD) {
            result = std::accumulate(values.begin(), values.end(), result);
        }

        EXPECT_EQ(result, sum);
    }

    std::vector<TestElem> values;
    TestElem sum;
};

TEST_F(ParallelAccumulateTest, Parallel) {
    accumulate_test(PARALLEL);
}

TEST_F(ParallelAccumulateTest, Std) {
    accumulate_test(STD);
}

///////////////////////////////////////
// quick_sort
///////////////////////////////////////

class ParallelQuickSortTest: public testing::Test {
protected:
    void SetUp() override {
        unsigned seed = 234;
        srand(seed);

        TestElem::handle_time = std::chrono::nanoseconds(40);
        values.resize(1000);
        for(auto&& v : values) {
            v.value = rand();
        }
        unsorted_values = values;

        sorted_values = values;
        std::sort(sorted_values.begin(), sorted_values.end(), 
            [](const TestElem& lhs, const TestElem& rhs){
                return lhs.value < rhs.value;
        });
    }

    void quick_sort_test(int state) {
        if(state == ASYNC) {
            async_quick_sort(values.begin(), values.end());
        } else if(state == STD) {
            std::sort(values.begin(), values.end());
        } else if(state == THREAD_POOL) {
            thread_pool_quick_sort(values.begin(), values.end());
        }
    }

    void expect_equal() {
        values = unsorted_values;
        quick_sort_test(ASYNC);

        for(int i = 0; i < (int)values.size(); i++) {
            ASSERT_EQ(values[i].value, sorted_values[i].value);
        }

        values = unsorted_values;
        quick_sort_test(THREAD_POOL);

        for(int i = 0; i < (int)values.size(); i++) {
            ASSERT_EQ(values[i].value, sorted_values[i].value);
        }
    }

    void TearDown() override {
        TestElem::handle_time = TestElem::default_time;
    }

    std::vector<TestElem> unsorted_values;
    std::vector<TestElem> values;
    std::vector<TestElem> sorted_values;
};

TEST_F(ParallelQuickSortTest, Std) {
    quick_sort_test(STD);
}



TEST_F(ParallelQuickSortTest, ThreadPool) {
    quick_sort_test(THREAD_POOL);
}

TEST_F(ParallelQuickSortTest, Async) {
    quick_sort_test(ASYNC);
}

TEST_F(ParallelQuickSortTest, Equal) {
    expect_equal();
}

