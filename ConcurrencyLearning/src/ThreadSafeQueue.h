#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadSafeQueue {
public:
	ThreadSafeQueue() {

	}

	void wait_and_pop(T& value) {
		std::unique_lock<std::mutex> lk(mut);
		data_cv.wait(lk, [this]() {
			return !data_queue.empty();
		});
		value = std::move(data_queue.front());
		data_queue.pop();
		return;
	}

	bool try_pop(T& value) {
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty()) {
			return false;
		}
		value = std::move(data_queue.front());
		data_queue.pop();
		return true;
	}

	void push(const T& value) {
		std::lock_guard<std::mutex> lk(mut);
		data_queue.push(value);
		data_cv.notify_one();
	}

	bool empty() const {
		std::lock_guard<std::mutex> lk(mut);
		return data_queue.empty();
	}

	int size() const {
		std::lock_guard<std::mutex> lk(mut);
		return (int)data_queue.size();
	}

private:
	std::queue<T> data_queue;
	mutable std::mutex mut;		// use in const function
	std::condition_variable data_cv;
};

#endif // !THREADSAFEQUEUE_H
