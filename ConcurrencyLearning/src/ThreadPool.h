#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "FineGrainedLockQueue.h"
#include "JoinerThreads.h"
#include <thread>
#include <future>
#include <atomic>

struct FunctionWrapper {
private:
    struct ImplBase {
        virtual void call() = 0;
        virtual ~ImplBase() {}
    };

    template<typename Func>
    struct ImplType: ImplBase {
        Func f;
        ImplType(Func&& f_):f(std::move(f_)) {

        }

        void call() override { f(); }
    };

public:
    template<typename Func>
    FunctionWrapper(Func&& f):
    impl(new ImplType<Func>(std::move(f))) {

    }

    FunctionWrapper():impl(nullptr) {

    }

    FunctionWrapper(FunctionWrapper&& wrapper):impl(wrapper.impl) {
        wrapper.impl = nullptr;
    }

    FunctionWrapper& operator=(FunctionWrapper&& wrapper) {
        impl = wrapper.impl;
        wrapper.impl = nullptr;
        return *this;
    }

    // ban copy operation
    FunctionWrapper(const FunctionWrapper&) = delete;
    FunctionWrapper& operator=(const FunctionWrapper&) = delete;

    void operator()() {
        impl->call();
    }

    ~FunctionWrapper() {
        delete impl;
    }

private:
    ImplBase* impl;
};

// this bad thread pool has the problem of dead lock
class DeadLockThreadPool {
public:
    DeadLockThreadPool():done(false), joiner(threads) {
        int num_hardware_threads = std::thread::hardware_concurrency();
        try {
            for(int i = 0; i < num_hardware_threads; i++) {
                threads.emplace_back(std::thread(
                    &DeadLockThreadPool::do_work_per_thread,
                    this
                ));
            }
        } catch(...) {
            kill_all();
            throw;
        }
    }

    ~DeadLockThreadPool() {
        kill_all();
    }

    void kill_all() {
        done.store(true);
    }

    // tips: use invoke_result for C++17
    // raw pointer: function(ptr)
    // member function: bind(&func, ...)
    template<typename Func>
    std::future<typename std::invoke_result_t<Func()>::result_type> 
        submit(Func f) {
        using ResultType = std::invoke_result_t<Func()>::result_type;
        
        // function -> packaged_task<param> -> FunctionWrapper -> call
        std::packaged_task<ResultType()> task(std::move(f));
        std::future<ResultType> result = task.get_future();
        queue.push(std::move(FunctionWrapper(std::move(task))));
        return result;
    }

private:

    void do_work_per_thread() {
        while(!done.load()) {
            FunctionWrapper task;
            if(queue.try_pop(task)) {
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }

    std::atomic<bool> done;
    FineGrainedLockQueue<FunctionWrapper> queue;
    std::vector<std::thread> threads;
    JoinThreads joiner;
};

// solve the dependency problem, 
// when waiting for another thread to finish, 
// current thread can take a new task
class NoDeadLockThreadPool {
public:
    NoDeadLockThreadPool():done(false), joiner(threads) {
        int num_hardware_threads = std::thread::hardware_concurrency();
        try {
            for(int i = 0; i < num_hardware_threads; i++) {
                threads.emplace_back(std::thread(
                    &NoDeadLockThreadPool::do_work_per_thread,
                    this
                ));
            }
        } catch(...) {
            kill_all();
            throw;
        }
    }

    ~NoDeadLockThreadPool() {
        kill_all();
    }

    void kill_all() {
        done.store(true);
    }

    // tips: use invoke_result for C++17
    // raw pointer: function(ptr)
    // member function: bind(&func, ...)
    template<typename Func>
    std::future<typename std::invoke_result_t<Func()>::result_type> 
        submit(Func f) {
        using ResultType = std::invoke_result_t<Func()>::result_type;
        
        // function -> packaged_task<param> -> FunctionWrapper -> call
        std::packaged_task<ResultType()> task(std::move(f));
        std::future<ResultType> result = task.get_future();
        queue.push(std::move(FunctionWrapper(std::move(task))));
        return result;
    }

    void run_pending_task() {
        FunctionWrapper task;
        if(queue.try_pop(task)) {
            task();
        } else {
            std::this_thread::yield();
        }
    }

private:

    void do_work_per_thread() {
        while(!done.load()) {
            FunctionWrapper task;
            if(queue.try_pop(task)) {
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }

    std::atomic<bool> done;
    FineGrainedLockQueue<FunctionWrapper> queue;
    std::vector<std::thread> threads;
    JoinThreads joiner;
};

#endif
