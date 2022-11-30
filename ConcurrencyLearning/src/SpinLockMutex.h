#ifndef SPINLOCKMUTEX_H
#define SPINLOCKMUTEX_H

#include<atomic>

class SpinLockMutex {
public:
	SpinLockMutex():
	flag() {

	}

	void lock() {
		while (flag.test_and_set(std::memory_order_acquire));
	}

	void unlock() {
		flag.clear(std::memory_order_release);
	}

private:
	std::atomic_flag flag;
};

#endif // !SPINLOCKMUTEX_H
