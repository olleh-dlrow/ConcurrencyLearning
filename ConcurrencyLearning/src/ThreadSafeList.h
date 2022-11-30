#ifndef THREADSAFELIST_H
#define THREADSAFELIST_H

#include <mutex>
#include <optional>

template<typename T>
class ThreadSafeList {
private:
    struct Node {
        std::mutex mut;
        T data;
        Node* next;

        Node(): 
        next(nullptr) {

        }

        Node(const T& value):
        data(value), next(nullptr) {

        }
    };

public:
    ThreadSafeList() {

    }

    ~ThreadSafeList() {
        remove_if([](const T& data) {
            return true;
        });
    }

    void push_front(const T& value) {
        Node* new_node = new Node(value);
        std::lock_guard<std::mutex> lk(head.mut);
        new_node->next = head.next;
        head.next = new_node;
    }

    template<typename Func>
    void for_each(Func f) {
        Node* curr = &head;
        std::unique_lock<std::mutex> curr_lk(curr->mut);
        while(Node* next = curr->next) {
            std::unique_lock<std::mutex> next_lk(next->mut);
            curr_lk.unlock();
            f(next->data);
            curr = next;
            curr_lk = std::move(next_lk);
        }
    }

    template<typename Pred>
    std::optional<T> find_first_if(Pred p) {
        Node* curr = &head;
        std::optional<T> opt_value;
        std::unique_lock<std::mutex> curr_lk(curr->mut);
        while(Node* next = curr->next) {
            std::unique_lock<std::mutex> next_lk(next->mut);
            curr_lk.unlock();
            if(p(next->data)) {
                opt_value = next->data;
                return opt_value;
            }
            curr = next;
            curr_lk = std::move(next_lk);
        }
        return opt_value;
    }

    template<typename Pred>
    void remove_if(Pred p) {
        Node* curr = &head;
        std::unique_lock<std::mutex> curr_lk(curr->mut);
        while(Node* next = curr->next) {
            std::unique_lock<std::mutex> next_lk(next->mut);

            if(p(next->data)) {
                Node* old_next = std::move(next);
                curr->next = next->next;
                next_lk.unlock();
                delete old_next;
            } else {
                curr_lk.unlock();
                curr = next;
                curr_lk = std::move(next_lk);
            }
        }        
    }

private:
    Node head;
};

#endif
