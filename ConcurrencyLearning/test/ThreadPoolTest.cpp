#define _SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING

#include "gtest/gtest.h"
#include "ThreadPool.h"
#include <thread>
#include <future>
#include <iostream>

void print_status(std::future_status status) {
        std::string str;
        switch (status)
        {
        case std::future_status::deferred:
            str = "deferred";
            break;
        case std::future_status::ready:
            str = "ready";
            break;
        case std::future_status::timeout:
            str = "timeout";
            break;        
        default:
            break;
        }
        std::cout << str << std::endl;
};

TEST(DeadLockThreadPoolTest, DeadLock) {
    DeadLockThreadPool pool;

    std::function<int(int, int)> recursive_call = [&pool, &recursive_call](int up, int v)->int {
        if(v == up)return v;
        std::future<int> result = pool.submit(std::bind(recursive_call, up, v + 1));

        std::chrono::time_point current_point = std::chrono::system_clock::now();
        std::chrono::time_point timeout_point = current_point + 
            std::chrono::milliseconds(100);
        // epoll to get the result, if timeout, end result
        for(;current_point < timeout_point &&
            result.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
            current_point = std::chrono::system_clock::now()) {
            
        }
        if(result.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            return result.get();
        }
        return 0;
    };

    int value;

    value = 5;
    std::future<int> res1 = std::async(recursive_call, value, 0);
    EXPECT_TRUE(res1.get() == value);
    
    value = 100;
    std::future<int> res2 = std::async(recursive_call, value, 0);
    EXPECT_FALSE(res2.get() == value);
}

TEST(NoDeadLockThreadPoolTest, SolveDependencyProblem) {
    NoDeadLockThreadPool pool;

    std::function<int(int, int)> recursive_call = [&pool, &recursive_call](int up, int v)->int {
        if(v == up)return v;
        std::future<int> result = pool.submit(std::bind(recursive_call, up, v + 1));

        while(result.wait_for(std::chrono::seconds(0)) == std::future_status::timeout) {
            pool.run_pending_task();
        }

        return result.get();
    };

    int value;

    value = 5;
    std::future<int> res1 = std::async(recursive_call, value, 0);
    EXPECT_TRUE(res1.get() == value);
    
    value = 100;
    std::future<int> res2 = std::async(recursive_call, value, 0);
    EXPECT_TRUE(res2.get() == value);
}
