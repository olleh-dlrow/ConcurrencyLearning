#ifndef FINEGRAINEDLOCKQUEUE_H
#define FINEGRAINEDLOCKQUEUE_H

#include <mutex>
#include <condition_variable>

template<typename T>
class FineGrainedLockQueue {
private:
	struct Node {
		T data;
		Node* next;
	};

public:
	FineGrainedLockQueue():
	head(new Node), tail(head), length(0) {

	}

	void push(const T& value) {
		Node* new_node = new Node;
		{
			std::lock_guard<std::mutex> lk(tail_mut);
			tail->data = value;
			tail->next = new_node;
			tail = new_node;
		}
		{
			std::lock_guard<std::mutex> size_lk(size_mut);
			length++;
		}
		data_cv.notify_one();
	}

	void push(T&& value) {
		Node* new_node = new Node;
		{
			std::lock_guard<std::mutex> lk(tail_mut);
			tail->data = std::move(value);
			tail->next = new_node;
			tail = new_node;
		}
		{
			std::lock_guard<std::mutex> size_lk(size_mut);
			length++;
		}
		data_cv.notify_one();		
	}

	void wait_and_pop(T& value) {
		std::unique_lock<std::mutex> lk(head_mut);
		data_cv.wait(lk, [this]() {
			return this->head != this->get_tail();
		});
		value = std::move(head->data);
		pop_head();
		{
			std::lock_guard<std::mutex> size_lk(size_mut);
			length--;
		}
		return;
	}

	bool try_pop(T& value) {
		std::lock_guard<std::mutex> lk(head_mut);
		if (head == get_tail()) {
			return false;
		}
		value = std::move(head->data);
		pop_head();
		{
			std::lock_guard<std::mutex> size_lk(size_mut);
			length--;
		}
		return true;
	}

	bool empty() const {
		std::lock_guard<std::mutex> lk(head_mut);
		return head == get_tail();
	}

	int size() const {
		std::lock_guard<std::mutex> lk(size_mut);
		return length;
	}

private:
	void pop_head() {
		Node* old_head = head;
		head = head->next;
		delete old_head;
	}

	Node* get_tail() const {
		std::lock_guard<std::mutex> lk(tail_mut);
		return tail;
	}

	int length;		// may use atomic has better performance
	Node* head;
	Node* tail;
	mutable std::mutex head_mut;
	mutable std::mutex tail_mut;
	mutable std::mutex size_mut;
	std::condition_variable data_cv;
};

#endif // !FINEGRAINEDLOCKQUEUE_H
