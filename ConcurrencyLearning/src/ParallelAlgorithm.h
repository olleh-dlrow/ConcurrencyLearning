#ifndef PARALLELALGORITHM_H
#define PARALLELALGORITHM_H

#include "ThreadPool.h"
#include "JoinerThreads.h"
#include <algorithm>
#include <thread>
#include <future>
#include <atomic>
#include <numeric>

template<typename Iterator, typename Func>
void async_for_each(Iterator first, Iterator last, Func f) {
    int length = std::distance(first, last);
    if(!length) {
        return;
    }

    int min_per_thread = 25;
    if(length < 2 * min_per_thread) {
        std::for_each(first, last, f);
    } else {
        Iterator mid_it = first + length / 2;
        std::future<void> first_half = std::async(
            &async_for_each<Iterator, Func>, 
            first, 
            mid_it, 
            f
        );
        async_for_each(mid_it, last, f);
        first_half.get();
    }
}

template<typename Iterator, typename Func>
void parallel_for_each(Iterator first, Iterator last, Func f) {
    int length = std::distance(first, last);
    if(!length) return;

    int min_per_thread = 25;
    int max_threads = (length + min_per_thread - 1) / min_per_thread;

    int num_hardware_threads = std::thread::hardware_concurrency();

    int num_threads = std::min(num_hardware_threads, max_threads);

    int block_size = length / num_threads;

    std::vector<std::future<void> > futures(num_threads - 1);
    
    Iterator block_start = first;
    for(int i = 0; i < num_threads - 1; i++) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        std::packaged_task<void(void)> task(
            [=]() {
                std::for_each(block_start, block_end, f);
            }
        );
        futures[i] = task.get_future();
        std::thread(std::move(task)).detach();
        block_start = block_end;
    }
    std::for_each(block_start, last, f);
    for(int i = 0; i < num_threads - 1; i++) {
        futures[i].get();
    }
}


template<typename Iterator, typename MatchType> 
Iterator parallel_find(Iterator first, Iterator last, MatchType match) {
    auto find_element = [](Iterator begin, Iterator end, MatchType match,
            std::promise<Iterator>& result, std::atomic<bool>& done_flag) {
        try {
            for(; (begin != end) && !done_flag.load(); ++begin) {
                if(*begin == match) {
                    result.set_value(begin);
                    done_flag.store(true);
                    return;    
                }
            }
        } catch(...) {
            // result has been set
            done_flag.store(true);
        }
    };
    
    int length = std::distance(first, last);
    if(!length)return last;

    int min_per_thread = 25;
    int max_threads = (length + min_per_thread - 1) / min_per_thread;

    int num_hardware_threads = std::thread::hardware_concurrency();

    int num_threads = std::min(num_hardware_threads, max_threads);

    int block_size = length / num_threads;

    std::promise<Iterator> result;
    std::atomic<bool> done_flag(false);
    std::vector<std::thread> threads(num_threads - 1);

    {
        JoinThreads joiner(threads);
        Iterator block_start = first;
        for(int i = 0; i < num_threads - 1; i++) {
            Iterator block_end = block_start;
            std::advance(block_end, block_size);
            threads[i] = std::thread(find_element, block_start, block_end, match, std::ref(result), std::ref(done_flag));
            block_start = block_end;
        }
        find_element(block_start, last, match, std::ref(result), std::ref(done_flag));
    }

    if(!done_flag.load()) {
        return last;
    }
    return result.get_future().get();
}

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    auto accumulate_block = [](Iterator first,Iterator last,T& result) {
        result=std::accumulate(first,last,result);
    };

    int length=std::distance(first,last);

    if(!length)
        return init;

    int min_per_thread=25;
    int max_threads=
        (length+min_per_thread-1)/min_per_thread;

    int hardware_threads=
        std::thread::hardware_concurrency();

    int num_threads=
        std::min(hardware_threads, max_threads);

    int block_size=length/num_threads;

    std::vector<T> results(num_threads);
    std::vector<std::thread>  threads(num_threads-1);

    {
        JoinThreads joiner(threads);
        Iterator block_start=first;
        for(int i=0;i<(num_threads-1);++i)
        {
            Iterator block_end=block_start;
            std::advance(block_end,block_size);
            threads[i]=std::thread(
                accumulate_block,
                block_start,block_end,std::ref(results[i]));
            block_start=block_end;
        }
        accumulate_block(block_start,last,results[num_threads-1]);
    }

    return std::accumulate(results.begin(),results.end(),init);
}

template<typename Iterator>
void async_quick_sort(Iterator first, Iterator last) {
    using T = std::remove_reference<decltype(*first)>::type;
    int length = std::distance(first, last);
    if(!length)return;

    int min_per_thread = 25;
    if(length < min_per_thread) {
        std::sort(first, last);
        return;
    }
    T mid_value = *next(first, length / 2);
    // split to less and greater_equal
    Iterator geq_point = std::partition(first, last, [&mid_value](const T& value){
        return value < mid_value;
    });
    // split to less, equal and greater
    Iterator greater_point = std::partition(geq_point, last, [&mid_value](const T& value){
        return value == mid_value;
    });

    std::future<void> lower_result = std::async(
        &async_quick_sort<Iterator>,
        first, geq_point
    );
    async_quick_sort(greater_point, last);
    lower_result.get();
    return;
}

template<typename Iterator>
void thread_pool_quick_sort(Iterator first, Iterator last) {
    NoDeadLockThreadPool pool;
    std::function<void(Iterator, Iterator)> do_sort = 
    [&pool, &do_sort](Iterator first, Iterator last) {
        using T = std::remove_reference<decltype(*first)>::type;
        int length = std::distance(first, last);
        if(!length)return;

        int min_per_thread = 25;
        if(length < min_per_thread) {
            std::sort(first, last);
            return;
        }
        T mid_value = *next(first, length / 2);
        // split to less and greater_equal
        Iterator geq_point = std::partition(first, last, [&mid_value](const T& value){
            return value < mid_value;
        });
        // split to less, equal and greater
        Iterator greater_point = std::partition(geq_point, last, [&mid_value](const T& value){
            return value == mid_value;
        });

        std::future<void> lower_result = pool.submit(
            std::bind(
                do_sort, first, geq_point
            )
        );

        while(lower_result.wait_for(std::chrono::seconds(0)) == std::future_status::timeout) {
            pool.run_pending_task();
        }

        do_sort(greater_point, last);
        lower_result.get();
        return;
    };

    do_sort(first, last);
}

#endif
